#include "ata.h"

#include <stdint.h>

#include "../../io/io.h"
#include "../../memory/heap.h"

// Array to hold detected ATA devices.
//
// The 'present' bit will be set if the device is detected.
ata_device devices[4];

typedef union {
    uint8_t i;
    struct {
        uint8_t err_chk : 1;
        uint8_t pad : 2;
        uint8_t drq : 1;
        uint8_t cmd : 1;
        uint8_t df_se : 1;
        uint8_t rdy : 1;
        uint8_t bsy : 1;
    };
} ata_status;

__attribute__((always_inline)) void inline ata_delay(uint16_t bus) {
    // 400ns delay
    for (int i = 0; i < 4; i++) {
        io_ins8(ATA_PORT_CTRL_ALTSTATUS_RESET(bus));
    }
}

ata_device *ata_get_device(uint16_t bus, uint8_t dev) {
    if (dev > 1) {
        return 0;
    }
    if (bus == ATA_BUS_1) {
        return &devices[dev];
    } else if (bus == ATA_BUS_2) {
        return &devices[dev + 2];
    }
    return 0;
}

// Return the status of the current device for the selected bus.
//
// The device must be selected prior to this command.
ata_status ata_get_status(uint16_t bus) {
    ata_status s = {0};
    s.i = io_ins8(ATA_PORT_COMMAND_STATUS(bus));
    return s;
}

int ata_wait_or_error(uint16_t bus) {
    ata_status s = {0};
    do {
        s = ata_get_status(bus);
        if (s.err_chk) {
            return 0;
        }
    } while (s.bsy || !s.drq);
    return 1;
}

void ata_set_device(uint16_t bus, uint8_t device) {
    if (device > 1) {
        return;
    }
    // LBA bit (0x40) to true for future read/writes.
    uint8_t dev_sel = 0x40 | (device << 4);
    io_out8(ATA_PORT_DRIVE_SELECT(bus), dev_sel);
    ata_delay(bus);
}

// Check for floating buses, if not floating, get status of each device.
//
// Devices should have at least one bit set in their status at anytime.
// If we receive a 0 status on a non-floating bus the device is not present on
// the bus.
int ata_probe_devices(uint16_t bus) {
    ata_device *devs = devices;

    if (bus == ATA_BUS_2) {
        devs += 2;
    }

    uint16_t *id_buffer = heap_zalloc(512);
    if (!id_buffer) return 0;

    for (uint8_t i = 0; i < 2; i++) {
        ata_set_device(bus, i);
        ata_status s = {0};
        s = ata_get_status(bus);
        if (s.i == 0xFF || s.i == 0) continue;

        devs[i].present = 1;

        // get identify block
        io_out8(ATA_PORT_COMMAND_STATUS(bus), ATA_COMMAND_IDENTIFY);
        ata_delay(bus);

        // wait for bsy to clear and drq to set
        if (!ata_wait_or_error(bus)) {
            continue;
        }

        // read identify data
        for (int j = 0; j < 256; j++) {
            id_buffer[j] = io_ins16(ATA_PORT_DATA(bus));
        }

        // extract 32 bit max addressable sector
        devs[i].sectors =
            ((uint64_t)id_buffer[ATA_IDENTIFY_SECTORS_ONE] |
             (uint64_t)id_buffer[ATA_IDENTIFY_SECTORS_TWO] << 16 |
             (uint64_t)id_buffer[ATA_IDENTIFY_SECTORS_THREE] << 32 |
             (uint64_t)id_buffer[ATA_IDENTIFY_SECTORS_FOUR] << 48);
    }

    heap_free(id_buffer);

    return 1;
};

// Use 48-bit LBA and READ EXT command semantics to read the requested sectors
int ata_read_sectors(uint16_t bus, uint8_t dev, uint64_t start_lba,
                     uint64_t count, uint16_t buffer[]) {
    ata_device *device = ata_get_device(bus, dev);
    if (!device || device->present == 0) {
        return 0;
    }
    if (!count) {
        return 0;
    }

    // cannot read more sectors then the device has.
    if (start_lba > (device->sectors - 1) ||
        (start_lba + count > device->sectors)) {
        return 0;
    }

    ata_set_device(bus, dev);

    ata_status s = {0};
    while (count > 0) {
        uint64_t read_count = count;

        if (count > ATA_SECTOR_MAX_COUNT) {
            // zero means read max sector count of 65,536 sectors.
            read_count = 0;
        }

        // double writes to registers are used for READ EXT command.
        // multi-bytes are written as high bits first, low bits last.
        io_out8(ATA_PORT_SECTOR_COUNT(bus), ATA_SECTOR_COUNT_ONE(read_count));
        io_out8(ATA_PORT_SECTOR_COUNT(bus), ATA_SECTOR_COUNT_TWO(read_count));

        if (!read_count) {
            read_count = ATA_SECTOR_MAX_COUNT;
        }

        io_out8(ATA_PORT_LBA_LOW(bus), ATA_LBA_LOW_ONE(start_lba));
        io_out8(ATA_PORT_LBA_LOW(bus), ATA_LBA_LOW_TWO(start_lba));
        io_out8(ATA_PORT_LBA_MID(bus), ATA_LBA_MID_ONE(start_lba));
        io_out8(ATA_PORT_LBA_MID(bus), ATA_LBA_MID_TWO(start_lba));
        io_out8(ATA_PORT_LBA_HIGH(bus), ATA_LBA_HIGH_ONE(start_lba));
        io_out8(ATA_PORT_LBA_HIGH(bus), ATA_LBA_HIGH_TWO(start_lba));

        io_out8(ATA_PORT_COMMAND_STATUS(bus), ATA_COMMAND_READ_EXT);

        for (uint64_t i = 0; i < read_count; i++) {
            if (!ata_wait_or_error(bus)) {
                return 0;
            }

            for (int j = 0; j < 256; j++) {
                uint16_t data = io_ins16(ATA_PORT_DATA(bus));
                *buffer++ = data;
            }
            count -= 1;
        }

        start_lba += read_count;
    }
    return 0;
}

int ata_init() {
    if (!ata_probe_devices(ATA_BUS_1)) {
        return 0;
    }
    if (!ata_probe_devices(ATA_BUS_2)) {
        return 0;
    }
    return 0;
}

#include <stdint.h>

#define ATA_BUS_1 0x1F0
#define ATA_BUS_2 0x170

#define ATA_PORT_DATA(bus) (bus)
#define ATA_PORT_ERROR_FEAT(bus) (bus + 1)
#define ATA_PORT_SECTOR_COUNT(bus) (bus + 2)
#define ATA_PORT_LBA_LOW(bus) (bus + 3)
#define ATA_PORT_LBA_MID(bus) (bus + 4)
#define ATA_PORT_LBA_HIGH(bus) (bus + 5)
#define ATA_PORT_DRIVE_SELECT(bus) (bus + 6)
#define ATA_PORT_COMMAND_STATUS(bus) (bus + 7)
#define ATA_PORT_CTRL_ALTSTATUS_RESET(bus) (bus + 0x206)
#define ATA_PORT_DEV_ADDRESS(bus) (bus + 0x207)

#define ATA_COMMAND_READ_EXT 0x24
#define ATA_COMMAND_IDENTIFY 0xEC

#define ATA_SECTOR_MAX_COUNT (1 << 16)
#define ATA_LBA_MAX (1ULL << 48)

#define ATA_LBA_LOW_ONE(lba) ((lba >> 24) & 0xFF)
#define ATA_LBA_MID_ONE(lba) ((lba >> 32) & 0xFF)
#define ATA_LBA_HIGH_ONE(lba) ((lba >> 40) & 0xFF)
#define ATA_LBA_LOW_TWO(lba) (lba & 0xFF)
#define ATA_LBA_MID_TWO(lba) ((lba >> 8) & 0xFF)
#define ATA_LBA_HIGH_TWO(lba) ((lba >> 16) & 0xFF)

#define ATA_SECTOR_COUNT_ONE(count) ((count >> 8) & 0xFF)
#define ATA_SECTOR_COUNT_TWO(count) (count & 0xFF)

#define ATA_IDENTIFY_SECTORS_ONE (100)
#define ATA_IDENTIFY_SECTORS_TWO (101)
#define ATA_IDENTIFY_SECTORS_THREE (102)
#define ATA_IDENTIFY_SECTORS_FOUR (103)

typedef struct ata_device {
    uint32_t present : 1;
    uint32_t : 31;
    uint64_t sectors;
} ata_device;

// Initialize the ATA subsystem.
//
// Probe the controller to identity which disks are present.
int ata_init();

int ata_read_sectors(uint16_t bus, uint8_t dev, uint64_t start_lba,
                     uint64_t count, uint16_t buffer[]);

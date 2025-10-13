#include "pic.h"

#include "../../io/io.h"

#define PIC_INIT_CMD 0x11       // 0b00010001
#define PIC_MASTER_OFFSET 0x20  // IRQ[0..7] -> INT[0x20..0x27]
#define PIC_SLAVE_OFFSET 0x28   // IRQ[8..15] -> INT[0x28..0x2F]
#define PIC_CASCADE 2           // for master, we shift this to get 0x04.
#define PIC_8086_MODE 0x01

void pic_init() {
    // Place both PICs into init mode, now expects 3 additional commands on
    // the data ports. (ICW1)
    io_out8(PIC_MASTER_CMD_PORT, PIC_INIT_CMD);
    io_out8(PIC_SLAVE_CMD_PORT, PIC_INIT_CMD);

    // Set PIC's IRQ to interrupt vector offets. (ICW2)
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_OFFSET);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_OFFSET);

    // Tell master to cascade to slave (ICW3)
    io_out8(PIC_MASTER_DATA_PORT, 1 << PIC_CASCADE);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_CASCADE);

    // Set PICs to 8086 mode (ICW4)
    io_out8(PIC_MASTER_DATA_PORT, PIC_8086_MODE);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_8086_MODE);

    // Init done, now we can unmaks our PICs, done by writing directly to DATA
    // ports with no previous commands.
    io_out8(PIC_MASTER_DATA_PORT, 0x00);
    io_out8(PIC_SLAVE_DATA_PORT, 0x00);

    // Our bootloader code disabled interrupts prior to jumping to the kernel.
    // With the PIC properly initialized now they should be enabled
    asm volatile("sti");
}

#ifndef PIC_H
#define PIC_H

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_KEYBOARD_IRQ 0x20

void pic_init();

#endif // PIC_H

#pragma once

extern void InitializePIC();

/* Primary PIC ports */
#define PIC_PRI_CMDR 0x20 /* Primary PIC Command and Status Register */
#define PIC_PRI_IMR 0x21 /* Primary PIC Interrupt Mask Register (read only) */
#define PIC_PRI_DATAR 0x21 /* Primary PIC Data Register (write only) */

/* Secondary PIC ports */
#define PIC_SLAVE_CMDR 0xA0 /* Slave PIC Command and Status Register */
#define PIC_SLAVE_IMR 0xA1 /* Slave PIC Interrupt Mask Register (read only) */
#define PIC_SLAVE_DATAR 0xA1 /* Slave PIC Data Register (write only) */

extern void SetPriEOI();
extern void SetSlaveEOI();

extern void EnableInterrupts();
extern void DisableInterrupts();

extern void EnableIRQ(uint8_t irq);

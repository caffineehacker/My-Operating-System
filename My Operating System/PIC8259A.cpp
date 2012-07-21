#include <cstdint>
#include "ports.h"
#include "PIC8259A.h"

void EnableInterrupts()
{
	_asm { sti }
}

void DisableInterrupts()
{
	_asm { cli }
}

/*
 * Port defines are included in ports.h
 * Port		Address	Description
 * 0x20		Primary PIC Command and Status Register
 * 0x21		Primary PIC Interrupt Mask Register and Data Register
 * 0xA0		Secondary (Slave) PIC Command and Status Register
 * 0xA1		Secondary (Slave) PIC Interrupt Mask Register and Data Register
 */

/*
 * Enables an IRQ
 */
void EnableIRQ(uint8_t irq)
{
	uint8_t shiftedIRQ = 1;
	for (; irq > 0; irq--)
		shiftedIRQ<<=1;
	outb(irq < 8 ? PIC_PRI_DATAR : PIC_SLAVE_DATAR, (uint8_t)(inb(PIC_PRI_IMR) & ~shiftedIRQ)); /* Setting a bit to 0 enables the IRQ */
}

/*
 * Disables an IRQ
 */
void DisableIRQ(uint8_t irq)
{
	uint8_t shiftedIRQ = 1;
	for (; irq > 0; irq--)
		shiftedIRQ<<=1;
	outb(irq < 8 ? PIC_PRI_DATAR : PIC_SLAVE_DATAR, (uint8_t)(inb(PIC_PRI_IMR) | shiftedIRQ)); /* Setting a bit to 1 disables the IRQ */
}

/*
Initialization Control Word (ICW) 1
Bit Number	Value	Description
0	IC4	If set(1), the PIC expects to recieve IC4 during initialization.
1	SNGL	If set(1), only one PIC in system. If cleared, PIC is cascaded with slave PICs, and ICW3 must be sent to controller.
2	ADI	If set (1), CALL address interval is 4, else 8. This is useually ignored by x86, and is default to 0
3	LTIM	If set (1), Operate in Level Triggered Mode. If Not set (0), Operate in Edge Triggered Mode
4	1	Initialization bit. Set 1 if PIC is to be initialized
5	0	MCS-80/85: Interrupt Vector Address. x86 Architecture: Must be 0
6	0	MCS-80/85: Interrupt Vector Address. x86 Architecture: Must be 0
7	0	MCS-80/85: Interrupt Vector Address. x86 Architecture: Must be 0
*/

/*
Initialization Control Word (ICW) 2
Bit Number	Value										Description
0-2			A8/A9/A10									Address bits A8-A10 for IVT when in MCS-80/85 mode.
3-7			A11(T3)/A12(T4)/A13(T5)/A14(T6)/A15(T7)		Address bits A11-A15 for IVT when in MCS-80/85 mode. In 80x86 mode, specifies the interrupt vector address. May be set to 0 in x86 mode
*/

/*
Initialization Control Word (ICW) 3 - Primary PIC
Bit Number	Value	Description
0-7			S0-S7	Specifies what Interrupt Request (IRQ) is connected to slave PIC

Initialization Control Word (ICW) 3 - Secondary PIC
Bit Number		Value	Description
0-2				ID0		IRQ number the master PIC uses to connect to (In binary notation)
3-7				0		Reserved, must be 0
*/

/*
Initialization Control Word (ICW) 4
Bit Number	Value		Description
0			uPM			If set (1), it is in 80x86 mode. Cleared if MCS-80/86 mode
1			AEOI		If set, on the last interrupt acknowledge pulse, controller automatically performs End of Interrupt (EOI) operation
2			M/S			Only use if BUF is set. If set (1), selects buffer master. Cleared if buffer slave.
3			BUF			If set, controller operates in buffered mode
4			SFNM		Special Fully Nested Mode. Used in systems with a large amount of cascaded controllers.
5-7			0			Reserved, must be 0
*/

void InitializePIC()
{
	outb(PIC_PRI_CMDR, 0x11); /* Initialize both PICs with ICW1 */
	outb(PIC_SLAVE_CMDR, 0x11);

	/* Map IRQ 0 to 0x20 and 8 to 0x28 */
	outb(PIC_PRI_DATAR, PIC_PRI_CMDR); /* Send ICW2 immediately after ICW1 */
	outb(PIC_SLAVE_DATAR, 0x28);

	/* Now for ICW3 */
	outb(PIC_PRI_DATAR, 0x4); /* Enable bit 2 which specifies that the slave is on IRQ 2 */
	outb(PIC_SLAVE_DATAR, 0x2); /* Let the slave know it's on IRQ 2 (send binary number in bits 0-2) */

	/* Finally ICW4 (needed since we set the bit in ICW1) */
	outb(PIC_PRI_DATAR, 0x11); /* This enables x86 mode */
	outb(PIC_SLAVE_DATAR, 0x11);

	for (int i = 0; i < 16; i++)
		EnableIRQ(i);

	SetPriEOI();
	SetSlaveEOI();
}

/*
Interrupt Mask Register (IMR) / OCW 1
Bit Number	IRQ Number (Primary controller)	IRQ Number (Slave controller)
0			IRQ0							IRQ8
1			IRQ1							IRQ9
2			IRQ2							IRQ10
3			IRQ3							IRQ11
4			IRQ4							IRQ12
5			IRQ5							IRQ13
6			IRQ6							IRQ14
7			IRQ7							IRQ15
/*

/*
Operation Command Word (OCW) 2
Bit	Number	Value		Description
0-2			L0/L1/L2	Interrupt level upon which the controller must react
3-4			0			Reserved, must be 0
5			EOI			End of Interrupt (EOI) request
6			SL			Selection
7			R			Rotation option
*/

/*
OCW2 Commands for bits
R Bit	SL Bit	EOI Bit		Description
0		0		0			Rotate in Automatic EOI mode (CLEAR)
0		0		1			Non specific EOI command
0		1		0			No operation
0		1		1			Specific EOI command
1		0		0			Rotate in Automatic EOI mode (SET)
1		0		1			Rotate on non specific EOI
1		1		0			Set priority command
1		1		1			Rotate on specific EOI
*/

void SetPriEOI()
{
	outb(PIC_PRI_CMDR, 0x20); /* Set the EOI bit */
}

void SetSlaveEOI()
{
	outb(PIC_SLAVE_CMDR, 0x20); /* Set the EOI bit */
}

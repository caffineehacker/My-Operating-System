#include <cstdint>
#include "PIT8253.h"
#include "idt.h"
#include "ports.h"
#include "hal.h"
#include "console.h"
#include "interrupt.h"
#include "PIC8259A.h"

/* Global tick count */
static volatile uint32_t system_ticks = 0;
uint8_t pit_irq_num = 0;

/* PIT8253 interrupt handler */
void interrupt _cdecl pit8253_irq()
{
	INTERRUPT_START();

	system_ticks++;

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

uint32_t GetSystemTicks()
{
	return system_ticks;
}

void pit_initialize(uint8_t irq, uint8_t irCodeSeg)
{
	pit_irq_num = irq;
	IDT_InstallIR(irq, IDT_DESC_PRESENT | IDT_DESC_BIT32, irCodeSeg, pit8253_irq);
	EnableIRQ(irq);
}

void pit_start_counter0(uint32_t freq, uint8_t mode_mask)
{
	if (freq == 0)
		return;
 
	uint16_t divisor = (uint16_t)(1193180 / freq);

	uint8_t ocw = 0;
	ocw = (uint8_t)(mode_mask | PIT_CONTROLWORD_RLMODE_LSBMSB_MASK | PIT_CONTROLWORD_SELECTCOUNTER_0_MASK);
	outb(PIT_CONTROLWORD_ADDR, ocw);
 
	outb(PIT_COUNTER_0_ADDR, (uint8_t)(divisor & 0xff));
	outb(PIT_COUNTER_0_ADDR, (uint8_t)((divisor >> 8) & 0xff));

	/* Reset ticks */
	system_ticks=0;
}

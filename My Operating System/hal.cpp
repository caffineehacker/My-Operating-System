#include "idt.h"
#include "gdt.h"
#include "PIC8259A.h"
#include "PIT8253.h"
#include "hal.h"

/* Initialize HAL */
int _cdecl HAL_Initialize()
{
	GDT_Initialize();
	IDT_Initialize(0x08);
	InitializePIC();

	/* In InitializePIC I remap PIT to IRQ 0x20 */
	pit_initialize(0x20, 0x08);
	pit_start_counter0(1000, PIT_CONTROLWORD_OPERATINGMODE_RATEGEN_MASK);

	EnableInterrupts();

	return 0;
}

/* Shutdown HAL */
int _cdecl HAL_Shutdown()
{
	return 0;
}

inline void _cdecl HARDWARE_INTERRUPT_DONE(unsigned int intno)
{
	/* Check if this is a hardware interrupt, do nothing for software */
	if (intno > 16)
		return;

	//! test if we need to send end-of-interrupt to second pic
	if (intno >= 8)
		SetSlaveEOI();

	//! always send end-of-interrupt to primary pic
	SetPriEOI();
}

void _cdecl InstallInterruptHandler(uint8_t IRQNumber, void (_cdecl *handler)(void))
{
	IDT_InstallIR(IRQNumber, IDT_DESC_PRESENT | IDT_DESC_BIT32, 0x08, handler);
}

uint32_t _cdecl GetTickCount()
{
	return GetSystemTicks();
}

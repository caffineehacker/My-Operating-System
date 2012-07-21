#include "interrupt.h"
#include "hal.h"

void interrupt divide_by_zero_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt single_step_trap()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt nmi_trap()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt breakpoint_trap()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt overflow_trap()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt bounds_check_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt invalid_opcode_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt no_device_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt double_fault_abort()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt invalid_tss_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt no_segment_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt stack_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt general_protection_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt page_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt fpu_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt alignment_check_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt machine_check_abort()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

void interrupt simd_fpu_fault()
{
	INTERRUPT_START();

	/* TODO: Handle error */
	for(;;);

	HARDWARE_INTERRUPT_DONE(0x0);
	INTERRUPT_RETURN();
}

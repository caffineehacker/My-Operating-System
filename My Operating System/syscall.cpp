#include "idt.h"
#include "interrupt.h"
#include "hal.h"

void*(*SyscallFunctionHandlers[256])(void*);

void interrupt int_handler_81()
{
	INTERRUPT_START();

	int functionNumber;
	void* data;

	/* I enable interrupts here since this is a software interrupt handler */
	_asm
	{
		sti
		mov [functionNumber], eax
		mov [data], ebx
	}

	SyscallFunctionHandlers[functionNumber](data);

	INTERRUPT_END();
	INTERRUPT_RETURN_RETVAL();
}

void InitializeSyscallHandler()
{
	IDT_InstallIR(0x81, IDT_DESC_PRESENT | IDT_DESC_BIT32, 0x8, (IDT_IRQ_HANDLER)int_handler_81);
}

void SetSyscallFunctionHandler(void*(*function)(void*), int functionNumber)
{
	SyscallFunctionHandlers[functionNumber] = function;
}

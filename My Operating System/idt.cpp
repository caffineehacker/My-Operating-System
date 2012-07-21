#include <cstring>
#include "idt.h"

idt_descriptor _idt[IDT_MAX_INTERRUPTS];
idtr _idtr;

/* Load the IDT */
static void IDT_Install()
{
	_asm
	{
		lidt [_idtr]
	}
}

/* Default handler for interrupts, just hangs us */
void IDT_DefaultHandler()
{ 
	for(;;);
}

/* This is a hack and a half */
void GenerateInterrupt(int n)
{
	_asm {
		mov al, byte ptr [n]
		mov byte ptr [genint+1], al
		jmp genint
	genint:
		int 0	/* Above code modifies the 0 to the interrupt number to generate */
	}
}

/* Install a new interrupt routine/handler */
int IDT_InstallIR(uint32_t i, uint8_t flags, uint16_t sel, IDT_IRQ_HANDLER irq)
{
	if (i>IDT_MAX_INTERRUPTS)
		return 0;
 
	if (!irq) /* Fail if the callback method is null */
		return 0;
 
	/* Get base address of interrupt handler */
	uint32_t uiBase = (uint32_t)&(*irq);
 
	/* Store base address into IDT */
	_idt[i].baseLo = uiBase & 0xffff;
	_idt[i].baseHi = (uiBase >> 16) & 0xffff;
	_idt[i].reserved = 0;
	_idt[i].flags = flags;
	_idt[i].sel = sel;
 
	return	0;
}

/* Initialize the IDT */
int IDT_Initialize(uint16_t codeSel)
{
	/* Setup the IDT register variable */
	_idtr.limit = sizeof(struct idt_descriptor) * IDT_MAX_INTERRUPTS -1;
	_idtr.base	= (uint32_t)&_idt[0];
 
	/* Clear out the current IDT */
	memset((void*)&_idt[0], 0, sizeof (idt_descriptor) * IDT_MAX_INTERRUPTS-1);
 
	/* Register default handlers */
	for (uint32_t i = 0; i<IDT_MAX_INTERRUPTS; i++)
		IDT_InstallIR(i, IDT_DESC_PRESENT | IDT_DESC_BIT32, codeSel, (IDT_IRQ_HANDLER)IDT_DefaultHandler);
 
	/* Install our idt */
	IDT_Install();
 
	return 0;
}

#pragma once

#include <cstdint>

#pragma pack (push)
#pragma pack (1)

/* Interrupt descriptor */
struct idt_descriptor
{
	/* bits 0-16 of interrupt routine (ir) address */
	uint16_t		baseLo;
 
	/* code selector in gdt */
	uint16_t		sel;
 
	/* reserved, shold be 0 */
	uint8_t			reserved;
 
	/* bit flags. Set with flags above */
	uint8_t			flags;
 
	/* bits 16-32 of ir address */
	uint16_t		baseHi;
};

/* IDT register struct */
struct idtr
{
	/* size of the interrupt descriptor table (idt) */
	uint16_t		limit;
 
	/* base address of idt */
	uint32_t		base;
};

#define IDT_MAX_INTERRUPTS 256
#define IDT_DESC_BIT16 0x06
#define IDT_DESC_BIT32 0x0E
#define IDT_DESC_RING1 0x40
#define IDT_DESC_RING2 0x20
#define IDT_DESC_RING3 0x60
#define IDT_DESC_PRESENT 0x80

/* This is the typedef for interrupt routine/handler functions */
typedef void (_cdecl *IDT_IRQ_HANDLER)(void);

extern void GenerateInterrupt(int n);
extern int IDT_Initialize(uint16_t codeSel);
extern int IDT_InstallIR(uint32_t irqNumber, uint8_t flags, uint16_t sel, IDT_IRQ_HANDLER irq);

#define interrupt _declspec(naked)

#pragma pack (pop)

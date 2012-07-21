#include <cstring>
#include "gdt.h"

/* Global Descriptor Table (GDT) */
static struct gdt_descriptor			_gdt [GDT_MAX_DESCRIPTORS];
 
/* The GDTR to be passed to the processor */
static struct gdtr				_gdtr;

/* Install the GDT */
static void GDT_Install()
{
	_asm
	{
		lgdt [_gdtr]
	}
}

/* Add a new descriptor to the GDT */
void GDT_SetDescriptor(uint32_t i, uint64_t base, uint64_t limit, uint16_t flags)
{
	if (i > GDT_MAX_DESCRIPTORS)
		return;

	/* Zero out the current GDT entry */
	memset((void*)&_gdt[i], 0, sizeof (gdt_descriptor));

	/* Set limit and address */
	_gdt[i].baseLo	= base & 0xffff;
	_gdt[i].baseMid	= (base >> 16) & 0xff;
	_gdt[i].baseHi	= (base >> 24) & 0xff;
	_gdt[i].limit	= limit & 0xffff;

	_gdt[i].flags = flags;
}

int GDT_Initialize()
{
	/* Initialize GDT register variable */
	_gdtr.m_limit = (sizeof (struct gdt_descriptor) * GDT_MAX_DESCRIPTORS)-1;
	_gdtr.m_base = (uint32_t)&_gdt[0];
 
	/* Create null descriptor */
	GDT_SetDescriptor(0, 0, 0, 0);

	/* Set up code descriptor */
	GDT_SetDescriptor(1,0,0xffffffff,
		GDT_FLAGS_RW_BIT|GDT_FLAGS_EXECUTABLE_BIT|GDT_FLAGS_DESCRIPTOR_BIT|GDT_FLAGS_SEG_IN_MEM_BIT |
		GDT_FLAGS_GRANULARITY | GDT_FLAGS_SEG_TYPE | GDT_FLAGS_SEG_LIMITHI_MASK);
 
	/* Set up data descriptor */
	GDT_SetDescriptor(2,0,0xffffffff,
		GDT_FLAGS_RW_BIT|GDT_FLAGS_DESCRIPTOR_BIT|GDT_FLAGS_SEG_IN_MEM_BIT |
		GDT_FLAGS_GRANULARITY | GDT_FLAGS_SEG_TYPE | GDT_FLAGS_SEG_LIMITHI_MASK);
 
	GDT_Install();
 
	return 0;
}

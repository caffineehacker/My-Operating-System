#pragma once

#include <cstdint>

#define GDT_FLAGS_ACCESS_BIT 0x1
/* 0 = Read only/Execute only, 1 = RW/Read+Execute */
#define GDT_FLAGS_RW_BIT 0x2
#define GDT_FLAGS_EXPANSION_DIR_BIT 0x4
#define GDT_FLAGS_CONFORMING_BIT 0x4
/* 0 = data segment, 1 = code segment */
#define GDT_FLAGS_EXECUTABLE_BIT 0x8
/* 0 = System descriptor, 1 = code or data */
#define GDT_FLAGS_DESCRIPTOR_BIT 0x10
/* 0 = ring 0, 3 = ring 3 */
#define GDT_FLAGS_DESCRIPTOR_PRIV 0x40
#define GDT_FLAGS_SEG_IN_MEM_BIT 0x80
/* This is 4 bits */
#define GDT_FLAGS_SEG_LIMIT_NIBBLE 0x800
#define GDT_FLAGS_RESERVERD_BIT1 0x1000
#define GDT_FLAGS_RESERVERD_BIT2 0x2000
/* 0 = 16 bit, 1 = 32 bit */
#define GDT_FLAGS_SEG_TYPE 0x4000
/* 0 = Byte, 1 = 4K */
#define GDT_FLAGS_GRANULARITY 0x8000

//! masks out limitHi (High 4 bits of limit)
#define GDT_FLAGS_SEG_LIMITHI_MASK		0x0f00

#pragma pack (push, 1)

struct gdt_descriptor {
	/* Bits 0-15: Bits 0-15 of the Segment Limit */
	uint16_t		limit;
 
	/* Bits 16-39: Bits 0-23 of the Base Address */
	uint16_t		baseLo;
	uint8_t			baseMid;
 
	/* See above defines */
	uint16_t		flags;
 
	/* Bits 56-63: Bits 24-32 of the base address */
	uint8_t			baseHi;
};

struct gdtr
{
	/* Size of GDT */
	uint16_t		m_limit;
 
	/* Base address of GDT */
	uint32_t		m_base;
};

#define GDT_MAX_DESCRIPTORS 3
extern void GDT_Install();
extern int GDT_Initialize();

#pragma pack (pop, 1)

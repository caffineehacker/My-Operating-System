#pragma once

/* Blocks reference per byte in the bitmap (8 bits per byte) */
#define PMMNGR_BLOCKS_PER_BYTE 8
 
/* Size of a block */
#define PMMNGR_BLOCK_SIZE	4096
 
/* Block alignment (all blocks start at a multiple of this) */
#define PMMNGR_BLOCK_ALIGN	PMMNGR_BLOCK_SIZE

#pragma pack(push, 1)

struct memory_region
{
	uint32_t	startLo;
	uint32_t	startHi;
	uint32_t	sizeLo;
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
};

#pragma pack(pop,1)

extern void physical_memorymgr_init(size_t memSize, uint32_t bitmap);
extern void physical_memorymgr_initialize_region(uint32_t base, size_t size);
extern void physical_memorymgr_deinitialize_region(uint32_t base, size_t size);
extern uint32_t physical_memorymgr_get_free_block_count();
extern uint32_t physical_memorymgr_get_used_block_count();
extern uint32_t physical_memorymgr_get_block_count();
extern void* physical_memorymgr_allocate_block();
extern void* physical_memorymgr_allocate_blocks(size_t size);
extern void physical_memorymgr_free_block(void* p);
extern void physical_memorymgr_load_PDBR(uint32_t addr);
extern uint32_t physical_memorymgr_get_PDBR();
extern void physical_memorymgr_paging_enable(bool b);
extern void* physical_memorymgr_allocate_aligned_blocks(size_t size, size_t alignment);

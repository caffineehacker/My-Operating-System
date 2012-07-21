#include <cstdint>
#include <cstring>
#include "physical_memorymgr.h"

/* Size of physical memory in KB */
static uint32_t physical_memorymgr_memory_size = 0;
 
/* Number of physical blocks in use */
static uint32_t	physical_memorymgr_used_blocks = 0;
 
/* Max number of blocks available */
static uint32_t physical_memorymgr_max_blocks = 0;
 
/* Block in use bitmap.  We use 32-bit rather than 8-bit because this is more efficient with the process/memory */
static uint32_t* physical_memorymgr_memory_map = 0;

inline void physical_memorymgr_setBlockUsed(int blockNumber)
{
  physical_memorymgr_memory_map[blockNumber / 32] |= (1 << (blockNumber % 32));
}

inline void physical_memorymgr_setBlockFree(int blockNumber)
{
  physical_memorymgr_memory_map[blockNumber / 32] &= ~(1 << (blockNumber % 32));
}

inline bool physical_memorymgr_isBlockUsed(int blockNumber)
{
	return physical_memorymgr_memory_map[blockNumber / 32] & (1 << (blockNumber % 32));
}

uint32_t physical_memorymgr_get_free_block_count()
{
	return physical_memorymgr_max_blocks - physical_memorymgr_used_blocks;
}

uint32_t physical_memorymgr_get_used_block_count()
{
	return physical_memorymgr_used_blocks;
}

uint32_t physical_memorymgr_get_block_count()
{
	return physical_memorymgr_max_blocks;
}

int physical_memorymgr_firstFreeAlignedBlock(size_t alignment)
{
	/* Find first 0'd bit */
	for (uint32_t i=0; i< physical_memorymgr_max_blocks / 32; i++)
	{
		if (physical_memorymgr_memory_map[i] != 0xffffffff)
		{
			for (int j=0; j<32; j++)
			{ /* Test each bit in the dword */
				int bit = 1 << j;
				if (!(physical_memorymgr_memory_map[i] & bit) && ((i*32 + j + 1) % alignment) == 0) /* Verify the bit is free and aligned */
					return (int)(i*32 + j);
			}
		}
	}
 
	return -1;
}

int physical_memorymgr_firstFreeBlock()
{
	return physical_memorymgr_firstFreeAlignedBlock(1);
}

inline bool IsPageInUse(int pageNumber)
{
	return physical_memorymgr_memory_map[pageNumber / 32] & (1 << (pageNumber % 32));
}

int physical_memorymgr_firstFreeAlignedBlockArray(size_t size, size_t alignment)
{
	if (size==0)
		return -1;

	if (size==1)
		return physical_memorymgr_firstFreeAlignedBlock(alignment);

	for (uint32_t i = 0; i < physical_memorymgr_get_block_count(); i++)
		if (physical_memorymgr_memory_map[i] != 0xffffffff)
			for (int j = 0; j < 32; j++)
			{ /* Test each bit in the dword */
				int bit = 1<<j;
				if (!(physical_memorymgr_memory_map[i] & bit) && ((i*32 + j + 1) % alignment) == 0)
				{
					int startingBit = i*32;
					startingBit += bit; /* Get the free bit in the dword at index i */

					uint32_t free = 0; /* Loop through each bit to see if its enough space */
					for (uint32_t count = 0; count <= size; count++)
					{
						if (!IsPageInUse(startingBit+count))
							free++;	/* This is not in use */

						if (free == size)
							return i*32 + j; /* free count == size needed; return index */
					}
				}
			}

	return -1;
}

int physical_memorymgr_firstFreeBlockArray(size_t size)
{
	return physical_memorymgr_firstFreeAlignedBlockArray(size, 1);
}

void physical_memorymgr_init(size_t memSize, uint32_t bitmap)
{
	physical_memorymgr_memory_size = memSize;
	physical_memorymgr_memory_map = (uint32_t*)bitmap;
	physical_memorymgr_max_blocks = (physical_memorymgr_memory_size*1024) / PMMNGR_BLOCK_SIZE;
	physical_memorymgr_used_blocks = physical_memorymgr_max_blocks;
 
	/* By default, all of memory is in use */
	memset(physical_memorymgr_memory_map, 0xf, physical_memorymgr_max_blocks / PMMNGR_BLOCKS_PER_BYTE );
}

void physical_memorymgr_initialize_region(uint32_t base, size_t size)
{
	int align = base / PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;

	for (; blocks>0; blocks--) {
		physical_memorymgr_setBlockFree(align++);
		physical_memorymgr_used_blocks--;
	}

	physical_memorymgr_setBlockUsed(0);	/* first block is always set. This blocks using null (0) as a valid address */
}

void physical_memorymgr_deinitialize_region(uint32_t base, size_t size)
{
	int align = base / PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;

	for (; blocks>0; blocks--) {
		physical_memorymgr_setBlockUsed(align++);
		physical_memorymgr_used_blocks++;
	}
}

void* physical_memorymgr_allocate_block()
{
 
	if (physical_memorymgr_get_free_block_count() <= 0)
		return 0;	/* out of memory */
 
	int frame = physical_memorymgr_firstFreeBlock();
 
	if (frame == -1)
		return 0;	//out of memory
 
	physical_memorymgr_setBlockUsed(frame);
 
	uint32_t addr = frame * PMMNGR_BLOCK_SIZE;
	physical_memorymgr_used_blocks++;
 
	return (void*)addr;
}

void physical_memorymgr_free_block(void* p)
{
	uint32_t addr = (uint32_t)p;
	int frame = addr / PMMNGR_BLOCK_SIZE;
 
	physical_memorymgr_setBlockFree(frame);
 
	physical_memorymgr_used_blocks--;
}

void* physical_memorymgr_allocate_aligned_blocks(size_t size, size_t alignment)
{
	if (physical_memorymgr_get_free_block_count() <= size)
		return 0; /* Not enough space */

	int frame = physical_memorymgr_firstFreeAlignedBlockArray(size, alignment);

	if (frame == -1)
		return 0; /* Not enough space */

	for (uint32_t i = 0; i < size; i++)
		physical_memorymgr_setBlockUsed(frame+i);

	uint32_t addr = frame*PMMNGR_BLOCK_SIZE;
	physical_memorymgr_used_blocks += size;

	return (void*)addr;
}

void* physical_memorymgr_allocate_blocks(size_t size)
{
	return physical_memorymgr_allocate_aligned_blocks(size, 1);
}

void physical_memorymgr_paging_enable(bool b)
{
	_asm
	{
		mov	eax, cr0
		cmp [b], 1
		je	enable
		jmp disable
enable:
		or eax, 0x80000000		//set bit 31
		mov	cr0, eax
		jmp done
disable:
		and eax, 0x7FFFFFFF		//clear bit 31
		mov	cr0, eax
done:
	}
}

void physical_memorymgr_load_PDBR(uint32_t addr)
{
	_asm
	{
		mov	eax, [addr]
		mov	cr3, eax		// PDBR is cr3 register in i86
	}
}

uint32_t physical_memorymgr_get_PDBR()
{
	_asm
	{
		mov	eax, cr3
		ret
	}
}

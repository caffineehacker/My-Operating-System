#pragma once

#include <cstdint>

#define PAGETABLE_ENTRY_PRESENT_MASK 0x1
#define PAGETABLE_ENTRY_WRITABLE_MASK 0x2
#define PAGETABLE_ENTRY_USER_MASK 0x4
#define PAGETABLE_ENTRY_WRITETHOUGH_MASK 0x8
#define PAGETABLE_ENTRY_NOT_CACHEABLE_MASK 0x10
#define PAGETABLE_ENTRY_ACCESSED_MASK 0x20
#define PAGETABLE_ENTRY_DIRTY_MASK 0x40
#define PAGETABLE_ENTRY_PAT_MASK 0x80
#define PAGETABLE_ENTRY_CPU_GLOBAL_MASK 0x100
#define PAGETABLE_ENTRY_LV4_GLOBAL_MASK 0x200
#define PAGETABLE_ENTRY_FRAME_MASK 0x7FFFF000

typedef uint32_t pagetable_entry;

#define PAGEDIRECTORY_ENTRY_PRESENT_MASK 0x1
#define PAGEDIRECTORY_ENTRY_WRITABLE_MASK 0x2
#define PAGEDIRECTORY_ENTRY_USER_MASK 0x4
#define PAGEDIRECTORY_ENTRY_PWT_MASK 0x8
#define PAGEDIRECTORY_ENTRY_PCD_MASK 0x10
#define PAGEDIRECTORY_ENTRY_ACCESSED_MASK 0x20
#define PAGEDIRECTORY_ENTRY_DIRTY_MASK 0x40
#define PAGEDIRECTORY_ENTRY_4MB_MASK 0x80
#define PAGEDIRECTORY_ENTRY_CPU_GLOBAL_MASK 0x100
#define PAGEDIRECTORY_ENTRY_LV4_GLOBAL_MASK 0x200
#define PAGEDIRECTORY_ENTRY_FRAME_MASK 0x7FFFF000

typedef uint32_t pagedirectory_entry;

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

#define pagetable_entry_set_frame(pEntry, addr) *pEntry = (*pEntry & ~PAGETABLE_ENTRY_FRAME_MASK) | addr
#define pagetable_entry_add_attrib(pEntry, attrib) *pEntry |= attrib
#define pagetable_entry_del_attrib(pEntry, attrib) *pEntry &= ~attrib
#define pagetable_entry_frame(pEntry) (*e & PAGETABLE_ENTRY_FRAME_MASK)

#define pagedirectory_entry_set_frame(pEntry, addr) *pEntry = (*pEntry & ~PAGEDIRECTORY_ENTRY_FRAME_MASK) | addr
#define pagedirectory_entry_add_attrib(pEntry, attrib) *pEntry |= attrib
#define pagedirectory_entry_del_attrib(pEntry, attrib) *pEntry &= ~attrib
#define pagedirectory_entry_frame(pEntry) (*e & PAGEDIRECTORY_ENTRY_FRAME_MASK)

struct pagetable
{
	pagetable_entry m_entries[PAGES_PER_TABLE];
};

struct pagedirectory
{
	pagedirectory_entry m_entries[PAGES_PER_DIR];
};

extern bool vmmngr_alloc_page(pagetable_entry* e);
extern void vmmngr_initialize(uint32_t currentPhysicalBase, uint32_t currentVirtualBase);
extern void vmmngr_map_page(void* phys, void* virt);

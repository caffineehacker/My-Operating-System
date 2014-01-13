#include "pagetable.h"
#include "physical_memorymgr.h"
#include <cstdint>
#include <cstring>

uint32_t _cur_pdbr = 0;

bool vmmngr_alloc_page(pagetable_entry* e)
{
	/* Allocate a free physical frame */
	void* p = physical_memorymgr_allocate_block();
	if (!p)
		return false;
 
	/* Map it to the page */
	pagetable_entry_set_frame(e, (uint32_t)p);
	pagetable_entry_add_attrib(e, PAGETABLE_ENTRY_PRESENT_MASK);
 
	return true;
}

void vmmngr_free_page(pagetable_entry* e)
{
	void* p = (void*)pagetable_entry_frame(e);
	if (p)
		physical_memorymgr_free_block(p);
 
	pagetable_entry_del_attrib(e, PAGEDIRECTORY_ENTRY_PRESENT_MASK);
}

pagetable_entry* vmmngr_pagetable_lookup_entry(pagetable* ptable, uint32_t addr) {
 
	if (ptable)
		return &ptable->m_entries[PAGE_TABLE_INDEX(addr)];
	return 0;
}

pagedirectory* _cur_directory = 0;
 
inline bool vmmngr_switch_pdirectory(pagedirectory* dir)
{
	if (!dir)
		return false;

	_cur_directory = dir;
	physical_memorymgr_load_PDBR(_cur_pdbr);
	return true;
}
 
pagedirectory* vmmngr_get_directory()
{
	return _cur_directory;
}

void vmmngr_flush_tlb_entry(uint32_t addr)
{
	_asm
	{
		cli
		invlpg addr
		sti
	}
}

/* Set a virtual page to map to a specific physical page */
void vmmngr_map_page(void* phys, void* virt)
{
	/* Get page directory */
	pagedirectory* pageDirectory = vmmngr_get_directory();

	/* Get page table for this virtual address */
	pagedirectory_entry* e = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

	/* Ensure that the table is present in memory */
	if ((*e & PAGEDIRECTORY_ENTRY_PRESENT_MASK) != PAGEDIRECTORY_ENTRY_PRESENT_MASK)
	{
		/* Page table not present, allocate it */
		pagetable* table = (pagetable*)physical_memorymgr_allocate_block();
		if (!table)
			return; /* Error: TODO: Handle the error */

		/* Clear page table */
		memset(table, 0, sizeof(pagetable));

		/* Create a new entry */
		pagedirectory_entry* entry = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

		/* Map in the table (Can also just do *entry |= 3) to enable these bits */
		pagedirectory_entry_add_attrib(entry, PAGEDIRECTORY_ENTRY_PRESENT_MASK);
		pagedirectory_entry_add_attrib(entry, PAGEDIRECTORY_ENTRY_WRITABLE_MASK);
		pagedirectory_entry_set_frame(entry, (uint32_t)table);
	}

	/* Get the table */
	pagetable* table = (pagetable*)PAGE_GET_PHYSICAL_ADDRESS(e);

	/* Get the page */
	pagetable_entry* page = &table->m_entries[PAGE_TABLE_INDEX((uint32_t)virt)];

	/* Map the address (Can also do (*page |= 3 to enable..) */
	pagetable_entry_set_frame(page, (uint32_t)phys);
	pagetable_entry_add_attrib(page, PAGETABLE_ENTRY_PRESENT_MASK);
}

void vmmngr_initialize(uint32_t currentPhysicalBase, uint32_t currentVirtualBase)
{
	/* Allocate default page table */
	pagetable* table = (pagetable*)physical_memorymgr_allocate_block();
	if (!table)
		return;
 
	///* Allocates a 4MB identity page table */
	pagetable* table2 = (pagetable*)physical_memorymgr_allocate_block();
	if (!table2)
		return;

	/* Clear page table */
	memset(table, 0, sizeof(pagetable));
	memset(table2, 0, sizeof(pagetable));

	/* 1st 4mb are idenitity mapped */
	for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096)
	{
 		/* Create a new page */
		pagetable_entry page = 0;
		pagetable_entry_add_attrib(&page, PAGETABLE_ENTRY_PRESENT_MASK);
 		pagetable_entry_set_frame(&page, frame);

		/* And add it to the page table */
		table2->m_entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	/* Map current code to virtual address */
	for (int i = 0, frame = currentPhysicalBase, virt = currentVirtualBase; i<1024; i++, frame+=4096, virt+=4096)
	{
		/* Create a new page */
		pagetable_entry page = 0;
		pagetable_entry_add_attrib(&page, PAGETABLE_ENTRY_PRESENT_MASK);
		pagetable_entry_set_frame(&page, frame);

		/* And add it to the page table */
		table->m_entries[PAGE_TABLE_INDEX(virt)] = page;
	}

	/* Create default directory table */
	pagedirectory* dir = (pagedirectory*)physical_memorymgr_allocate_blocks(3);
	if (!dir)
		return;
 
	/* Clear directory table and set it as current */
	memset(dir, 0, sizeof(pagedirectory));

	pagedirectory_entry* entry = &dir->m_entries[PAGE_DIRECTORY_INDEX(currentVirtualBase)];
	pagedirectory_entry_add_attrib(entry, PAGEDIRECTORY_ENTRY_PRESENT_MASK);
	pagedirectory_entry_add_attrib(entry, PAGEDIRECTORY_ENTRY_WRITABLE_MASK);
	pagedirectory_entry_set_frame(entry, (uint32_t)table);

	///* 4MB identity directory */
	pagedirectory_entry* entry2 = &dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];
	pagedirectory_entry_add_attrib(entry2, PAGEDIRECTORY_ENTRY_PRESENT_MASK);
	pagedirectory_entry_add_attrib(entry2, PAGEDIRECTORY_ENTRY_WRITABLE_MASK);
	pagedirectory_entry_set_frame(entry2, (uint32_t)table2);

	/* Store current PDBR */
	_cur_pdbr = (uint32_t)&dir->m_entries;

	/* Switch to our page directory */
	vmmngr_switch_pdirectory(dir);
 
	/* Enable paging */
	physical_memorymgr_paging_enable(true);
}

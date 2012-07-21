#include <cstdio>
#include "idt.h"
#include "hal.h"
#include "syscall.h"
#include "console.h"
#include "processor_exceptions.h"
#include "PIC8259A.h"
#include "multiboot.h"
#include "physical_memorymgr.h"
#include "pagetable.h"
#include "floppy.h"
#include "filesystem.h"

#define VID_MEMORY 0xB8000

#pragma warning (disable:4244)

/* Memory type strings for memory_region.type */
const char* strMemoryTypes[] = {
	{"Available"},			/* memory_region.type==0 */
	{"Reserved"},			/* memory_region.type==1 */
	{"ACPI Reclaim"},		/* memory_region.type==2 */
	{"ACPI NVS Memory"}		/* memory_region.type==3 */
};

void __cdecl main(multiboot_info* bootinfo)
{
	/* Get kernel size passed from boot loader */
	uint32_t kernelSize = 0;
	_asm
	{
		mov word ptr [kernelSize], dx
	}

	vmmngr_initialize(0x000000, 0x000000);
	
	/* Initialize the console */
	InitializeConsole();
	ConsoleSetColor(0x3F);
	HAL_Initialize();
	InitializeSyscallHandler();

	/* Get memory size in KB */
	uint32_t memSize = 1024 + bootinfo->memorySize;

	/* Place the physical memory bitmap at the end of the kernel */
	physical_memorymgr_init(memSize, 0x100000 + kernelSize*512);

	/* Enable interrupts and install exception handlers */
	EnableInterrupts();
	InstallInterruptHandler(0,(void (__cdecl &)(void))divide_by_zero_fault);
	InstallInterruptHandler (1,(void (__cdecl &)(void))single_step_trap);
	InstallInterruptHandler (2,(void (__cdecl &)(void))nmi_trap);
	InstallInterruptHandler(3,(void (__cdecl &)(void))breakpoint_trap);
	InstallInterruptHandler(4,(void (__cdecl &)(void))overflow_trap);
	InstallInterruptHandler(5,(void (__cdecl &)(void))bounds_check_fault);
	InstallInterruptHandler(6,(void (__cdecl &)(void))invalid_opcode_fault);
	InstallInterruptHandler(7,(void (__cdecl &)(void))no_device_fault);
	InstallInterruptHandler(8,(void (__cdecl &)(void))double_fault_abort);
	InstallInterruptHandler(10,(void (__cdecl &)(void))invalid_tss_fault);
	InstallInterruptHandler(11,(void (__cdecl &)(void))no_segment_fault);
	InstallInterruptHandler(12,(void (__cdecl &)(void))stack_fault);
	InstallInterruptHandler(13,(void (__cdecl &)(void))general_protection_fault);
	InstallInterruptHandler(14,(void (__cdecl &)(void))page_fault);
	InstallInterruptHandler(16,(void (__cdecl &)(void))fpu_fault);
	InstallInterruptHandler(17,(void (__cdecl &)(void))alignment_check_fault);
	InstallInterruptHandler(18,(void (__cdecl &)(void))machine_check_abort);
	InstallInterruptHandler(19,(void (__cdecl &)(void))simd_fpu_fault);

	//physical_memorymgr_init(bootinfo->memorySize, 0xC0000000 + kernelSize*512);

	/* Clear the console */
	ConsoleClrScr(0x13);
	ConsoleGotoXY(0, 0);
	ConsoleSetColor(0x3F);
	printf("\t\t\tPhysical Memory Map\n\n\0");
	ConsoleSetColor(0x17);

	fprintf(stdout, "pmm initialized with %i KB physical memory; memLo: %i memHi: %i\n\n", memSize, bootinfo->memorySize & 0xFFFFFFFF, bootinfo->memorySize >> 32);
	stdout->flushFunc();

	ConsoleSetColor(0x19);
	printf("Physical Memory Map:\n");

	memory_region* region = (memory_region*)0x1000;

	for (uint32_t i = 0; i < 15; i++)
	{
		/* Sanity check; if type is > 4 mark it reserved */
		if (region[i].type > 4)
			region[i].type = 1;

		/* If start address is 0, there is no more entries, break out */
		if (i > 0 && region[i].startLo == 0)
			break;

		/* Display entry */
		fprintf(stdout, "region %i: start: 0x%x%x length (bytes): 0x%x%x type: %i (%s)\n", i, region[i].startHi, region[i].startLo, region[i].sizeHi, region[i].sizeLo, region[i].type, strMemoryTypes[region[i].type-1]);

		/* If region is avilable memory, initialize the region for use */
		if (region[i].type == 1)
			physical_memorymgr_initialize_region(region[i].startLo, region[i].sizeLo);
	}

	/* Reserve the region the kernel is in */
	physical_memorymgr_deinitialize_region(0x100000, kernelSize*512);
	ConsoleSetColor(0x17);

	printf("\npmm regions initialized: %i allocation blocks; used or reserved blocks: %i\nfree blocks: %i\n", physical_memorymgr_get_block_count(),  physical_memorymgr_get_used_block_count(), physical_memorymgr_get_free_block_count() );
	stdout->flushFunc();

	flpydsk_set_working_drive(0);
	flpydsk_install(0x26); /* 0x20 + 0x06 */
	flpydsk_mount_filesystem();

	printf("Calling read sector\n");
	stdout->flushFunc();
	flpydsk_read_sector(10);
	printf("Called\n");
	stdout->flushFunc();

	if (volOpenFile("a:KERNEL.EXE").flags & FS_INVALID)
	{
		printf("\n\nERROR! Can't find KERNEL.EXE");
	}
	else
	{
		printf("\n\nFOUND IT");
	}

	stdout->flushFunc();

	//! allocating and deallocating memory examples...

	/*DebugSetColor (0x12);

	uint32_t* p = (uint32_t*)pmmngr_alloc_block ();
	DebugPrintf ("\np allocated at 0x%x", p);

	uint32_t* p2 = (uint32_t*)pmmngr_alloc_blocks (2);
	DebugPrintf ("\nallocated 2 blocks for p2 at 0x%x", p2);

	pmmngr_free_block (p);
	p = (uint32_t*)pmmngr_alloc_block ();
	DebugPrintf ("\nUnallocated p to free block 1. p is reallocated to 0x%x", p);

	pmmngr_free_block (p);
	pmmngr_free_blocks (p2, 2);*/
}

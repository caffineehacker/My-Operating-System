#include <crt.h>
#include "multiboot.h"
#include "pagetable.h"
#include "hal.h"
#include "physical_memorymgr.h"

#ifdef ONEMB_STUB
#include "floppy.h"
#include "PIC8259A.h"
#include "filesystem.h"
#include "..\Fat12\fat12.h"
#include "portableexecutable.h"
#include "syscall.h"
#endif

extern void main(multiboot_info* bootinfo);

void __cdecl kernel_entry(multiboot_info* bootinfo)
{
#ifdef ARCH_X86
_asm { /* Set up the segments and stack */
	cli
	mov ax, 10h /* Use the data selector */
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov eax, bootinfo ; Move the argument to the new stack
	mov esp, 0x90000 /* Stack base address */
	
	mov ebp, esp /* Store this address for ret commands */
	sub esp, 8 ; Make sure we still have room for bootinfo
	push ebp

	mov bootinfo, eax
}
#endif

	_CallGlobalConstructors();

#ifndef ONEMB_STUB
	main(bootinfo);

	/* Call dtor's */
	_exit();
#else
	uint32_t kernelSize = 0;
	_asm
	{
		mov word ptr [kernelSize], dx
	}

	/* Get memory size in KB */
	uint32_t memSize = 1024 + (uint32_t)bootinfo->memorySize;
	/* Place the physical memory bitmap at the end of the kernel */
	physical_memorymgr_init(memSize, 0x100000 + kernelSize*512);

	memory_region* region = (memory_region*)0x1000;
	for (uint32_t i = 0; i < 15; i++)
	{
		/* Sanity check; if type is > 4 mark it reserved */
		if (region[i].type > 4)
			region[i].type = 1;

		/* If start address is 0, there is no more entries, break out */
		if (i > 0 && region[i].startLo == 0)
			break;

		/* If region is avilable memory, initialize the region for use */
		if (region[i].type == 1)
			physical_memorymgr_initialize_region(region[i].startLo, region[i].sizeLo);
	}

	/* Reserve the region the kernel is in */
	physical_memorymgr_deinitialize_region(0x100000, kernelSize*512);

	HAL_Initialize();
	InitializeSyscallHandler();

	/* This code just sets up basic virtual memory so we can jump to a virtual address of 3GB */
	vmmngr_initialize(0x100000, 0xc0000000);

	EnableInterrupts();

	flpydsk_set_working_drive(0);
	flpydsk_install(0x26); /* 0x20 + 0x06 */
	flpydsk_mount_filesystem();

	FILE kernl = volOpenFile("a:KERNEL.EXE");

	if (kernl.flags & FS_INVALID == FS_INVALID)
	{
		_asm
		{
			cli
			hlt
		}
	}
	
	/* TODO: Get a mechanism for getting a file length */
	void* kernelAddress = physical_memorymgr_allocate_blocks(((PFAT12_FILE_ENTRY_RAW)kernl.tag)->FileSize / PMMNGR_BLOCK_SIZE);
	//void* kernelAddress = (void*)0x3000000;

	_asm
		{
			cli
			hlt
			hlt
		}

	/* Map the kernel address to 3GB */
	vmmngr_map_page(kernelAddress, (void*)0xC0000000);

	/* Read the file to this new virtual address space */
	//volReadFile(&kernl, (unsigned char*)0xC0000000, ((PFAT12_FILE_ENTRY_RAW)kernl.tag)->FileSize);
	volReadFile(&kernl, (unsigned char *)kernelAddress, ((PFAT12_FILE_ENTRY_RAW)kernl.tag)->FileSize);

	_asm { cli };

	/* Code, stack and everything disappears once we move to the next stage */
	//PIMAGE_FILE_HEADER imgFileHeader = (PIMAGE_FILE_HEADER)(((PIMAGE_DOS_HEADER)0xC0000000)->e_lfanew + 0xC0000000 + 0x04);
	PIMAGE_FILE_HEADER imgFileHeader = (PIMAGE_FILE_HEADER)(((PIMAGE_DOS_HEADER)kernelAddress)->e_lfanew + (uint32_t)kernelAddress + 0x04);
	PIMAGE_OPTIONAL_HEADER imgOptHeader = (PIMAGE_OPTIONAL_HEADER)(imgFileHeader + sizeof(imgFileHeader));

	*((char*)0x2000000) = 50;
	
	//void* kernelEntryAddr = (void*)(imgOptHeader->AddressOfEntryPoint + 0xC0000000);
	void* kernelEntryAddr = (void*)(imgOptHeader->AddressOfEntryPoint + (uint32_t)kernelAddress);
	_asm
	{
		mov dx, word ptr [kernelSize]
		mov eax, [kernelEntryAddr]
		mov ecx, [bootinfo]
		push ecx
			hlt
		jmp eax
	}
#endif

	for (;;)
		_asm { hlt };
#ifdef ARCH_X86
	_asm cli
#endif
	for (;;);
}
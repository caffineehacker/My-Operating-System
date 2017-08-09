#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct multiboot_info
{
	uint32_t flags;
	uint64_t memorySize;
	uint32_t bootDevice; /* boot device. Present if flags[1] is set */
	uint32_t cmdLine; /* kernel command line. Present if flags[2] is set */
	uint32_t mods_count; /* number of modules loaded along with kernel. present if flags[3] is set */
	uint32_t mods_addr;
	uint32_t syms0; /* symbol table info. present if flags[4] or flags[5] is set */
	uint32_t syms1;
	uint32_t syms2;
	uint32_t mmap_length; /* memory map. Present if flags[6] is set */
	uint32_t mmap_addr;
	uint32_t drives_length; /* phys address of first drive structure. present if flags[7] is set */
	uint32_t drives_addr;
	uint32_t config_table; /* ROM configuation table. present if flags[8] is set */
	uint32_t bootloader_name; /* Bootloader name. present if flags[9] is set */
	uint32_t apm_table; /* advanced power management (apm) table. present if flags[10] is set */
	uint32_t vbe_control_info; /* video bios extension (vbe). present if flags[11] is set */
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
};

#pragma pack(pop,1)

#pragma once

#include <cstdint>
#include "..\My Operating System\filesystem.h"

#pragma pack(push, 1)

typedef struct _FAT12_BIOS_PARAMATER_BLOCK
{
	// Typical floppy values in comments
	uint8_t OEMName[8];
	uint16_t BytesPerSector; // 0x200
	uint8_t SectorsPerCluster; // 0x1
	uint16_t ReservedSectors; // 0x1
	uint8_t NumberOfFats; // 0x2
	uint16_t NumDirEntries; // 0xE0
	uint16_t NumSectors; // 0xB40
	uint8_t Media; // 0xF8
	uint16_t SectorsPerFat; // 0x9
	uint16_t SectorsPerTrack; // 0x12
	uint16_t HeadsPerCyl; // 0x2
	uint32_t HiddenSectors; // 0
	uint32_t LongSectors; // 0
} FAT12_BIOSPARAMATERBLOCK, *PFAT12_BIOSPARAMATERBLOCK;

typedef struct _FAT12_BIOS_PARAMATER_BLOCK_EXT
{
	uint32_t SectorsPerFat32; /* Sectors per FAT */
	uint16_t Flags; /* Flags */
	uint16_t Version; /* Version */
	uint32_t RootCluster; /* Starting root directory */
	uint16_t InfoCluster;
	uint16_t BackupBoot; /* Location of bootsector copy */
	uint16_t Reserved[6];
} FAT12_BIOSPARAMATERBLOCKEXT, *PFAT12_BIOSPARAMATERBLOCKEXT;

/* First 512 bytes of a partition */
typedef struct _FAT12_BOOT_SECTOR
{
	uint8_t Ignore[3]; /* First 3 bytes are ignored (our jmp instruction) */
	FAT12_BIOSPARAMATERBLOCK Bpb; /* BPB structure */
	FAT12_BIOSPARAMATERBLOCKEXT BpbExt; /* Extended BPB info */
	uint8_t Filler[448]; /* Needed to make struct 512 bytes */
} FAT12_BOOTSECTOR, *PFAT12_BOOTSECTOR;

typedef struct _FAT12_MOUNT_INFO
{
	uint32_t numSectors;
	uint32_t fatOffset;
	uint32_t numRootEntries;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint32_t fatSize;
	uint32_t fatEntrySize;
} FAT12_MOUNT_INFO, *PFAT12_MOUNT_INFO;

/* Attributes for FAT12 entries:
 * Read only: 1
 * Hidden: 2
 * System: 4
 * Volume Lable: 8
 * Subdirectory: 0x10
 * Archive: 0x20
 * Device: 0x60
 *
 *
 * For date:
 * Bits 0-4: Day (0-31)
 * Bits 5-8: Month (0-12)
 * Bits 9-15: Year
 *
 *
 * For time:
 * Bits 0-4: Second
 * Bits 5-10: Minute
 * Bits 11-15: Hour
 */

/* TODO: Create a general directory struct that isn't FS specific */
typedef struct _FAT12_FILE_ENTRY_RAW
{
	uint8_t Filename[8]; /* Filename */
	uint8_t Ext[3]; /* Extension (8.3 filename format) */
	uint8_t Attrib; /* File attributes */
	uint8_t Reserved;
	uint8_t TimeCreatedMs; /* Creation time */
	uint16_t TimeCreated;
	uint16_t DateCreated; /* Creation date */
	uint16_t DateLastAccessed;
	uint16_t FirstClusterHiBytes;
	uint16_t LastModTime; /* Last modification date/time */
	uint16_t LastModDate;
	uint16_t FirstCluster;          /* First cluster of file data */
	uint32_t FileSize; /* Size in bytes */
} FAT12_FILE_ENTRY_RAW, *PFAT12_FILE_ENTRY_RAW;

typedef struct _FAT12_FILE_ENTRY
{
	uint8_t Filename[8]; /* Filename */
	uint8_t Ext[3]; /* Extension (8.3 filename format) */
	uint8_t Attrib; /* File attributes */
	uint8_t Reserved;
	uint8_t TimeCreatedMs; /* Creation time */
	uint16_t TimeCreated;
	uint16_t DateCreated; /* Creation date */
	uint16_t DateLastAccessed;
	uint16_t FirstClusterHiBytes;
	uint16_t LastModTime; /* Last modification date/time */
	uint16_t LastModDate;
	uint16_t FirstCluster;          /* First cluster of file data */
	uint32_t FileSize; /* Size in bytes */
	uint32_t currentCluster; /* The current cluster, used by the FILE descriptors */
} FAT12_FILE_ENTRY, *PFAT12_FILE_ENTRY;

extern PFILESYSTEM fat12_mount(PFAT12_BOOTSECTOR bootsector);

#pragma pack(pop, 1)

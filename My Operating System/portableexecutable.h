#pragma once
#include <cstdint>

typedef struct _IMAGE_DOS_HEADER
{  /* DOS .EXE header */
    uint16_t e_magic;         /* Magic number (Should be MZ) */
    uint16_t e_cblp;          /* Bytes on last page of file */
    uint16_t e_cp;            /* Pages in file */
    uint16_t e_crlc;          /* Relocations */
    uint16_t e_cparhdr;       /* Size of header in paragraphs */
    uint16_t e_minalloc;      /* Minimum extra paragraphs needed */
    uint16_t e_maxalloc;      /* Maximum extra paragraphs needed */
    uint16_t e_ss;            /* Initial (relative) SS value */
    uint16_t e_sp;            /* Initial SP value */
    uint16_t e_csum;          /* Checksum */
    uint16_t e_ip;            /* Initial IP value */
    uint16_t e_cs;            /* Initial (relative) CS value */
    uint16_t e_lfarlc;        /* File address of relocation table */
    uint16_t e_ovno;          /* Overlay number */
    uint16_t e_res[4];        /* Reserved words */
    uint16_t e_oemid;         /* OEM identifier (for e_oeminfo) */
    uint16_t e_oeminfo;       /* OEM information e_oemid specific */
    uint16_t e_res2[10];      /* Reserved words */
    uint32_t e_lfanew;        /* File address of new exe header */
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER
{
    uint16_t  Machine;
    uint16_t  NumberOfSections;
    uint32_t   TimeDateStamp;
    uint32_t   PointerToSymbolTable;
    uint32_t   NumberOfSymbols;
    uint16_t  SizeOfOptionalHeader;
    uint16_t  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER
{
    //
    // Standard fields.
    //
    uint16_t  Magic;
    char   MajorLinkerVersion;
    char   MinorLinkerVersion;
    uint32_t   SizeOfCode;
    uint32_t   SizeOfInitializedData;
    uint32_t   SizeOfUninitializedData;
    uint32_t   AddressOfEntryPoint;
    uint32_t   BaseOfCode;
    uint32_t   BaseOfData;
    //
    // NT additional fields.
    //
    uint32_t   ImageBase;
    uint32_t   SectionAlignment;
    uint32_t   FileAlignment;
    uint16_t  MajorOperatingSystemVersion;
    uint16_t  MinorOperatingSystemVersion;
    uint16_t  MajorImageVersion;
    uint16_t  MinorImageVersion;
    uint16_t  MajorSubsystemVersion;
    uint16_t  MinorSubsystemVersion;
    uint32_t   Reserved1;
    uint32_t   SizeOfImage;
    uint32_t   SizeOfHeaders;
    uint32_t   CheckSum;
    uint16_t  Subsystem;
    uint16_t  DllCharacteristics;
    uint32_t   SizeOfStackReserve;
    uint32_t   SizeOfStackCommit;
    uint32_t   SizeOfHeapReserve;
    uint32_t   SizeOfHeapCommit;
    uint32_t   LoaderFlags;
    uint32_t   NumberOfRvaAndSizes;
    /*IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];*/
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

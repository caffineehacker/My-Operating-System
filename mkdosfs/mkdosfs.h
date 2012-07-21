
// set it to 1 (align on byte)
#pragma pack(push)
#pragma pack (1)

char strtstr[] = "\n\nMTOOLS   Make DOS Image  v00.14.15    Forever Young Software 1984-2009\n\n";

// http://www.geocities.com/thestarman3/asm/mbr/PartTables.htm#Decoding
struct PART_TBLE {
  unsigned  char bi;
  unsigned  char s_head;    // 8 bit head count
  unsigned  char s_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  unsigned  char s_cyl;     // bottom 8 bits
  unsigned  char si;
  unsigned  char e_head;    // 8 bit head count
  unsigned  char e_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  unsigned  char e_cyl;     // bottom 8 bits
  unsigned  long startlba;
  unsigned  long size;
};

struct S_FAT1216_BPB {
  unsigned  char jmps[2];       // The jump short instruction
  unsigned  char nop;           // nop instruction;
            char oemname[8];    // OEM name
  unsigned short nBytesPerSec;  // Bytes per sector
  unsigned  char nSecPerClust;  // Sectors per cluster
  unsigned short nSecRes;       // Sectors reserved for Boot Record
  unsigned  char nFATs;         // Number of FATs
  unsigned short nRootEnts;     // Max Root Directory Entries allowed
  unsigned short nSecs;         // Number of Logical Sectors (0B40h)
  unsigned  char mDesc;         // Medium Descriptor Byte
  unsigned short nSecPerFat;    // Sectors per FAT
  unsigned short nSecPerTrack;  // Sectors per Track
  unsigned short nHeads;        // Number of Heads
  unsigned  long nSecHidden;    // Number of Hidden Sectors
  unsigned  long nSecsExt;      // This value used when there are more
  unsigned  char DriveNum;      // Physical drive number
  unsigned  char nResByte;      // Reserved (we use for FAT type (12- 16-bit)
  unsigned  char sig;           // Signature for Extended Boot Record
  unsigned  long SerNum;        // Volume Serial Number
            char VolName[11];   // Volume Label
            char FSType[8];     // File system type
  unsigned  char filler[384];
  struct PART_TBLE part_tble[4]; // partition table
  unsigned short boot_sig;
};

#define   SECT_RES32  32   // sectors reserved

struct S_FAT32_BPB {
  unsigned  char jmps[2];
  unsigned  char nop;           // nop instruction;
            char oemname[8];
  unsigned short nBytesPerSec;
  unsigned  char nSecPerClust;
  unsigned short nSecRes;
  unsigned  char nFATs;
  unsigned short nRootEnts;
  unsigned short nSecs;
  unsigned  char mDesc;
  unsigned short nSecPerFat;
  unsigned short nSecPerTrack;
  unsigned short nHeads;
  unsigned  long nSecHidden;
  unsigned  long nSecsExt;
  unsigned  long sect_per_fat32; // offset 36 (24h)
  unsigned short ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
  unsigned short fs_version;
  unsigned  long root_base_cluster; //
  unsigned short fs_info_sec;
  unsigned short backup_boot_sec;
  unsigned  char reserved[12];
  unsigned  char DriveNum;  // not FAT specific
  unsigned  char nResByte;
  unsigned  char sig;
  unsigned  long SerNum;
            char VolName[11];
            char FSType[8];
  unsigned  char filler[356];
  struct PART_TBLE part_tble[4]; // partition table
  unsigned short boot_sig;
};

struct S_FAT32_FSINFO {
	unsigned  long sig0;              // 0x41615252 ("RRaA")
  unsigned  char resv[480];
	unsigned  long sig1;              // 0x61417272 ("rrAa")
	unsigned  long free_clust_fnt;    // 0xFFFFFFFF when the count is unknown
	unsigned  long next_free_clust;   // most recent allocated cluster  + 1
	unsigned  char resv1[12];
	unsigned  long trail_sig;         // 0xAA550000
};


unsigned char boot_code[] = {
  0xFA,           //CLI
  0xB8,0xC0,0x07, //MOV AX,07C0
  0x8E,0xD8,      //MOV DS,AX
  0x8E,0xD0,      //MOV SS,AX
  0xBC,0x00,0x40, //MOV SP,4000
  0xFB,           //STI
  0xBE,0x6B,0x00, //MOV SI,006B
  0xE8,0x06,0x00, //CALL  0156
  0x30,0xE4,      //XOR AH,AH
  0xCD,0x16,      //INT 16
  0xCD,0x18,      //INT 18
  0x50,           //PUSH  AX
  0x53,           //PUSH  BX
  0x56,           //PUSH  SI
  0xB4,0x0E,      //MOV AH,0E
  0x31,0xDB,      //XOR BX,BX
  0xFC,           //CLD
  0xAC,           //LODSB
  0x08,0xC0,      //OR  AL,AL
  0x74,0x04,      //JZ  0167
  0xCD,0x10,      //INT 10
  0xEB,0xF6,      //JMP 015D
  0x5E,           //POP SI
  0x5B,           //POP BX
  0x58,           //POP AX
  0xC3,           //RET
  13,10
};
unsigned char boot_data[] = "Error reading disk or Non-System Disk"
                            "\x0D\x0A"
                            "Press a key to reboot\x00";

unsigned char empty_mbr[] = {
  0xFA, 0xB8, 0xC0, 0x07, 0x8E, 0xD8, 0x8E, 0xD0, 0xBC, 0x00, 0x40, 0xFB, 0xBE, 0x24, 0x00, 0xE8, 
  0x03, 0x00, 0xF4, 0xEB, 0xFD, 0xB4, 0x0E, 0x31, 0xDB, 0xFC, 0xAC, 0x08, 0xC0, 0x74, 0x04, 0xCD, 
  0x10, 0xEB, 0xF6, 0xC3, 0x03, 0x0A, 0x07, 0x49, 0x20, 0x61, 0x6D, 0x20, 0x61, 0x6E, 0x20, 0x65, 
  0x6D, 0x70, 0x74, 0x79, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x63, 0x74, 0x6F, 0x72, 
  0x2E, 0x20, 0x20, 0x49, 0x20, 0x77, 0x69, 0x6C, 0x6C, 0x20, 0x6A, 0x75, 0x73, 0x74, 0x20, 0x68, 
  0x61, 0x6C, 0x74, 0x20, 0x68, 0x65, 0x72, 0x65, 0x2E, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x55, 0xAA
};

#pragma pack(pop)

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace Windows_Fat12_Floppy_Image_Writer
{
    class Program
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        unsafe struct FAT12_BIOS_PARAMATER_BLOCK
        {
            public fixed byte OEMName[8];
            public UInt16 BytesPerSector;
            public byte SectorsPerCluster;
            public UInt16 ReservedSectors;
            public byte NumberOfFats;
            public UInt16 NumDirEntries;
            public UInt16 NumSectors;
            public byte Media;
            public UInt16 SectorsPerFat;
            public UInt16 SectorsPerTrack;
            public UInt16 HeadsPerCyl;
            public UInt16 HiddenSectors;
            public UInt16 LongSectors;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        unsafe struct FAT12_BIOS_PARAMATER_BLOCK_EXT
        {
            public UInt32 SectorsPerFat32; /* Sectors per FAT */
            public UInt16 Flags; /* Flags */
            public UInt16 Version; /* Version */
            public UInt32 RootCluster; /* Starting root directory */
            public UInt16 InfoCluster;
            public UInt16 BackupBoot; /* Location of bootsector copy */
            public fixed UInt16 Reserved[6];
        };

        /* First 512 bytes of a partition */
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        unsafe struct FAT12_BOOT_SECTOR
        {
            public fixed byte Ignore[3]; /* First 3 bytes are ignored (our jmp instruction) */
            public FAT12_BIOS_PARAMATER_BLOCK Bpb; /* BPB structure */
            public FAT12_BIOS_PARAMATER_BLOCK_EXT BpbExt; /* Extended BPB info */
            public fixed byte Filler[448]; /* Needed to make struct 512 bytes */
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        unsafe struct FAT12_MOUNT_INFO
        {
            public UInt32 numSectors;
            public UInt32 fatOffset;
            public UInt32 numRootEntries;
            public UInt32 rootOffset;
            public UInt32 rootSize;
            public UInt32 fatSize;
            public UInt32 fatEntrySize;
        };

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
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        unsafe struct FAT12_FILE_ENTRY
        {
            public fixed byte Filename[8]; /* Filename */
            public fixed byte Ext[3]; /* Extension (8.3 filename format) */
            public byte Attrib; /* File attributes */
            public byte Reserved;
            public byte TimeCreatedMs; /* Creation time */
            public UInt16 TimeCreated;
            public UInt16 DateCreated; /* Creation date */
            public UInt16 DateLastAccessed;
            public UInt16 FirstClusterHiBytes;
            public UInt16 LastModTime; /* Last modification date/time */
            public UInt16 LastModDate;
            public UInt16 FirstCluster;          /* First cluster of file data */
            public UInt32 FileSize; /* Size in bytes */
        };

        static void Main(string[] args)
        {
            string imageFileName = args[0];

            FileInfo image = new FileInfo(imageFileName);
            FileStream fStream = image.Open(FileMode.Open, FileAccess.ReadWrite);

            FAT12_BOOT_SECTOR bootSector = LoadBootSector(fStream);
            FAT12_MOUNT_INFO mount_info = new FAT12_MOUNT_INFO();

            mount_info.numSectors = bootSector.Bpb.NumSectors;
            mount_info.fatOffset = 1;
            mount_info.fatSize = bootSector.Bpb.SectorsPerFat;
            mount_info.fatEntrySize = 8;
            mount_info.numRootEntries = bootSector.Bpb.NumDirEntries;
            mount_info.rootOffset = (uint)(bootSector.Bpb.NumberOfFats * bootSector.Bpb.SectorsPerFat) + 1;
            mount_info.rootSize = (uint)(bootSector.Bpb.NumDirEntries * 32) / bootSector.Bpb.BytesPerSector;

            for (int i = 1; i < args.Length; i++)
	        {
                SaveFileToDiskImage(args[i], fStream, bootSector, mount_info);
	        }
        }

        private static FAT12_BOOT_SECTOR LoadBootSector(FileStream fStream)
        {
            FAT12_BOOT_SECTOR retVal = new FAT12_BOOT_SECTOR();

            fStream.Seek(0, SeekOrigin.Begin);
            unsafe
            {
                byte[] buf = new byte[sizeof(FAT12_BOOT_SECTOR)];
                fStream.Read(buf, 0, sizeof(FAT12_BOOT_SECTOR));
                GCHandle pinnedBuf = GCHandle.Alloc(buf, GCHandleType.Pinned);
                retVal = (FAT12_BOOT_SECTOR)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject(), typeof(FAT12_BOOT_SECTOR));
                pinnedBuf.Free();
            }

            return retVal;
        }

        static void SaveFileToDiskImage(string fileName, FileStream fStream, FAT12_BOOT_SECTOR bootSector, FAT12_MOUNT_INFO mountInfo)
        {
            /* First delete the file if it exists */
            DeleteFile(fileName, fStream, bootSector, mountInfo);

            byte[] buf = new byte[512];
            FAT12_FILE_ENTRY fat12FileEntry;

            /* Get 8.3 directory name */
            char[] DosFileName = new char[12];
            ToDosFileName(fileName, ref DosFileName);
            DosFileName[11] = '\0';

            /* 16 entries per sector, 14 sectors total */
            for (int sector = 0; sector < 14; sector++)
            {
                /* Read in sector */
                fStream.Seek((mountInfo.rootOffset + sector) * 512, SeekOrigin.Begin);
                fStream.Read(buf, 0, 512);

                /* Get file info */
                GCHandle pinnedBuf = GCHandle.Alloc(buf, GCHandleType.Pinned);
                int count = 0;
                unsafe
                {
                    fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                }

                for (int i = 0; i < 16; i++)
                {
                    /* I'm looking for an entry beginning with \0 or 0xE5 */
                    unsafe
                    {
                        if (fat12FileEntry.Filename[0] == '\0' || fat12FileEntry.Filename[0] == 0xE5)
                        {
                            /* Found a blank file entry */
                            for (int j = 0; j < 8; j++)
                                fat12FileEntry.Filename[j] = (byte)DosFileName[j];
                            for (int j = 0; j < 3; j++)
                                fat12FileEntry.Ext[j] = (byte)DosFileName[j + 8];

                            fat12FileEntry.Attrib = 0;
                            /* TODO: Set the date and times */
                            fat12FileEntry.DateCreated = 0;
                            fat12FileEntry.DateLastAccessed = 0;
                            fat12FileEntry.LastModDate = 0;
                            fat12FileEntry.LastModTime = 0;
                            fat12FileEntry.TimeCreated = 0;
                            fat12FileEntry.TimeCreatedMs = 0;
                            fat12FileEntry.FileSize = 0;

                            fat12FileEntry.FirstCluster = GetFirstFreeCluster(fStream, 2);

                            UInt16 currentCluster = fat12FileEntry.FirstCluster;
                            FileStream inputFile = new FileStream(fileName, FileMode.Open);
                            byte[] inputBuffer = new byte[512];
                            int readBytes = 0;
                            while ((readBytes = inputFile.Read(inputBuffer, 0, 512)) > 0)
                            {
                                fat12FileEntry.FileSize += (uint)readBytes;
                                fStream.Seek((31 + currentCluster) * 512, SeekOrigin.Begin);
                                fStream.Write(inputBuffer, 0, readBytes);
                                
                                if (fStream.Length > fat12FileEntry.FileSize)
                                { /* Allocate next block */
                                    UInt16 nextBlock = GetFirstFreeCluster(fStream, (UInt16)(currentCluster + 1));
                                    MarkBlockChain(fStream, currentCluster, nextBlock);
                                    currentCluster = nextBlock;
                                }
                            }

                            MarkBlockChain(fStream, currentCluster, 0xFFF);

                            fStream.Seek((mountInfo.rootOffset + sector) * 512 + (count * sizeof(FAT12_FILE_ENTRY)), SeekOrigin.Begin);
                            Marshal.StructureToPtr(fat12FileEntry, pinnedBuf.AddrOfPinnedObject(), true);
                            fStream.Write(buf, 0, sizeof(FAT12_FILE_ENTRY));

                            return;
                        }

                        count++;
                        unsafe
                        {
                            fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                        }
                    }
                }

                pinnedBuf.Free();
            }
        }

        private static void MarkBlockChain(FileStream fStream, ushort currentCluster, ushort nextCluster)
        {
            /* Read in sector */
            byte[] FAT = new byte[4];

            uint FAT_Offset = (uint)(currentCluster + (currentCluster / 2)); /* Multiply by 1.5 */
            uint FAT_Sector = 1 + (FAT_Offset / 512);
            uint entryOffset = FAT_Offset % 512;

            /* Read FAT sectors */
            fStream.Seek(FAT_Sector * 512 + entryOffset, SeekOrigin.Begin);
            fStream.Read(FAT, 0, 4);

            /* Read entry for next cluster */
            UInt32 writeBackClusterValue = BitConverter.ToUInt32(FAT, 0);

            /* Test if entry is odd or even */
            if ((currentCluster & 0x0001) == 0x0001)
            {
                writeBackClusterValue &= 0xFFFF000F; /* Zero out the high 12 bits */
                writeBackClusterValue |= (UInt16)(((UInt16)0xFFF0) & (nextCluster << 4));
            }
            else
            {
                writeBackClusterValue &= 0xFFFFF000; /* Zero out the low 12 bits */
                writeBackClusterValue |= (UInt16)(((UInt16)0xFFF) & nextCluster);
            }

            fStream.Seek(FAT_Sector * 512 + entryOffset, SeekOrigin.Begin);
            fStream.Write(BitConverter.GetBytes(writeBackClusterValue), 0, 4);
        }

        private static ushort GetFirstFreeCluster(FileStream fStream, UInt16 firstClusterToExamine)
        {
            /* Read in sector */
            byte[] FAT = new byte[4];

            for (int currentCluster = firstClusterToExamine; currentCluster < 1024; currentCluster++)
            {

                uint FAT_Offset = (uint)(currentCluster + (currentCluster / 2)); /* Multiply by 1.5 */
                uint FAT_Sector = 1 + (FAT_Offset / 512);
                uint entryOffset = FAT_Offset % 512;

                /* Read FAT sectors */
                fStream.Seek(FAT_Sector * 512 + entryOffset, SeekOrigin.Begin);
                fStream.Read(FAT, 0, 4);

                /* Read entry for next cluster */
                UInt32 nextCluster = BitConverter.ToUInt32(FAT, 0);

                /* Test if entry is odd or even */
                if ((currentCluster & 0x0001) == 0x0001)
                {
                    nextCluster = (nextCluster >> 4) & 0xFFF;      /* Grab high 12 bits */
                }
                else
                {
                    nextCluster = nextCluster & 0x00000FFF;   /* Grab low 12 bits */
                }

                /* Test for free cluster */
                if (nextCluster == 0)
                {
                    return (UInt16)currentCluster;
                }
            }

            return 0xFFF;
        }

        private static FAT12_FILE_ENTRY? OpenSubDirectory(string fileName, FileStream fStream, FAT12_MOUNT_INFO mountInfo, FAT12_FILE_ENTRY? curFile)
        {
	        byte[] buf = new byte[512];
	        FAT12_FILE_ENTRY fat12FileEntry;
            
	        /* Get 8.3 directory name */
	        char[] DosFileName = new char[12];
            ToDosFileName(fileName, ref DosFileName);
	        DosFileName[11] = '\0';

	        /* 16 entries per sector, 14 sectors total */
	        for (int sector = 0; sector < 14; sector++)
	        {
		        /* Read in sector */
                fStream.Seek(mountInfo.rootOffset + sector, SeekOrigin.Begin);
                fStream.Read(buf, 0, 512);

		        /* Get file info */
                GCHandle pinnedBuf = GCHandle.Alloc(buf, GCHandleType.Pinned);
                int count = 0;
                unsafe
                {
                    fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                }

		        for (int i = 0; i < 16; i++)
		        {
			        /* Get current filename */
                    char[] name = new char[12];
                    unsafe
                    {
                        for (int j = 0; j < 11; j++)
                            name[j] = (char)fat12FileEntry.Filename[j];
                    }
			        name[11] = '\0';

			        /* Find a match? */
			        if (String.Compare(DosFileName.ToString(), name.ToString()) == 0)
			        {
				        /* Found it, set up file info */
				        /* strcpy(file.name, DirectoryName); */
				        /* file.currentCluster = directory->FirstCluster; */

                        return fat12FileEntry;
			        }

                    count++;
                    unsafe
                    {
                        fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                    }
		        }

                pinnedBuf.Free();
	        }

            return null;
        }

        private static void DeleteFile(string fileName, FileStream fStream, FAT12_BOOT_SECTOR bootSector, FAT12_MOUNT_INFO mountInfo)
        {
            byte[] buf = new byte[512];
            FAT12_FILE_ENTRY fat12FileEntry;

            /* Get 8.3 directory name */
            char[] DosFileName = new char[12];
            ToDosFileName(fileName, ref DosFileName);
            DosFileName[11] = '\0';

            /* 16 entries per sector, 14 sectors total */
            for (int sector = 0; sector < 14; sector++)
            {
                /* Read in sector */
                fStream.Seek((mountInfo.rootOffset + sector) * 512, SeekOrigin.Begin);
                fStream.Read(buf, 0, 512);

                /* Get file info */
                GCHandle pinnedBuf = GCHandle.Alloc(buf, GCHandleType.Pinned);
                int count = 0;
                unsafe
                {
                    fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                }

                for (int i = 0; i < 16; i++)
                {
                    /* Get current filename */
                    char[] name = new char[12];
                    unsafe
                    {
                        for (int j = 0; j < 11; j++)
                            name[j] = (char)fat12FileEntry.Filename[j];
                    }
                    name[11] = '\0';

                    /* Find a match? */
                    if (String.Compare(String.Concat(DosFileName), String.Concat(name)) == 0)
                    {
                        FreeClusterChain(fat12FileEntry.FirstCluster, fStream);

                        unsafe
                        {
                            fat12FileEntry.Filename[0] = 0xE5;
                            fStream.Seek((mountInfo.rootOffset + sector) * 512 + (count * sizeof(FAT12_FILE_ENTRY)), SeekOrigin.Begin);
                            Marshal.StructureToPtr(fat12FileEntry, pinnedBuf.AddrOfPinnedObject(), true);
                            fStream.Write(buf, 0, sizeof(FAT12_FILE_ENTRY));
                        }

                        return;
                    }

                    count++;
                    unsafe
                    {
                        fat12FileEntry = (FAT12_FILE_ENTRY)Marshal.PtrToStructure(pinnedBuf.AddrOfPinnedObject() + (count * sizeof(FAT12_FILE_ENTRY)), typeof(FAT12_FILE_ENTRY));
                    }
                }

                pinnedBuf.Free();
            }

            return;
        }

        private static void FreeClusterChain(UInt16 firstCluster, FileStream fStream)
        {
		    /* Read in sector */
		    byte[] FAT = new byte[4];

		    uint FAT_Offset = (uint)(firstCluster + (firstCluster / 2)); /* Multiply by 1.5 */
		    uint FAT_Sector = 1 + (FAT_Offset / 512);
		    uint entryOffset = FAT_Offset % 512;

		    /* Read FAT sectors */
            fStream.Seek(FAT_Sector * 512 + entryOffset, SeekOrigin.Begin);
            fStream.Read(FAT, 0, 4);

		    /* Read entry for next cluster */
            UInt32 nextCluster = BitConverter.ToUInt32(FAT, 0);
            UInt32 writeBackClusterValue = nextCluster;

		    /* Test if entry is odd or even */
            if ((firstCluster & 0x0001) == 0x0001)
            {
                nextCluster = (nextCluster >> 4) & 0xFFF;      /* Grab high 12 bits */
                writeBackClusterValue &= 0xFFFF000F; /* Zero out the high 12 bits */
            }
            else
            {
                nextCluster = nextCluster & 0x00000FFF;   /* Grab low 12 bits */
                writeBackClusterValue &= 0xFFFFF000; /* Zero out the low 12 bits */
            }

            fStream.Seek(FAT_Sector * 512 + entryOffset, SeekOrigin.Begin);
            fStream.Write(BitConverter.GetBytes(writeBackClusterValue), 0, 4);

		    /* Test for end of file */
		    if (nextCluster >= 0xff8)
		    {
			    return;
		    }

		    /* Test for file corruption */
		    if (nextCluster == 0)
		    {
			    return;
		    }

		    /* Set next cluster */
            FreeClusterChain((UInt16)nextCluster, fStream);
        }

        private static void ToDosFileName(string fileName, ref char[] DosFileName)
        {
            bool hitDot = false;
            for (int i = 0; i < 8; i++)
            {
                if (!hitDot && fileName[i] != '\0')
                {
                    if (fileName[i] == '.')
                    {
                        hitDot = true;
                        DosFileName[i] = ' ';
                    }
                    else
                        DosFileName[i] = Char.ToUpper(fileName[i]);
                }
                else
                {
                    DosFileName[i] = ' ';
                }
            }

            int fileNameExtStart = fileName.LastIndexOf('.');
            fileNameExtStart++;

            for (int i = 0; i < 3; i++)
            {
                if (fileNameExtStart + i < fileName.Length && fileName[fileNameExtStart + i] != '\0')
                {
                    DosFileName[8 + i] = Char.ToUpper(fileName[fileNameExtStart + i]);
                }
                else
                {
                    DosFileName[8 + i] = ' ';
                }
            }
        }
    }
}

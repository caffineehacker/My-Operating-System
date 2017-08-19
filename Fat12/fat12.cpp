#include <cstdint>
#include <cstring>
#include "fat12.h"
#include "..\My Operating System\filesystem.h"
#include "..\CRT\systemcall.h"

/* TODO: This should be a class inheriting from FILESYSTEM */

FILESYSTEM _fat12_fsys;
FAT12_MOUNT_INFO _fat12_MountInfo;

/* TODO: THIS REALLY SHOULDN'T BE GLOBAL, IT NEEDS TO BE MALLOC'D BUT I DON'T HAVE A MALLOC YET */
FAT12_FILE_ENTRY _fat12FileEntry;
char FAT[1024];

FILE fsysFat12FileEntry(const char* FileEntryName);
FILE fsysFat12OpenSubDir(FILE kFile, const char* filename);
void fsysFat12Read(PFILE file, unsigned char* Buffer, unsigned int Length);

/**
*	Opens a file
*/
FILE fsysFat12Open(const char* FileName)
{
	FILE curFile;
	char* p = 0;
	bool rootDir = true;
	char* path = (char*)FileName;

	/* Any '\'s in path? */
	p = strchr(path, '\\');
	if (!p)
	{
		/* Root Directory */
		curFile = fsysFat12FileEntry(path);

		/* Found file? */
		if (curFile.flags == FS_FILE)
			return curFile;

		/* Unable to find file */
		FILE ret;
		ret.flags = FS_INVALID;
		return ret;
	}

	/* Go to next character after first '\' */
	p++;

	while (p)
	{
		/* Get pathname */
		char pathname[16];
		int i = 0;
		for (i = 0; i < 16; i++)
		{
			/* If another '\' or end of line is reached, we are done */
			if (p[i] == '\\' || p[i] == '\0')
				break;

			pathname[i] = p[i];
		}

		pathname[i] = 0;

		/* Open subdirectory or file */
		if (rootDir)
		{
			/* Search root directory - open pathname */
			curFile = fsysFat12FileEntry(pathname);
			rootDir = false;
		}
		else
		{
			/* Search a subdirectory pathname */
			curFile = fsysFat12OpenSubDir(curFile, pathname);
		}

		/* Found directory or file? */
		if (curFile.flags == FS_INVALID)
			break;
		else if (curFile.flags == FS_FILE)
			return curFile;

		/* Find next '\' */
		p = strchr(p + 1, '\\');
		if (p)
			p++;
	}

	/* Unable to find */
	FILE ret;
	ret.flags = FS_INVALID;
	return ret;
}

void fsysFat12Close(PFILE file)
{
	/* No need to do anything to close a file for FAT12 */
}

PFILESYSTEM fat12_mount(PFAT12_BOOTSECTOR bootsector)
{
	strcpy(_fat12_fsys.Name, "FAT12");
	_fat12_fsys.Directory = fsysFat12FileEntry;
	/*_fat12_fsys.Mount = fsysFat12Mount;*/
	_fat12_fsys.Open = fsysFat12Open;
	_fat12_fsys.Read = fsysFat12Read;
	_fat12_fsys.Close = fsysFat12Close;

	/* Store mount info */
	_fat12_MountInfo.numSectors = bootsector->Bpb.NumSectors;
	_fat12_MountInfo.fatOffset = 1;
	_fat12_MountInfo.fatSize = bootsector->Bpb.SectorsPerFat;
	_fat12_MountInfo.fatEntrySize = 8;
	_fat12_MountInfo.numRootEntries = bootsector->Bpb.NumDirEntries;
	_fat12_MountInfo.rootOffset = (bootsector->Bpb.NumberOfFats * bootsector->Bpb.SectorsPerFat) + 1;
	_fat12_MountInfo.rootSize = (bootsector->Bpb.NumDirEntries * 32) / bootsector->Bpb.BytesPerSector;

	_fat12_fsys.tag = &_fat12_MountInfo;

	return &_fat12_fsys;
}

void ToDosFileName(const char* inName, char* outName)
{
	char *tmpInName = (char*)inName;
	char *tmpOutName = outName;
	int i = 0;

	for (; i < 8 && *tmpInName != '\0' && *tmpInName != '.'; tmpInName++, tmpOutName++, i++)
	{
		*tmpOutName = *tmpInName;
	}

	for (; i < 8; i++, tmpOutName++)
		*tmpOutName = ' ';

	if (*tmpInName != '\0')
	{
		/* Move past the period */
		tmpInName++;
		for (; i < 11 && *tmpInName != '\0'; tmpInName++, tmpOutName++, i++)
		{
			*tmpOutName = *tmpInName;
		}
	}

	for (; i < 11; i++, tmpOutName++)
		*tmpOutName = ' ';
}

void* flpydsk_read_sector(int sectorNumber)
{
	return ExecuteSysCall(SYSCALL_READ_FLOPPY_SECTOR, (void*)sectorNumber);
}

/* I call this a file entry because it could be a directory or a file, but directories are really just files */
FILE fsysFat12FileEntry(const char* FileEntryName)
{
	FILE file;
	unsigned char* buf;
	/* TODO: This NEEDS to be malloc'd */
	PFAT12_FILE_ENTRY_RAW fat12FileEntry;

	/* Get 8.3 directory name */
	char DosFileName[12];
	ToDosFileName(FileEntryName, DosFileName);
	DosFileName[11] = 0;

	/* 16 entries per sector, 14 sectors total */
	for (int sector = 0; sector < 14; sector++)
	{
		/* Read in sector */
		buf = (unsigned char*)flpydsk_read_sector(_fat12_MountInfo.rootOffset + sector);

		/* Get file info */
		fat12FileEntry = (PFAT12_FILE_ENTRY_RAW)buf;

		for (int i = 0; i < 16; i++)
		{
			/* Get current filename */
			char name[12];
			memcpy(name, fat12FileEntry->Filename, 11);
			name[11] = 0;

			/* Find a match? */
			if (strcmp(DosFileName, name) == 0)
			{
				/* Found it, set up file info */
				/* strcpy(file.name, DirectoryName); */
				/* file.currentCluster = directory->FirstCluster; */
				memcpy(&_fat12FileEntry, fat12FileEntry, sizeof(FAT12_FILE_ENTRY_RAW));

				_fat12FileEntry.currentCluster = _fat12FileEntry.FirstCluster;
				file.flags = 0;
				file.tag = &fat12FileEntry;

				/* Set file type */
				if (_fat12FileEntry.Attrib == 0x10)
					file.flags = FS_DIRECTORY;
				else
					file.flags = FS_FILE;

				return file;
			}

			fat12FileEntry++;
		}
	}

	/* Unable to find file */
	file.flags = FS_INVALID;
	return file;
}

void fsysFat12Read(PFILE file, unsigned char* Buffer, unsigned int Length)
{
	_asm {
		cli
		hlt
	};
	if (file)
	{
		_asm {
			cli
			hlt
		};
		/* Starting physical sector, start on the 32nd sector to skip over the bootloader and two fats */
		unsigned int physSector = 31 + (((PFAT12_FILE_ENTRY)file->tag)->currentCluster);

		/* Read in sector */
		unsigned char* sector = (unsigned char*)flpydsk_read_sector(physSector);

		/* Copy block of memory */
		memcpy(Buffer, sector, 512);

		unsigned int FAT_Offset = ((PFAT12_FILE_ENTRY)file->tag)->currentCluster + (((PFAT12_FILE_ENTRY)file->tag)->currentCluster / 2); /* Multiply by 1.5 */
		unsigned int FAT_Sector = 1 + (FAT_Offset / 512);
		unsigned int entryOffset = FAT_Offset % 512;

		/* Read 1st FAT sector */
		sector = (unsigned char*)flpydsk_read_sector(FAT_Sector);
		memcpy(FAT, sector, 512);

		/* Read 2nd FAT sector */
		sector = (unsigned char*)flpydsk_read_sector(FAT_Sector + 1);
		memcpy(FAT + 512, sector, 512);

		/* Read entry for next cluster */
		uint16_t nextCluster = *(uint16_t*)&FAT[entryOffset];

		/* Test if entry is odd or even */
		if (((PFAT12_FILE_ENTRY)file->tag)->currentCluster & 0x0001)
			nextCluster >>= 4;      /* Grab high 12 bits */
		else
			nextCluster &= 0x0FFF;   /*Grab low 12 bits */

		/* Test for end of file */
		if (nextCluster >= 0xff8)
		{
			file->flags |= FILE_FLAG_EOF;
			return;
		}

		/* Test for file corruption */
		if (nextCluster == 0)
		{
			file->flags |= FILE_FLAG_EOF;
			return;
		}

		/* Set next cluster */
		((PFAT12_FILE_ENTRY)file)->currentCluster = nextCluster;
	}
}

FILE fsysFat12OpenSubDir(FILE kFile, const char* filename)
{
	FILE file;

	/* Get 8.3 directory name */
	char DosFileName[12];
	ToDosFileName(filename, DosFileName);
	DosFileName[11] = 0;

	/* Read directory */
	while (!(kFile.flags & FILE_FLAG_EOF))
	{
		unsigned char buf[512];
		fsysFat12Read(&file, buf, 512);

		/* Set directory */
		PFAT12_FILE_ENTRY_RAW pkDir = (PFAT12_FILE_ENTRY_RAW)buf;

		/* 16 entries in buffer */
		for (unsigned int i = 0; i < 16; i++)
		{
			/* Get current filename */
			char name[12];
			memcpy(name, pkDir->Filename, 11);
			name[11] = 0;

			if (strcmp(name, DosFileName) == 0)
			{
				/* Found it, set up file info */
				/* strcpy(file.name, filename);
				file.id             = 0;
				file.currentCluster = pkDir->FirstCluster;
				file.fileLength     = pkDir->FileSize;
				file.eof            = 0; */
				memcpy(&_fat12FileEntry, pkDir, sizeof(FAT12_FILE_ENTRY_RAW));

				file.tag = &_fat12FileEntry;
				file.flags = 0;
				_fat12FileEntry.currentCluster = _fat12FileEntry.FirstCluster;

				/* Set file type */
				if (pkDir->Attrib == 0x10)
					file.flags = FS_DIRECTORY;
				else
					file.flags = FS_FILE;

				return file;
			}

			pkDir++;
		}
	}

	/* Unable to find file */
	file.flags = FS_INVALID;
	return file;
}
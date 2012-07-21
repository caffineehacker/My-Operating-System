#pragma once
#include <cstdio>

typedef struct _FILE_SYSTEM
{
	char Name [8];
	/* Gets a FILE struct for a directory given a directory path name */
	FILE (*Directory)(const char* DirectoryName);
	/* Mounds the volume/file system */
	void (*Mount)();
	/* Reads length number of bytes from file to buffer */
	void (*Read)(PFILE file, unsigned char* Buffer, unsigned int Length);
	/* Closes a file */
	void (*Close)(PFILE file);
	/* Opens a file based on a file name and returns a FILE struct */
	FILE (*Open)(const char* FileName);
	/* This property can be used by the FS driver to store data */
	void* tag;
} FILESYSTEM, *PFILESYSTEM;

#define DEVICE_MAX 26

extern FILE volOpenFile(const char* fname);
extern void volReadFile(PFILE file, unsigned char* Buffer, unsigned int Length);
extern void volCloseFile(PFILE file);
extern void volRegisterFileSystem(PFILESYSTEM fsys, unsigned int deviceID);
extern void volUnregisterFileSystem(PFILESYSTEM fsys);
extern void volUnregisterFileSystemByID (unsigned int deviceID);

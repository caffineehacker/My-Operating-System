#include "filesystem.h"

/* File system list */
PFILESYSTEM _FileSystems[DEVICE_MAX];

FILE volOpenFile(const char* filename)
{
	if (filename)
	{
		/* Default to device 'a' */
		/* TODO: Use a "current drive"/CWD/PWD for this */
		unsigned char device = 'a';

		/* In all cases, if filename[1] == ':' then the first character must be device letter */
		if (filename[1] == ':')
		{
			device = filename[0];
			filename += 2; //strip it from pathname
		}

		/* Call filesystem */
		if (_FileSystems[device - 'a'])
		{
			/* Set volume specific information and return file */
			FILE file = _FileSystems[device - 'a']->Open(filename);
			file.deviceID = device;
			return file;
		}
	}

	FILE file;
	file.flags = FS_INVALID;
	return file;
}

void volRegisterFileSystem(PFILESYSTEM fsys, unsigned int deviceID)
{
	if (deviceID < DEVICE_MAX && fsys)
		_FileSystems[deviceID] = fsys;
}

void volUnregisterFileSystem(PFILESYSTEM fsys)
{
	if (fsys)
	{
		for (int i = 0; i < DEVICE_MAX; i++)
		{
			if (_FileSystems[i] && _FileSystems[i] == fsys)
			{
				volUnregisterFileSystemByID(i);
			}
		}
	}
}

void volUnregisterFileSystemByID(unsigned int deviceID)
{
	_FileSystems[deviceID] = NULL;
}

void volReadFile(PFILE file, unsigned char* Buffer, unsigned int Length)
{
	if (file)
	{
		if (_FileSystems[file->deviceID - 'a'])
		{
			_FileSystems[file->deviceID - 'a']->Read(file,Buffer,Length);
		}
	}
}

void volCloseFile(PFILE file)
{
	if (file)
		if (_FileSystems [file->deviceID - 'a'])
			_FileSystems[file->deviceID - 'a']->Close(file);
}

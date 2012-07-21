#pragma once

#define SYSCALL_GET_STDOUT 0
#define SYSCALL_READ_FLOPPY_SECTOR 1

extern void GenerateInterrupt(int n);
extern void* ExecuteSysCall(int functionNumber);
extern void* ExecuteSysCall(int functionNumber, void* data);

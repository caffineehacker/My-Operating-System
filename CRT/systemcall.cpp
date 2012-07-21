#include <_null.h>

/* This is a hack and a half */
inline void GenerateInterrupt(int n)
{
	_asm {
		mov al, byte ptr [n]
		mov byte ptr [genint+1], al
		jmp genint
	genint:
		int 0	/* Above code modifies the 0 to the interrupt number to generate */
	}
}

void* ExecuteSysCall(int functionNumber, void* data)
{
	void *retVal = NULL;
	_asm {
		mov eax, [functionNumber]
		mov ebx, [data]
		int 0x81
		mov [retVal], eax
	}

	return retVal;
}

void* ExecuteSysCall(int functionNumber)
{
	return ExecuteSysCall(functionNumber, 0);
}

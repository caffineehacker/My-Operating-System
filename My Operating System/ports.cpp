#include <cstdint>
#include "ports.h"

_inline void __cdecl outb(uint16_t port, uint8_t val)
{
	_asm
	{
		mov dx, [port]
		mov al, [val]
		out dx,al
	}
}

_inline uint8_t __cdecl inb(uint16_t port)
{
	_asm
	{
		mov dx, [port]
		in al, dx
	}
}

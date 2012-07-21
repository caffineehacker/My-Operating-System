/* This enables floating point math */
extern "C" int _fltused = 1;

/* TODO: Use SSE to convert float to long */
extern "C" long __declspec (naked) _ftol2_sse() {
	int a;
#ifdef ARCH_X86
	_asm	fistp 	[a]
	_asm	mov	ebx, a
	_asm	ret
#endif
}

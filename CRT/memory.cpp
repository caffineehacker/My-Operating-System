/* Disable warning until I write these */
#pragma warning (disable:4100)

void* __cdecl ::operator new (unsigned int size) { return 0; }
void* __cdecl operator new[] (unsigned int size) { return 0; }
void __cdecl ::operator delete (void * p) {}
void __cdecl operator delete[] (void * p) { }


/* THIS SHOULD NEVER BE CALLED
 * _purecall is called if a virtual method is called
 */
#pragma warning (disable:4702) /* I know this should be unreachable */
int __cdecl ::_purecall() { for (;;); return 0; }

#pragma warning (default:4702)

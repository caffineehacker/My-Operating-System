/* Function pointer typedef for less typing */
typedef void (__cdecl *_PVFV)(void);

/* Just so I can place the functions out of order */
extern int __cdecl atexit(_PVFV fn);
extern void __cdecl _atexit_init(void);

/*
 * MSVC++ stores global initializers in a section called .CRT$XC? where ? is a letter.
 * The sections are ordered by the linker alphabetically so we can guarentee that .CRT$XCA is first
 * and we can use that to get a pointer to the list of global initializers.
 */
#pragma data_seg(".CRT$XCA")
_PVFV __xc_a[] = { 0 };

/*
 * Just like how .CRT$XCA came before the list, we can guarentee that .CRT$XCZ will be at the
 * end of the list.
 */
#pragma data_seg(".CRT$XCZ")
_PVFV __xc_z[] = { 0 };

#pragma data_seg() /* Bring us back to the normal data segment */

/*
 * This merges the CRT sections with the DATA section so we get read/write permissions
 */
#pragma comment(linker, "/merge:.CRT=.data")

void __cdecl _initterm(_PVFV* pfbegin, _PVFV* pfend)
{
	// Go through each initializer
	while (pfbegin < pfend)
	{
		// Execute the global initializer
		if ( *pfbegin != 0 )
			(**pfbegin) ();

	// Go to next initializer inside the initializer table
		++pfbegin;
	}
}

__declspec(dllexport) void __cdecl _CallGlobalConstructors()
{
	_atexit_init();
	// This initializes all global initializer routines:
	_initterm(__xc_a, __xc_z);
}

/*******************************************
 * Deconstructor stuff
 *******************************************/
/* Pointer to my list of deinitializers (deconstructors) */
static _PVFV* pf_atexitlist = 0;
/* Maximum number of entries allowed */
static unsigned max_atexitlist_entries = 32;
/* Current number of entries in the list */
static unsigned cur_atexitlist_entries = 0;

/* This routine is called by MSCV++ when each constructor is called */
int __cdecl atexit(_PVFV fn)
{
	/* Make sure we have room in the list */
	if (cur_atexitlist_entries>=max_atexitlist_entries)
	{
		return 1;
	}
	else
	{
		/* Add the dtor to the list */
		*(pf_atexitlist++) = fn;
		cur_atexitlist_entries++;
	}
 
	return 0;
}

void __cdecl _atexit_init(void)
{
    max_atexitlist_entries = 32;
 
	/* TODO: This is currently statically allocated due to our lack of a memory manager.
	 * Once we have a memory manager we should use that to allocate this table.
	 *
	 * TODO: We should also make it so the list can grow with realloc
	 */
    pf_atexitlist = (_PVFV *)0x5000;
}

__declspec(dllexport) void __cdecl _exit(void)
{
	/* Call all of the global dtor's */
	while (cur_atexitlist_entries--) {
		(*(--pf_atexitlist))();
	}
}


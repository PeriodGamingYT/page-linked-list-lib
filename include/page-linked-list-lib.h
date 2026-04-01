#ifndef PAGE_LINKED_LIST_LIB_H
#define PAGE_LINKED_LIST_LIB_H

	//// Struct forward/unfilled declarations (N/A).
	// ...


	//// Base includes, defines, and typedefs (N/A).
	#define WIN32_LEAN_AND_MEAN
	#define COBJMACROS
	#include <windows.h>
	#include <winuser.h>
	#include <winternl.h>
	#include <shlobj.h>
	#include <shobjidl.h>

	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "shell32.lib")
	#pragma comment(lib, "ole32.lib")
	#pragma comment(lib, "ntdll.lib")

	#include <stdarg.h>
	#include <stdint.h>
	#include <wchar.h>

	#include <page-linked-list.h>

	// Misc. macros.
	#define ARRAY_SIZE(_array) \
		(sizeof(_array) / sizeof((_array)[0]))

	#ifdef DEBUG_MODE
		#define DEBUG_CRASH(...) (*(uint8_t *)(0) = 0)
	#else
		#define DEBUG_CRASH(...) OutputDebugStringA("DEBUG_CRASH at " __FILE__)
	#endif

	// Misc. typedefs.
	typedef uint8_t Bool;
	typedef int Error;
	typedef char FilePath[MAX_PATH];

	// NOTE: This is only meant to keep track of what variables are on the heap
	// versus the stack.
	//
	// NOTE: Usage is: MALLOC(type) variable, and
	// MALLOC_JOIN(otherVariable, type) variable.
	#define MALLOC(...) __VA_ARGS__
	#define MALLOC_JOIN(joineeName, ...) __VA_ARGS__
#endif

#ifndef PAGE_LINKED_LIST_LIB_H
#define PAGE_LINKED_LIST_LIB_H

	//// Base includes, defines, and typedefs (N/A).
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <intrin.h>

	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "shell32.lib")
	#pragma comment(lib, "ole32.lib")
	#pragma comment(lib, "ntdll.lib")

	#include <stdarg.h>
	#include <stdint.h>
	#include <wchar.h>

	#define PAGE_LINKED_LIST_INIT_PAGE_FUNC InitPage
	#define PAGE_LINKED_LIST_DEINIT_PAGE_FUNC DeinitPage
	#define PAGE_LINKED_LIST_BYTES_IN_PAGE_FUNC BytesInPage
	#include <page-linked-list.h>

	// Misc. macros.
	#define ARRAY_SIZE(_array) \
		(sizeof(_array) / sizeof((_array)[0]))

	#ifdef DEBUG_MODE
		#define DEBUG_CRASH(...) __debugbreak()
	#else
		#define DEBUG_CRASH(...) OutputDebugStringA("DEBUG_CRASH at " __FILE__)
	#endif

	// Misc. typedefs.
	typedef uint8_t Bool;

	//// Functions needed for page linked list.
	uint8_t *InitPage(size_t);
	void DeinitPage(uint8_t **);
	size_t BytesInPage();
#endif

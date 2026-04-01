#include <page-linked-list-lib.h>

#define PAGE_LINKED_LIST_IMPL
#include <page-linked-list.h>

uint8_t *InitPage(size_t byteAmount) {

	// NOTE: POSIX version of this would be using mmap().
	return VirtualAlloc(
		NULL,
		byteAmount,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	);
}

void DeinitPage(uint8_t **bufferPointer) {
	if(*bufferPointer == NULL) { return; }

	// NOTE: POSIX version of this would be using munmap().
	VirtualFree(*bufferPointer, 0, MEM_RELEASE);
	*bufferPointer = NULL;
}

size_t BytesInPage() {

	// NOTE: POSIX version of this would be using getconf().
	SYSTEM_INFO systemInfo = { 0 };
	GetSystemInfo(&systemInfo);
	size_t bytesInPage = (size_t)(systemInfo.dwPageSize);
}

int WINAPI WinMain(
	HINSTANCE appHandle,
	HINSTANCE legacyPrevAppHandle,
	LPSTR commandLine,
	int windowShowFlags
) {
	(void)(legacyPrevAppHandle);
	(void)(commandLine);
	(void)(windowShowFlags);

	// ...
	return 0;
}

#include <page-linked-list-lib.h>

#define PAGE_LINKED_LIST_IMPL
#include <page-linked-list.h>

#include <stdio.h>

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
	return (size_t)(systemInfo.dwPageSize);
}

int main() {

	// NOTE: This is meant to demonstrate the usage of PageLinkedList and also
	// act as a test suite.
	PageLinkedList linkedList = InitPageLinkedList(sizeof(int));
	for(int i = 0; i < 200000; i++) {
		*(int *)(PageLinkedListAppend(&linkedList, 1)) = i;
	}

	PageLinkedListIterator iterator = InitPageLinkedListIterator(&linkedList);
	do {

		// NOTE: This is commented out because printing all these out takes a
		// fair bit of time.
		// fprintf(stderr, "%d\n", *(int *)(iterator.currentElement));
	} while(PageLinkedListIteratorNext(&iterator, 1));

	DeinitPageLinkedList(&linkedList);

	printf("Press any key to exit this program's console\n");
	getchar();
	return 0;
}

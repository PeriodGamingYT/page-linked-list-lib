#include <page-linked-list-lib.h>

#define PAGE_LINKED_LIST_IMPL
#include <page-linked-list.h>

#include <stdio.h>

// NOTE: InitPage and DeinitPage's allocation mechanisms are changed
// during debugging so that address sanitizer can catch any leaks (malloc and
// free are intercepted when running with address sanitizer).
#ifdef DEBUG_MODE
	uint8_t *InitPage(size_t byteAmount) {
		uint8_t *result = (uint8_t *)(malloc(byteAmount));
		memset(result, 0, byteAmount);
		return result;
	}

	void DeinitPage(uint8_t **bufferPointer) {
		if(*bufferPointer == NULL) { return; }
		free(*bufferPointer);
		*bufferPointer = NULL;
	}
#else
	uint8_t *InitPage(size_t byteAmount) {

		// NOTE: POSIX version of this would be using mmap().
		return (uint8_t *)(VirtualAlloc(
			NULL,
			byteAmount,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		));
	}

	void DeinitPage(uint8_t **bufferPointer) {
		if(*bufferPointer == NULL) { return; }

		// NOTE: POSIX version of this would be using munmap().
		VirtualFree(*bufferPointer, 0, MEM_RELEASE);
		*bufferPointer = NULL;
	}
#endif

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
	while(PageLinkedListIteratorNext(&iterator, 1)) {

		// NOTE: This is commented out because printing all these out takes a
		// fair bit of time, printing is slow (Thanks to Microslop!, see
		// refterm).
		// printf("%d\n", *(int *)(iterator.currentElement));
	}

	for(int i = 0; /* ... */; i += 1000) {
		int *result = (int *)(PageLinkedListGetAtIndex(&linkedList, i));
		if(result == NULL) { break; }
		printf("%d\n", *result);
	}

	printf(
		"The page linked list is %zd elements long\n",
		PageLinkedListGetTotalSize(&linkedList)
	);

	DeinitPageLinkedList(&linkedList);

	printf("Press any key to exit this program's console\n");
	getchar();
	return 0;
}

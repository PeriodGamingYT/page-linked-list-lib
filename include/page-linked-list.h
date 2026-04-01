#ifndef PAGE_LINKED_LIST_H
#define PAGE_LINKED_LIST_H

	//// Guard clauses to ensure user has given all appropriate information needed for library to operate.
	#ifndef PAGE_LINKED_LIST_INIT_PAGE_FUNC
		#error "PAGE_LINKED_LIST_INIT_PAGE_FUNC isn't defined:"
		#error "Give a method that conforms to uint8_t *funcName(size_t) that will allocate memory, preferably a memory page."
	#endif

	#ifndef PAGE_LINKED_LIST_DEINIT_PAGE_FUNC
		#error "PAGE_LINKED_LIST_DEINIT_PAGE_FUNC isn't defined:"
		#error "Give a method that conforms to void funcName(uint8_t **) that will deallocate memory."
	#endif

	//// Struct forward/unfilled declarations.
	typedef struct PageCellStruct PageCell;
	typedef struct PageCellArrayHeaderStruct PageCellArrayHeader;
	typedef struct PageLinkedListStruct PageLinkedList;


	//// Base includes, defines, and typedefs.
	#include <stdint.h>

	#define PAGE_LINKED_LIST_INIT_PAGE(_byteAmount) \
		PAGE_LINKED_LIST_INIT_PAGE_FUNC ((_byteAmount))

	#define PAGE_LINKED_LIST_DEINIT_PAGE(_bufferPointer) \
		PAGE_LINKED_LIST_DEINIT_PAGE_FUNC ((_bufferPointer))


	typedef struct PageCellStruct {
		size_t amount, cap;
		uint8_t *buffer;
	} PageCell;

	// NOTE: This struct is identical to PageCell and is only here to clarify semantic meaning. Casting a PageCell
	// to a PageCellArrayHeaderStruct and vice versa results in no data loss.
	typedef struct PageCellArrayHeaderStruct {
		size_t cellAmount, cellCap;
		PageCellArrayHeader *nextCellArray;
	} PageCellArrayHeader;

	typedef struct PageLinkedListStruct {

		// NOTE: A PageLinkedList contains PageCells which contain elements.
		//
		// NOTE: PageLinkedList will enforce everything within its pages to conform to the same size
		// in order to not introduce fragmentation. However, this doesn't mean that elements of varying size can't
		// be done with this data structure, bytePerElement just must be equal to sizeof(uint8_t).
		size_t bytesPerElement, defaultCellCap;
		PageCellArrayHeader *firstCellArray;

		// NOTE: This is for appending memory, having the last page cell array on demand makes appending
		// memory much easier and faster.
		PageCellArrayHeader *lastCellArray;
	} PageLinkedList;

	PageCell *PageCellArrayFirstCell(PageCellArrayHeader *arrayHeader);
	PageCell *PageCellArrayLastCell(PageCellArrayHeader *arrayHeader);
	PageCellArrayHeader *PageCellArrayNextCellArray(PageCellArrayHeader *arrayHeader);

	PageLinkedList InitPageLinkedList(size_t bytesPerElement, size_t defaultCellCap);
	void DeinitPageLinkedList(PageLinkedList *linkedList);

	// NOTE: pageCap is in bytes.
	PageCell *PageLinkedListAppendCustomPage(PageLinkedList *linkedList, size_t pageCap);
	PageCell *PageLinkedListAppendPage(PageLinkedList *linkedList);
	uint8_t *PageLinkedListAppend(PageLinkedList *linkedList, size_t amountToAppend);

	// NOTE: PageCell is used as the return type here because it has a amount
	// and buffer, PageCell can act as a memory arena. Here, PageCell.amount ==
	// PageCell.cap.
	typedef struct PageLinkedListIteratorStruct {
		PageLinkedList *linkedList;

		PageCellArrayHeader *currentCellArray;

		PageCell *currentCell;
		size_t currentCellIndex;

		uint8_t *currentElement;
		size_t currentElementIndex;
	} PageLinkedListIterator;

	// NOTE: Since PageLinkedListIterator doesn't hold any memory on its own,
	// DeinitPageLinkedListIterator() isn't necessary.
	PageLinkedListIterator InitPageLinkedListIterator(PageLinkedList *linkedList);

	// NOTE: These *Next* functions will only reset their relevant cell array/cell/element.
	// Resetting any other relevant cell/element is the responsibility of the caller.
	Bool PageLinkedListIteratorNextCellArray(PageIterator *iterator, size_t increaseAmount);
	Bool PageLinkedListIteratorNextCell(PageIterator *iterator, size_t increaseAmount);
	Bool PageLinkedListIteratorNext(PageIterator *iterator, size_t increaseAmount);

	size_t PageLinkedListGetTotalSize(PageLinkedList *linkedList);
	PageCell PageLinkedListAsFlatMemory(PageLinkedList *linkedList);

	uint8_t *PageLinkedListGetAtIndex(PageLinkedList *linkedList, size_t index);
#endif

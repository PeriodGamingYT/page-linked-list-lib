#ifndef PAGE_LINKED_LIST_H
#define PAGE_LINKED_LIST_H

	// NOTE: This single header library is based off of a Gist, whose URL is as
	// follows:
	// https://gist.github.com/PeriodGamingYT/75f921f7b66e05100ce1f35da6b51baf

	//// Guard clauses to ensure user has given all appropriate information
	// needed for library to operate.
	#ifndef PAGE_LINKED_LIST_INIT_PAGE_FUNC
		#error "PAGE_LINKED_LIST_INIT_PAGE_FUNC isn't defined:"
		#error "Give a method that conforms to uint8_t *funcName(size_t) that will allocate zeroed memory, preferably as a memory page."
	#endif

	#ifndef PAGE_LINKED_LIST_DEINIT_PAGE_FUNC
		#error "PAGE_LINKED_LIST_DEINIT_PAGE_FUNC isn't defined:"
		#error "Give a method that conforms to void funcName(uint8_t **) that will deallocate memory."
	#endif

	#ifndef PAGE_LINKED_LIST_BYTES_IN_PAGE_FUNC
		#error "PAGE_LINKED_LIST_BYTES_IN_PAGE_FUNC isn't defined:"
		#error "Give a method that conforms to size_t funcName() that will return the amount of bytes in a single page of memory."
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

	#define PAGE_LINKED_LIST_BYTES_IN_PAGE_AMOUNT() \
		PAGE_LINKED_LIST_BYTES_IN_PAGE_FUNC ()

	typedef uint8_t Bool;


	//// Single header structs and forward/unfilled function declarations.
	typedef struct PageCellStruct {
		size_t amount, cap;
		uint8_t *buffer;
	} PageCell;

	// NOTE: This struct is identical to PageCell and is only here to clarify
	// semantic meaning. Casting a PageCell to a PageCellArrayHeaderStruct and
	// vice versa results in no data loss.
	typedef struct PageCellArrayHeaderStruct {
		size_t cellAmount, cellCap;
		PageCellArrayHeader *nextCellArray;
	} PageCellArrayHeader;

	typedef struct PageLinkedListStruct {

		// NOTE: A PageLinkedList contains PageCells which contain elements.
		//
		// NOTE: PageLinkedList will enforce everything within its pages to
		// conform to the same size in order to not introduce fragmentation.
		// However, this doesn't mean that elements of varying size can't be
		// done with this data structure, bytePerElement just must be equal to
		// sizeof(uint8_t).
		size_t bytesPerElement, defaultCellCap, cellArrayCap;
		PageCellArrayHeader *firstCellArray;

		// NOTE: This is for appending memory, having the last page cell array
		// on demand makes appending memory much easier and much faster.
		PageCellArrayHeader *lastCellArray;
	} PageLinkedList;

	PageCell *PageCellArrayFirstCell(PageCellArrayHeader *);
	PageCell *PageCellArrayLastCell(PageCellArrayHeader *);

	PageLinkedList InitPageLinkedList(size_t);
	void DeinitPageLinkedList(PageLinkedList *);

	// NOTE: pageCap is in bytes.
	PageCell *PageLinkedListAppendCustomPage(PageLinkedList *, size_t);
	PageCell *PageLinkedListAppendPage(PageLinkedList *);
	uint8_t *PageLinkedListAppend(PageLinkedList *, size_t);

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
	PageLinkedListIterator InitPageLinkedListIterator(PageLinkedList *);

	// NOTE: These ...Next... functions will only reset their relevant cell
	// array/cell/element. Resetting any other relevant cell/element is the
	// responsibility of the caller.
	Bool PageLinkedListIteratorNextCellArray(PageLinkedListIterator *, size_t);
	Bool PageLinkedListIteratorNextCell(PageLinkedListIterator *, size_t);
	Bool PageLinkedListIteratorNext(PageLinkedListIterator *, size_t);

	size_t PageLinkedListGetTotalSize(PageLinkedList *);
	PageCell PageLinkedListAsFlatMemory(PageLinkedList *);

	uint8_t *PageLinkedListGetAtIndex(PageLinkedList *, size_t);
#endif

// NOTE: This is put outside of the guard clause because a user might include
// only the header part and then in another file include the implementation
// part.
#ifdef PAGE_LINKED_LIST_IMPL
	PageCell *PageCellArrayFirstCell(PageCellArrayHeader *arrayHeader) {

		// NOTE: Can't do PageCellArrayHeader * -> PageCell *, I have to use
		// void * as a man in the middle.
		PageCell *arrayHeaderAsCell = (PageCell *)((void *)(arrayHeader));

		// NOTE: Since the first cell in a page cell array is reserved for an
		// array header, the next element will be used to hold actual page
		// cells.
		return &arrayHeaderAsCell[1];
	}

	PageCell *PageCellArrayLastCell(PageCellArrayHeader *arrayHeader) {

		// NOTE: Can't do PageCellArrayHeader * -> PageCell *, I have to use
		// void * as a man in the middle.
		PageCell *arrayHeaderAsCell = (PageCell *)((void *)(arrayHeader));

		// NOTE: Since the array header is considered to be a part of the page
		// cell array, I don't use arrayHeader->cellAmount by itself.
		return &arrayHeaderAsCell[arrayHeader->cellAmount - 1];
	}

	PageLinkedList InitPageLinkedList(size_t bytesPerElement) {
		size_t bytesInPage = PAGE_LINKED_LIST_BYTES_IN_PAGE_AMOUNT();
		return (PageLinkedList) {
			.bytesPerElement = bytesPerElement,
			.defaultCellCap = bytesInPage / bytesPerElement,
			.cellArrayCap = bytesInPage / sizeof(PageCell)
		};
	}

	void DeinitPageLinkedList(PageLinkedList *linkedList) {
		PageCellArrayHeader *currentCellArray = linkedList->firstCellArray;
		while(currentCellArray != NULL) {
			PageCell *cells = PageCellArrayFirstCell(currentCellArray);
			for(int i = 0; i < currentCellArray->cellAmount; i++) {
				PAGE_LINKED_LIST_DEINIT_PAGE(&cells[i].buffer);
			}

			PageCellArrayHeader *tempCellArray = currentCellArray;
			currentCellArray = currentCellArray->nextCellArray;
			PAGE_LINKED_LIST_DEINIT_PAGE((uint8_t **)(&tempCellArray));
		}
	}

	PageCell *PageLinkedListAppendCustomPage(
		PageLinkedList *linkedList, size_t pageCap
	) {
		if(
			linkedList->lastCellArray->cellAmount >=
			linkedList->lastCellArray->cellCap
		) {
			PageCellArrayHeader *newCellArray = (PageCellArrayHeader *)(
				PAGE_LINKED_LIST_INIT_PAGE(
					sizeof(PageCell) * linkedList->cellArrayCap
				)
			);

			if(newCellArray == NULL) { return NULL; }

			*newCellArray = (PageCellArrayHeader) {
				.cellAmount = 1, .cellCap = linkedList->cellArrayCap
			};

			linkedList->lastCellArray->nextCellArray = newCellArray;
			linkedList->lastCellArray = newCellArray;
		}

		linkedList->lastCellArray->cellAmount++;
		PageCell *result = PageCellArrayLastCell(linkedList->lastCellArray);

		uint8_t *newPage = PAGE_LINKED_LIST_INIT_PAGE(
			pageCap * linkedList->bytesPerElement
		);

		if(newPage == NULL) { return NULL; }

		*result = (PageCell) {
			.amount = 0, .cap = pageCap,
			.buffer = newPage
		};

		return result;
	}

	PageCell *PageLinkedListAppendPage(PageLinkedList *linkedList) {
		return PageLinkedListAppendCustomPage(
			linkedList, linkedList->defaultCellCap
		);
	}

	uint8_t *PageLinkedListAppend(
		PageLinkedList *linkedList, size_t amountToAppend
	) {

		// NOTE: This is intended for extraordinarily large pages.
		if(amountToAppend > linkedList->defaultCellCap) {
			PageCell *newCell = PageLinkedListAppendCustomPage(
				linkedList, amountToAppend
			);

			if(newCell == NULL) { return NULL; }

			return newCell->buffer;
		}

		PageCell *lastCell = PageCellArrayLastCell(linkedList->lastCellArray);
		if(lastCell->amount + amountToAppend > lastCell->cap) {
			PageCell *newCell = PageLinkedListAppendPage(linkedList);
			if(newCell == NULL)  { return NULL; }
			lastCell = newCell;
		}

		uint8_t *result = &lastCell->buffer[
			linkedList->bytesPerElement * lastCell->amount
		];

		lastCell->amount += amountToAppend;
		return result;
	}

	PageLinkedListIterator InitPageLinkedListIterator(
		PageLinkedList *linkedList
	) {
		PageLinkedListIterator result = {
			.linkedList = linkedList,

			.currentCellArray = linkedList->firstCellArray
		};

		result.currentCell = PageCellArrayFirstCell(result.currentCellArray);
		result.currentElement = &result.currentCell->buffer[0];
		return result;
	}

	Bool PageLinkedListIteratorNextCellArray(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		for(
			int i = 0;
			iterator->currentCellArray != NULL && i < increaseAmount;
			i++
		) {
			iterator->currentCellArray = (
				iterator->currentCellArray->nextCellArray
			);
		}


		if(iterator->currentCellArray == NULL) { return FALSE; }
		return TRUE;
	}

	Bool PageLinkedListIteratorNextCell(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		iterator->currentCellIndex += increaseAmount;

		if(
			iterator->currentCellIndex >=
			iterator->currentCellArray->cellAmount
		) {

			// NOTE: This is a while loop that goes through the cells one by
			// one since assumptions about each individual page cell amount
			// can't be made without creating bugs.
			while(
				iterator->currentCellIndex >=
				iterator->currentCellArray->cellAmount
			) {
				iterator->currentCellIndex -= (
					iterator->currentCellArray->cellAmount
				);

				if(!PageLinkedListIteratorNextCellArray(iterator, 1)) {
					return FALSE;
				}
			}

			iterator->currentCell = PageCellArrayFirstCell(
				iterator->currentCellArray
			);

			iterator->currentCell += iterator->currentCellIndex;
			return TRUE;
		}

		iterator->currentCell += increaseAmount;
		return TRUE;
	}

	Bool PageLinkedListIteratorNext(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		iterator->currentElementIndex += increaseAmount;
		if(iterator->currentElementIndex >= iterator->currentCell->amount) {

			// NOTE: This is a while loop that goes through the cell's elements
			// one by one since assumptions about each individual page cell
			// amount can't me made without creating bugs.
			while(
				iterator->currentElementIndex >=
				iterator->currentCell->amount
			) {
				iterator->currentElementIndex -= iterator->currentCell->amount;
				if(!PageLinkedListIteratorNextCell(iterator, 1)) {
					return FALSE;
				}
			}
		}

		iterator->currentElement = &iterator->currentCell->buffer[
			iterator->linkedList->bytesPerElement *
			iterator->currentElementIndex
		];

		return TRUE;
	}

	size_t PageLinkedListGetTotalSize(PageLinkedList *linkedList) {
		size_t result = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		do {
			result += iterator.currentCell->amount;
		} while(PageLinkedListIteratorNextCell(&iterator, 1));

		return result;
	}

	PageCell PageLinkedListAsFlatMemory(PageLinkedList *linkedList) {
		size_t totalSize = PageLinkedListGetTotalSize(linkedList);
		PageCell result = {
			.amount = totalSize, .cap = totalSize,
			.buffer = PAGE_LINKED_LIST_INIT_PAGE(totalSize)
		};

		size_t currentIndex = 0;

		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		do {
			size_t byteAmount = (
				linkedList->bytesPerElement *
				iterator.currentCell->amount
			);

			memcpy(
				&result.buffer[currentIndex],
				&iterator.currentCell->buffer[0],
				byteAmount
			);

			currentIndex += byteAmount;
		} while(PageLinkedListIteratorNextCell(&iterator, 1));

		return result;
	}

	uint8_t *PageLinkedListGetAtIndex(
		PageLinkedList *linkedList, size_t index
	) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		if(!PageLinkedListIteratorNext(&iterator, index)) { return NULL; }
		return iterator.currentElement;
	}
#endif

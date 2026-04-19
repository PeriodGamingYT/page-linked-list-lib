#ifndef PAGE_LINKED_LIST_H
#define PAGE_LINKED_LIST_H
	#define PAGE_LINKED_LIST_VERSION 4
	#define PAGE_LINKED_LIST_IS_RELEASE 1

	// NOTE: This single header library is based off of a Gist, whose URL is as
	// follows:
	// https://gist.github.com/PeriodGamingYT/75f921f7b66e05100ce1f35da6b51baf

	//// Guard clauses to ensure user has given all appropriate information
	//// needed for library to operate.
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


	//// Base includes, defines, and typedefs.
	#include <stdint.h>

	// NOTE: Spaces are included between functions to deliniate that the
	// macros are not macros that take a form similar to that of a function.
	#define PAGE_LINKED_LIST_INIT_PAGE(_byteAmount) \
		PAGE_LINKED_LIST_INIT_PAGE_FUNC (_byteAmount)

	#define PAGE_LINKED_LIST_DEINIT_PAGE(_bufferPointer) \
		PAGE_LINKED_LIST_DEINIT_PAGE_FUNC (_bufferPointer)

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
	typedef struct PageCellArrayHeaderStruct PageCellArrayHeader;
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

		// NOTE: This is meant to store the previous size of the last
		// CustomSizeElement for PageLinkedListAppendWithSize().
		size_t prevByteAmount;
	} PageLinkedList;

	PageCell *PageCellArrayFirstCell(PageCellArrayHeader *);
	PageCell *PageCellArrayLastCell(PageCellArrayHeader *);

	PageLinkedList InitPageLinkedList(size_t);
	void DeinitPageLinkedList(PageLinkedList *);

	// NOTE: pageCap is in bytes.
	PageCell *PageLinkedListAppendCustomPage(PageLinkedList *, size_t);
	PageCell *PageLinkedListAppendPage(PageLinkedList *);
	uint8_t *PageLinkedListAppend(PageLinkedList *, size_t);

	typedef struct CustomSizeElementStruct {

		// NOTE: prevByteAmount makes it possible to pop things off of the
		// stack of CustomSizeElements.
		uint8_t byteAmount, prevByteAmount;

		// NOTE: Since the custom size elements shouldn't be seperated, the
		// buffer isn't a pointer, but instead it's attached to the end of a
		// CustomSizeElement.
		uint8_t buffer[1];
	} CustomSizeElement;

	// NOTE: If you want to use something akin to CustomSizeElement but have
	// no CustomSizeElement to actually point to, you can use this.
	typedef struct CustomSizeElementRefStruct {
		size_t byteAmount;
		uint8_t *buffer;
	} CustomSizeElementRef;

	// NOTE: These function's buffer pointers point back to their arguments.
	// As such, ensure that they are readily available to whatever relevant
	// functions will use them.
	//
	// REMARK: Break consistency with CustomSizeElement as the name would be
	// ridiculously long otherwise.
	CustomSizeElementRef CustomSizeElementIntoRef(
		CustomSizeElement *
	);

	CustomSizeElementRef CustomSizeElementRefFromCString(uint8_t *);

	// NOTE: This only will run if PageLinkedList.bytesPerElement == sizeof(
	// uint8_t).
	CustomSizeElement *PageLinkedListAppendWithSize(PageLinkedList *, size_t);

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

	// NOTE: Will only run if PageIterator.linkedList->bytesPerElement ==
	// sizeof(uint8_t).
	Bool PageLinkedListIteratorNextWithSize(PageLinkedListIterator *, size_t);

	size_t PageLinkedListGetTotalSize(PageLinkedList *);
	PageCell PageLinkedListAsFlatMemory(PageLinkedList *);

	uint8_t *PageLinkedListGetAtIndex(PageLinkedList *, size_t);

	// NOTE: Since this function can't assume that all elements will be evenly
	// spaced out, it must go through the elements one by one and not make
	// any assumptions about spacing. As such, it's advised to use this
	// function sparingly as it can cause performance issues.
	CustomSizeElement *PageLinkedListGetAtIndexWithSize(
		PageLinkedList *, size_t
	);
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

			// NOTE: Since the first page cell (The array header) was skipped,
			// then the loop should go through one less than the amount.
			for(int i = 0; i < currentCellArray->cellAmount - 1; i++) {
				if(
					cells[i].amount <= 0 || cells[i].cap <= 0 ||
					cells[i].buffer == NULL
				) { continue; }

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

		// NOTE: If the first cell array in a page linked list is null, this
		// means that the last cell array will also be null.
		if(linkedList->firstCellArray == NULL) {
			PageCellArrayHeader *newCellArray = (PageCellArrayHeader *)(
				PAGE_LINKED_LIST_INIT_PAGE(
					sizeof(PageCell) * linkedList->cellArrayCap
				)
			);

			if(newCellArray == NULL) { return NULL; }

			*newCellArray = (PageCellArrayHeader) {
				.cellAmount = 1, .cellCap = linkedList->cellArrayCap
			};

			linkedList->firstCellArray = newCellArray;
			linkedList->lastCellArray = newCellArray;
		}

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
		if(linkedList->firstCellArray == NULL) {
			PageLinkedListAppendPage(linkedList);
		}

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
			if(newCell == NULL) { return NULL; }
			lastCell = newCell;
		}

		uint8_t *result = &lastCell->buffer[
			linkedList->bytesPerElement * lastCell->amount
		];

		lastCell->amount += amountToAppend;
		return result;
	}


	CustomSizeElementRef CustomSizeElementIntoRef(CustomSizeElement *element) {
		return (CustomSizeElementRef) {
			.byteAmount = element->byteAmount,
			.buffer = &element->buffer[0]
		};
	}

	CustomSizeElementRef CustomSizeElementRefFromCString(uint8_t *string) {
		int stringCharAmount = 0;
		for(; string[stringCharAmount] != 0; stringCharAmount++);
		return (CustomSizeElementRef) {
			.byteAmount = stringCharAmount * sizeof(uint8_t),
			.buffer = &string[0]
		};
	}


	CustomSizeElement *PageLinkedListAppendWithSize(
		PageLinkedList *linkedList, size_t bufferByteAmount
	) {

		// NOTE: 1 is subtracted because CustomSizeElement's buffer is only
		// meant as a convinience to access a CustomSizeElement's buffer
		// without the use of a helper function or pointer property. But since
		// buffer is an actual property in CustomSizeElement, it's still
		// counted towards sizeof(CustomSizeElement). Thus, when calculating
		// the true size of byteAmount, this discrepancy must be accounted
		// for.
		size_t byteAmount = sizeof(CustomSizeElement) + bufferByteAmount - 1;
		CustomSizeElement *result = (CustomSizeElement *)(PageLinkedListAppend(
			linkedList, byteAmount
		));

		if(result == NULL) { return NULL; }
		*result = (CustomSizeElement) {
			.byteAmount = bufferByteAmount,
			.prevByteAmount = linkedList->prevByteAmount
		};

		linkedList->prevByteAmount = bufferByteAmount;

		return result;
	}


	PageLinkedListIterator InitPageLinkedListIterator(
		PageLinkedList *linkedList
	) {
		return (PageLinkedListIterator) { .linkedList = linkedList };
	}

	Bool PageLinkedListIteratorNextCellArray(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		if(
			increaseAmount == 0 ||
			iterator->linkedList->firstCellArray == NULL
		) { return FALSE; }

		if(iterator->currentCellArray == NULL) {
			iterator->currentCellArray = iterator->linkedList->firstCellArray;
			increaseAmount--;
		}

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
		if(increaseAmount == 0) { return FALSE; }
		if(iterator->currentCell == NULL) {
			if(
				iterator->currentCellArray == NULL &&
				!PageLinkedListIteratorNextCellArray(iterator, 1)
			) { return FALSE; }

			iterator->currentCell = PageCellArrayFirstCell(
				iterator->currentCellArray
			);

			increaseAmount--;
		}

		iterator->currentCellIndex += increaseAmount;

		// NOTE: Since the current cell index starts at the first cell and
		// amount accounts the first cell array (for consistency with
		// PageCell, since they are two sides of the same coin), subtracting
		// by two is the only solution I could come up with that would keep
		// this consistency.
		if(
			iterator->currentCellIndex >
			iterator->currentCellArray->cellAmount - 2
		) {

			// NOTE: This is a while loop that goes through the cells one by
			// one since assumptions about each individual page cell amount
			// can't be made without creating bugs.
			while(
				iterator->currentCellIndex >
				iterator->currentCellArray->cellAmount - 2
			) {
				iterator->currentCellIndex -= (
					iterator->currentCellArray->cellAmount - 1
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
		if(increaseAmount == 0) { return FALSE; }
		if(iterator->currentElement == NULL) {
			if(
				iterator->currentCell == NULL &&
				!PageLinkedListIteratorNextCell(iterator, 1)
			) { return FALSE; }

			increaseAmount--;
		}

		iterator->currentElementIndex += increaseAmount;
		if(iterator->currentElementIndex > iterator->currentCell->amount - 1) {

			// NOTE: This is a while loop that goes through the cell's elements
			// one by one since assumptions about each individual page cell
			// amount can't me made without creating bugs.
			while(
				iterator->currentElementIndex >
				iterator->currentCell->amount - 1
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

	Bool PageLinkedListIteratorNextWithSize(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		if(iterator->linkedList->bytesPerElement != sizeof(uint8_t)) {
			return FALSE;
		}

		if(iterator->currentElement == NULL) {
			if(!PageLinkedListIteratorNext(iterator, 1)) { return FALSE; }
			increaseAmount--;
		}

		for(int i = 0; i < increaseAmount; i++) {
			CustomSizeElement *element = (CustomSizeElement *)(
				iterator->currentElement
			);

			if(!PageLinkedListIteratorNext(
				iterator,
				sizeof(CustomSizeElement) + element->byteAmount - 1
			)) { return FALSE; }
		}

		return TRUE;
	}

	size_t PageLinkedListGetTotalSize(PageLinkedList *linkedList) {
		size_t result = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		while(PageLinkedListIteratorNextCell(&iterator, 1)) {
			result += iterator.currentCell->amount;
		}

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

		while(PageLinkedListIteratorNextCell(&iterator, 1)) {
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
		}

		return result;
	}

	uint8_t *PageLinkedListGetAtIndex(
		PageLinkedList *linkedList, size_t index
	) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		// NOTE: Since PageLinkedListIterator can be initalized with a yet
		// to be initalized PageLinkedList. Calling PageLinkedListNext*()
		// will initalize its corresponding iteration index and element/cell/
		// cell array and decrease the amount to iterate by 1.
		if(!PageLinkedListIteratorNext(&iterator, index + 1)) { return NULL; }
		return iterator.currentElement;
	}

	CustomSizeElement *PageLinkedListGetAtIndexWithSize(
		PageLinkedList *linkedList, size_t index
	) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		if(!PageLinkedListIteratorNextWithSize(
			&iterator, index + 1
		)) { return NULL; }

		return (CustomSizeElement *)(iterator.currentElement);
	}
#endif

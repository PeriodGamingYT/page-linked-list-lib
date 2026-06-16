#ifndef PAGE_LINKED_LIST_H
#define PAGE_LINKED_LIST_H
	#define PAGE_LINKED_LIST_VERSION 5
	#define PAGE_LINKED_LIST_IS_RELEASE 0

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

	#define USE_DEFAULT_CAP ((size_t)(-1))

	typedef uint8_t Bool;


	//// Single header structs and forward/unfilled function declarations.

	// NOTE: PageCell isn't meant to be called for allocation by itself,
	// whereas PageLinkedList and SinglePageCell are meant to be allocated
	// to. PageCell *does* contain allocations, but it doesn't have enough
	// context about the memory allocations happening to be empowered enough
	// to do allocations on its own.
	typedef struct PageCell {
		size_t amount, prevAmount, cap;
		uint8_t *buffer;
	} PageCell;

	// NOTE: This struct is identical to PageCell and is only here to clarify
	// semantic meaning. Casting a PageCell to a PageCellArrayHeaderStruct and
	// vice versa results in no data loss.
	typedef struct PageCellArrayHeader PageCellArrayHeader;
	typedef struct PageCellArrayHeader {
		size_t cellAmount, cellCap;
		PageCellArrayHeader *nextCellArray;
	} PageCellArrayHeader;

	typedef struct PageLinkedList {

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
	PageCell *AddCustomPageToPageLinkedList(PageLinkedList *, size_t);
	PageCell *AddPageToPageLinkedList(PageLinkedList *);
	uint8_t *AddToPageLinkedList(PageLinkedList *, size_t);

	typedef struct Slice {

		// NOTE: prevByteAmount makes it possible to pop things off of the
		// stack of Slices for a given page.
		//
		// NOTE: Since the custom size elements shouldn't be seperated, the
		// buffer isn't a pointer, but instead it's attached to the end of a
		// Slice.
		uint8_t byteAmount, prevByteAmount, buffer[];
	} Slice;

	// NOTE: If you want to use something akin to Slice but have
	// no Slice to actually point to, you can use this. However, since
	// SliceRef doesn't have functionality to be utilized by AddTo() type
	// functions, nor does it need to, is lacks prevByteAmount.
	typedef struct SliceRef {
		size_t byteAmount;
		uint8_t *buffer;
	} SliceRef;

	// NOTE: This will only store the size of the pointer, not the size of
	// the reference the pointer is actually pointing to.
	#define SLICE_REF_FROM_VAR(_var) \
		(SliceRef) { \
			.byteAmount = sizeof(_var), \
			.buffer = (uint8_t *)(&(_var)) \
		}

	// NOTE: These function's buffer pointers point back to their arguments.
	// As such, ensure that they are readily available to whatever relevant
	// functions will use them.
	SliceRef SliceIntoRef(Slice *);
	SliceRef SliceRefFromCString(uint8_t *);

	// NOTE: This only will run if PageLinkedList.bytesPerElement == sizeof(
	// uint8_t).
	Slice *AddSliceToPageLinkedList(PageLinkedList *, size_t);

	typedef struct SinglePageCell SinglePageCell;
	Slice *AddSliceToSinglePageCell(SinglePageCell *, size_t);

	typedef struct PageLinkedListIterator {
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
	Bool IterateNextCellArrayInPageLinkedList(PageLinkedListIterator *, size_t);
	Bool IterateNextCellInPageLinkedList(PageLinkedListIterator *, size_t);
	Bool IterateNextInPageLinkedList(PageLinkedListIterator *, size_t);

	// NOTE: Will only run if PageIterator.linkedList->bytesPerElement ==
	// sizeof(uint8_t).
	Bool IterateNextSliceInPageLinkedList(PageLinkedListIterator *, size_t);

	size_t PageLinkedListGetTotalSize(PageLinkedList *);
	PageCell PageLinkedListToPageCell(PageLinkedList *);

	uint8_t *GetFromIndexForPageLinkedList(PageLinkedList *, size_t);

	// NOTE: Since this function can't assume that all elements will be evenly
	// spaced out, it must go through the elements one by one and not make
	// any assumptions about spacing. As such, it's advised to use this
	// function sparingly as it can cause performance issues.
	Slice *GetSliceFromIndexForPageLinkedList(PageLinkedList *, size_t);

	// NOTE: SinglePageCell can not convert to PageCell with no data loss,
	// it is meant to be used by itself. If you have a piece of memory that
	// will need to be flat and will often not change, you can use this.
	typedef struct SinglePageCell {

		// NOTE: This union makes it trivial to copy functionality from
		// PageCell to SinglePageCell, making SinglePageCell functions simple
		// wrappers, while also allowing convenient access to data members of
		// a given SinglePageCell.
		union {
			struct {
				size_t amount, prevAmount, cap;
				uint8_t *buffer;
			};

			PageCell pageCell;
		};

		size_t bytesPerElement;
	} SinglePageCell;

	SinglePageCell InitSinglePageCell(size_t, size_t);
	void DeinitSinglePageCell(SinglePageCell *);
	uint8_t *AddToSinglePageCell(SinglePageCell *, size_t);
	SinglePageCell SinglePageCellFromPageLinkedList(PageLinkedList *);
	uint8_t *GetFromIndexForSinglePageCell(SinglePageCell *, size_t);
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

	static void DeinitPageCell(PageCell *cell) {
		PAGE_LINKED_LIST_DEINIT_PAGE(&cell->buffer);
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

				DeinitPageCell(&cells[i]);
			}

			PageCellArrayHeader *tempCellArray = currentCellArray;
			currentCellArray = currentCellArray->nextCellArray;
			PAGE_LINKED_LIST_DEINIT_PAGE((uint8_t **)(&tempCellArray));
		}
	}

	static PageCell InitPageCell(size_t pageCap, size_t bytesPerElement) {
		if(pageCap == USE_DEFAULT_CAP) {
			pageCap = PAGE_LINKED_LIST_BYTES_IN_PAGE_AMOUNT();
		}

		uint8_t *buffer = PAGE_LINKED_LIST_INIT_PAGE(
			pageCap * bytesPerElement
		);

		if(buffer == NULL) { return (PageCell) { 0 }; }
		return (PageCell) { .cap = pageCap, .buffer = buffer };
	}

	PageCell *AddCustomPageToPageLinkedList(
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
		*result = InitPageCell(pageCap, linkedList->bytesPerElement);
		return result;
	}

	PageCell *AddPageToPageLinkedList(PageLinkedList *linkedList) {
		return AddCustomPageToPageLinkedList(
			linkedList, linkedList->defaultCellCap
		);
	}

	static uint8_t *AddToPageCell(
		PageCell *cell,
		size_t amountToAdd, size_t bytesPerElement
	) {
		if(amountToAdd + cell->amount > cell->cap) { return NULL; }

		uint8_t *result = &cell->buffer[bytesPerElement * cell->amount];
		cell->prevAmount = cell->amount;
		cell->amount += amountToAdd;
		return result;
	}

	uint8_t *AddToPageLinkedList(
		PageLinkedList *linkedList, size_t amountToAdd
	) {
		if(linkedList->firstCellArray == NULL) {
			AddPageToPageLinkedList(linkedList);
		}

		// NOTE: This is intended for extraordinarily large pages.
		if(amountToAdd > linkedList->defaultCellCap) {
			PageCell *newCell = AddCustomPageToPageLinkedList(
				linkedList, amountToAdd
			);

			if(newCell == NULL) { return NULL; }

			return newCell->buffer;
		}

		PageCell *lastCell = PageCellArrayLastCell(linkedList->lastCellArray);
		if(lastCell->amount + amountToAdd > lastCell->cap) {
			PageCell *newCell = AddPageToPageLinkedList(linkedList);
			if(newCell == NULL) { return NULL; }
			lastCell = newCell;
		}

		return AddToPageCell(
			lastCell,
			amountToAdd, linkedList->bytesPerElement
		);
	}


	SliceRef SliceIntoRef(Slice *element) {
		return (SliceRef) {
			.byteAmount = element->byteAmount,
			.buffer = &element->buffer[0]
		};
	}

	SliceRef SliceRefFromCString(uint8_t *string) {
		int stringCharAmount = 0;
		for(; string[stringCharAmount] != 0; stringCharAmount++);

		// NOTE(ElkElan): Bring in null terminator so that the whole string
		// is shown, since Slice/Ref doesn't work on null
		// termination.
		stringCharAmount++;
		return (SliceRef) {
			.byteAmount = stringCharAmount * sizeof(uint8_t),
			.buffer = &string[0]
		};
	}

	static size_t ByteAmountForSlice(size_t byteAmount) {
		return sizeof(Slice) + byteAmount;
	}

	static Slice *InitSliceFromData(
		uint8_t *sliceAsData, size_t byteAmount, size_t prevByteAmount
	) {
		if(sliceAsData == NULL) { return NULL; }

		Slice *slice = (Slice *)(sliceAsData);
		*slice = (Slice) {
			.byteAmount = byteAmount, .prevByteAmount = prevByteAmount
		};

		return slice;
	}

	static Slice *InitSliceFromDataForPageCell(
		PageCell *cell, uint8_t *sliceAsData, size_t byteAmount
	) {
		return InitSliceFromData(
			sliceAsData, byteAmount, cell->amount - cell->prevAmount
		);
	}

	Slice *AddSliceToPageLinkedList(
		PageLinkedList *linkedList, size_t amountToAdd
	) {
		if(linkedList->bytesPerElement != sizeof(uint8_t)) { return NULL; }

		size_t byteAmount = ByteAmountForSlice(amountToAdd);
		uint8_t *sliceAsData = AddToPageLinkedList(linkedList, byteAmount);
		return InitSliceFromDataForPageCell(
			PageCellArrayLastCell(linkedList->lastCellArray),
			sliceAsData, byteAmount
		);
	}

	Slice *AddSliceToSinglePageCell(
		SinglePageCell *cell, size_t amountToAdd
	) {
		if(cell->bytesPerElement != sizeof(uint8_t)) { return NULL; }

		size_t byteAmount = ByteAmountForSlice(amountToAdd);
		uint8_t *sliceAsData = AddToSinglePageCell(cell, byteAmount);
		return InitSliceFromDataForPageCell(
			&cell->pageCell, sliceAsData, byteAmount
		);
	}


	PageLinkedListIterator InitPageLinkedListIterator(
		PageLinkedList *linkedList
	) {
		return (PageLinkedListIterator) { .linkedList = linkedList };
	}

	Bool IterateNextCellArrayInPageLinkedList(
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

	Bool IterateNextCellInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		if(increaseAmount == 0) { return FALSE; }
		if(iterator->currentCell == NULL) {
			if(
				iterator->currentCellArray == NULL &&
				!IterateNextCellArrayInPageLinkedList(iterator, 1)
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

				if(!IterateNextCellArrayInPageLinkedList(iterator, 1)) {
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

	Bool IterateNextInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		if(increaseAmount == 0) { return FALSE; }
		if(iterator->currentElement == NULL) {
			if(
				iterator->currentCell == NULL &&
				!IterateNextCellInPageLinkedList(iterator, 1)
			) { return FALSE; }

			increaseAmount--;
		}

		iterator->currentElementIndex += increaseAmount;
		if(iterator->currentElementIndex > iterator->currentCell->amount - 1) {

			// NOTE: This is a while loop that goes through the cell's elements
			// one by one since assumptions about each individual page cell
			// amount can't be made without creating bugs.
			while(
				iterator->currentElementIndex >
				iterator->currentCell->amount - 1
			) {
				iterator->currentElementIndex -= iterator->currentCell->amount;
				if(!IterateNextCellInPageLinkedList(iterator, 1)) {
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

	Bool IterateNextSliceInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount
	) {
		if(iterator->linkedList->bytesPerElement != sizeof(uint8_t)) {
			return FALSE;
		}

		if(iterator->currentElement == NULL) {
			if(!IterateNextInPageLinkedList(iterator, 1)) { return FALSE; }
			increaseAmount--;
		}

		for(int i = 0; i < increaseAmount; i++) {
			Slice *element = (Slice *)(
				iterator->currentElement
			);

			if(!IterateNextInPageLinkedList(
				iterator,
				sizeof(Slice) + element->byteAmount - 1
			)) { return FALSE; }
		}

		return TRUE;
	}

	size_t PageLinkedListGetTotalSize(PageLinkedList *linkedList) {
		size_t result = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		while(IterateNextCellInPageLinkedList(&iterator, 1)) {
			result += iterator.currentCell->amount;
		}

		return result;
	}

	PageCell PageLinkedListToPageCell(PageLinkedList *linkedList) {
		size_t totalSize = PageLinkedListGetTotalSize(linkedList);
		PageCell result = {
			.amount = totalSize, .cap = totalSize,
			.buffer = PAGE_LINKED_LIST_INIT_PAGE(totalSize)
		};

		size_t currentIndex = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		while(IterateNextCellInPageLinkedList(&iterator, 1)) {
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

	uint8_t *GetFromIndexForPageLinkedList(
		PageLinkedList *linkedList, size_t index
	) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		// NOTE: Since PageLinkedListIterator can be initalized with a yet
		// to be initalized PageLinkedList. Calling PageLinkedListNext...()
		// will initalize its corresponding iteration index and element/cell/
		// cell array and decrease the amount to iterate by 1.
		if(!IterateNextInPageLinkedList(&iterator, index + 1)) { return NULL; }
		return iterator.currentElement;
	}

	Slice *GetSliceFromIndexForPageLinkedList(
		PageLinkedList *linkedList, size_t index
	) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList
		);

		if(!IterateNextSliceInPageLinkedList(&iterator, index + 1)) { return NULL; }
		return (Slice *)(iterator.currentElement);
	}


	SinglePageCell InitSinglePageCell(size_t bytesPerElement, size_t cap) {
		return (SinglePageCell) {
			.bytesPerElement = bytesPerElement,
			.pageCell = InitPageCell(cap, bytesPerElement)
		};
	}

	void DeinitSinglePageCell(SinglePageCell *singlePageCell) {
		DeinitPageCell(&singlePageCell->pageCell);
	}

	uint8_t *AddToSinglePageCell(
		SinglePageCell *singlePageCell, size_t amountToAdd
	) {
		return AddToPageCell(
			&singlePageCell->pageCell,
			amountToAdd, singlePageCell->bytesPerElement
		);
	}

	// NOTE: If you want a SinglePageCell that has slightly more information
	// about what you're working with, then calling this will work.
	SinglePageCell SinglePageCellFromPageLinkedList(
		PageLinkedList *linkedList
	) {
		return (SinglePageCell) {
			.pageCell = PageLinkedListToPageCell(linkedList),
			.bytesPerElement = linkedList->bytesPerElement,
		};
	}

	uint8_t *GetFromIndexForSinglePageCell(
		SinglePageCell *cell, size_t index
	) {
		return &cell->buffer[index * cell->bytesPerElement];
	}
#endif

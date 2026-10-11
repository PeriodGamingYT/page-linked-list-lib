#ifndef PAGE_LINKED_LIST_H
#define PAGE_LINKED_LIST_H

	// TODO: Bounds-checks need to be implemented basically anywhere that
	// deals with sizes and indices.
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

	// NOTE: A convention for Slice/SliceRef is that byteAmount will exclude
	// byteAmount and prevByteAmount.
	//
	// NOTE: For string slices, byteAmount will include the null terminator,
	// but since SliceRef doesn't necessarily need a null terminator (nor does
	// Slice, but since Slice is found in PageCells, null terminators are the
	// norm for Slices). So when copying from a SliceRef into a Slice, do not
	// copy over the null terminator, instead, you must put the null
	// terminator in manually.
	//
	// NOTE: If you are making an extension to a Slice, then make your struct
	// first as:
	// typedef struct YourStruct { union {
	// 	Slice slice;
	// 	struct {
	// 		uint8_t byteAmount, prevByteAmount;
	// 		// Other properties go here.
	// 	};
	// }; } MyStruct;
	//
	// After this, for allocation purposes, make this function:
	// // NOTE: sizeof(MyStruct) + ... - sizeof(Slice) isn't needed, and also
	// // won't work for tagged unions that vary in size. If this is the case,
	// // forgo using the above and just compute the size of your struct
	// // excluding the Slice manually.
	// size_t ByteAmountForMyStruct() {
	// 	return sizeof(MyStruct) + /* FAM stuff, if any. */ - sizeof(Slice);
	// }
	//
	// If you are going to make any functions that get the size of your FAM,
	// then make sure to subtract sizeof(Slice) to isolate the byte amount of
	// just your struct as well as any other things you need to subtract
	// and isolate against.
	//
	// NOTE: Just to clarify, byteAmount (and by extension, prevByteAmount)
	// encompass *ALL* of the bytes in a slice, not just the user data in a
	// slice, meaning byteAmount and prevByteAmount are counted towards the
	// total byte amount in byteAmount.
	typedef struct Slice {

		// NOTE: prevByteAmount makes it possible to pop things off of the
		// stack of Slices for a given page.
		//
		// NOTE: Since the custom size elements shouldn't be seperated, the
		// buffer isn't a pointer, but instead it's attached to the end of a
		// Slice.
		uint8_t byteAmount, prevByteAmount, buffer[];
	} Slice;

	size_t GetBufferByteAmountInSlice(size_t);
	size_t ByteAmountInSlice(size_t);
	size_t GetStringByteAmountInSlice(size_t);

	// NOTE: The input for the byte amount does not include the null
	// terminator.
	size_t ByteAmountForStringSlice(size_t);

	typedef struct PageHeader { union {
		Slice slice;
		struct {
			uint8_t byteAmount, prevByteAmount, elementByteAmount;
			uint16_t elementAmount, elementCap;
			struct PageHeader *prev, *next;
		};
	}; } PageHeader;

	PageHeader *InitPageHeader(size_t, size_t);
	PageHeader *InitPageHeaderFromHeader(PageHeader *);
	void DeinitPageHeader(PageHeader **);
	uint8_t *FirstElementInPage(PageHeader *);
	uint8_t *NthElementInPage(PageHeader *, size_t);

	typedef struct PageLinkedList {
		PageHeader *head, *tail;
		size_t defaultPageByteCap, defaultElementByteAmount;
	} PageLinkedList;

	PageLinkedList InitPageLinkedList(size_t, size_t);
	void DeinitPageLinkedList(PageLinkedList *);
	PageHeader *AddPageToLinkedListFromHeader(PageLinkedList *, PageHeader *);
	PageHeader *AddPageToLinkedList(PageLinkedList *);
	void RemovePageFromLinkedList(PageLinkedList *);
	uint8_t *AddElementsToPageLinkedList(PageLinkedList *, size_t);

	// NOTE: Removing an element amount over the current page's element amount
	// will go to the next page and work on the remaining element amounts left.
	// If all pages are exhausted, then the function will return early and not
	// error.
	void RemoveElementsFromPageLinkedList(PageLinkedList *, size_t);

	typedef struct PageLinkedListIter {
		PageLinkedList *linkedList;
		PageHeader *pageHeader;
		uint8_t *element; size_t elementIndex;
	} PageLinkedListIter;

	PageLinkedListIter InitPageLinkedListIter(PageLinkedList *);
	Bool NextPageInLinkedListIter(PageLinkedListIter *, size_t);
	Bool NextElementInLinkedListIter(PageLinkedListIter *, size_t);

	size_t TotalElementAmountOfPageLinkedList(PageLinkedList *);
	uint8_t *NthElementInPageLinkedList(PageLinkedList *, size_t);

	/*
	// TODO: Check if PageLinkedListIterator needs the same bytePerElement
	// for every page it encounters.
	typedef struct PageLinkedListIterator {
		PageLinkedList *linkedList;

		PageCellArrayHeader *currentCellArray;

		PageCell *currentCell;
		size_t currentCellIndex;

		uint8_t *currentElement;
		size_t currentElementIndex;
	} PageLinkedListIterator;

	// TODO?: Make PageLinkedListIterator be able to iterate just on a
	// PageCell.
	//
	// NOTE: Since PageLinkedListIterator doesn't hold any memory on its own,
	// DeinitPageLinkedListIterator() isn't necessary.
	PageLinkedListIterator InitPageLinkedListIterator(PageLinkedList *);

	// NOTE: These IterateNext...() functions will only reset their relevant
	// cell array/cell/element. Resetting any other relevant cell/element is
	// the responsibility of the caller.
	Bool IterateNextCellArrayInPageLinkedList(
		PageLinkedListIterator *, size_t);

	Bool IterateNextCellInPageLinkedList(PageLinkedListIterator *, size_t);
	Bool IterateNextInPageLinkedList(PageLinkedListIterator *, size_t);

	size_t GetTotalSizeForPageLinkedList(PageLinkedList *);
	uint8_t *GetViaIndexForPageLinkedList(PageLinkedList *, size_t);
	*/

	// NOTE: If you want to use something akin to Slice but have
	// no Slice to actually point to, you can use this. However, since
	// SliceRef doesn't have functionality to be utilized by AddTo() type
	// functions, nor does it need to, is lacks prevByteAmount.
	//
	// NOTE: The byteAmount of SliceRef will imitate the byteAmount for Slice
	// in order to maintain compatability with Slice functions such as
	// Get...AmountIn...(), and ByteAmountFor...().
	typedef struct SliceRef {
		size_t byteAmount;

		// NOTE: This is meant for string SliceRefs, where zeroByteAmount = 1,
		// totalByteAmount != byteAmount + zeroByteAmount, nonzeroByteAmount =
		// byteAmount - zeroByteAmount.
		//
		// TODO?: Bit of a bad solution, better solutions will be able to
		// better expand upon on how to copy slice refs without needing
		// functions that require the user to remember the "type" of slice they
		// are copying, which in some cases is not an option. But also, it will
		// suffice for now.
		size_t zeroByteAmount;
		uint8_t *buffer;
	} SliceRef;

	// NOTE: These functions will not copy if the byteAmount of the two
	// SliceRefs don't match (or zeroByteAmount, for that matter).
	//
	// NOTE: First SliceRef not shortened in order to remove ambiguity.
	void CopySliceRefToRef(SliceRef, SliceRef);

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
	Bool AreSliceRefsEqual(SliceRef, SliceRef);
	// SliceRef SliceIntoRef(Slice *);
	SliceRef SliceRefFromCString(uint8_t *);

	/*
	// NOTE: This suite of functions below will only work if .bytesPerElement
	// == sizeof(uint8_t).

	// For PageCell.
	Slice *AddSliceToPageCell(PageCell *, size_t);
	SliceRef SliceRefFromPageCell(PageCell *);

	// For PageLinkedList.
	Slice *AddSliceToPageLinkedList(PageLinkedList *, size_t);
	Bool IterateNextSliceInPageLinkedList(PageLinkedListIterator *, size_t);

	// NOTE: Since this function can't assume that all elements will be evenly
	// spaced out, it must go through the elements one by one and not make
	// any assumptions about spacing. As such, it's advised to use this
	// function sparingly as it can cause performance issues.
	//
	// TODO: Transferring a slice to a new page, since this opens room, perhaps
	// a slab allocator is in order too?
	Slice *GetSliceFromIndexForPageLinkedList(PageLinkedList *, size_t);

	// NOTE: Since Slice is required to be continuous, it can't be extended to
	// another page. As such, the builder only works on PageCells but does
	// allow the user to Init a builder from a PageLinkedList.
	typedef struct SliceBuilder {
		PageCell *cell; Slice *slice;
	} SliceBuilder;

	SliceBuilder InitSliceBuilderFromPageCell(PageCell *);
	SliceBuilder InitSliceBuilder(PageLinkedList *);
	uint8_t *BuildUponSliceBuilder(SliceBuilder *, size_t, uint8_t *);
	uint8_t *BuildUponSliceBuilderWithRef(SliceBuilder *, SliceRef);
	*/
#endif

// NOTE: This is put outside of the guard clause because a user might include
// only the header part and then in another file include the implementation
// part.
#ifdef PAGE_LINKED_LIST_IMPL
	size_t GetBufferByteAmountInSlice(size_t byteAmount) {
		if(byteAmount < sizeof(Slice)) { return 0; }
		return byteAmount - sizeof(Slice);
	}

	size_t ByteAmountForSlice(size_t bufferByteAmount) {
		return sizeof(Slice) + bufferByteAmount;
	}

	size_t GetStringByteAmountInSlice(size_t byteAmount) {
		if(byteAmount < sizeof(Slice) + sizeof(uint8_t)) { return 0; }
		return (
			GetBufferByteAmountInSlice(byteAmount) -
			sizeof(uint8_t) // Null terminator.
		);
	}

	size_t ByteAmountForStringSlice(size_t stringByteAmount) {
		return ByteAmountForSlice(
			stringByteAmount + sizeof(uint8_t) // Null terminator.
		);
	}

	PageHeader *InitPageHeader(size_t pageByteCap, size_t elementByteAmount) {
		if(pageByteCap == USE_DEFAULT_CAP) {
			pageByteCap = PAGE_LINKED_LIST_BYTES_IN_PAGE_AMOUNT();
		}

		uint8_t *buffer = PAGE_LINKED_LIST_INIT_PAGE(pageByteCap);
		if(buffer == NULL) { return NULL; }

		PageHeader *header = (PageHeader *)(buffer);
		*header = (PageHeader) {
			.byteAmount = sizeof(PageHeader),
			.elementByteAmount = elementByteAmount,
			.elementCap = (int)(pageByteCap / elementByteAmount)
		};

		return header;
	}

	PageHeader *InitPageHeaderFromHeader(PageHeader *header) {
		return InitPageHeader(
			header->elementCap * header->elementByteAmount,
			header->elementByteAmount);
	}

	void DeinitPageHeader(PageHeader **header) {
		PAGE_LINKED_LIST_DEINIT_PAGE((uint8_t **)(header));
	}

	uint8_t *FirstElementInPage(PageHeader *header) {
		return &(((uint8_t *)(header))[header->byteAmount]);
	}

	uint8_t *NthElementInPage(PageHeader *header, size_t index) {
		return &FirstElementInPage(header)[index * header->elementByteAmount];
	}

	PageLinkedList InitPageLinkedList(
		size_t pageByteCap, size_t elementByteAmount) {
		return (PageLinkedList) {
			.defaultPageByteCap = pageByteCap,
			.defaultElementByteAmount = elementByteAmount
		};
	}

	void DeinitPageLinkedList(PageLinkedList *linkedList) {
		for(
			PageHeader *header = linkedList->head;
			header != NULL;
			/* ... */
		) {
			PageHeader *next = header->next;
			DeinitPageHeader(&next);
			header = next;
		}
	}

	PageHeader *AddPageToLinkedListFromHeader(
		PageLinkedList *linkedList, PageHeader *header) {
		PageHeader *newPage = InitPageHeaderFromHeader(header);
		if(newPage == NULL) { return NULL; }

		linkedList->tail->next = newPage;
		newPage->prev = linkedList->tail;

		linkedList->tail = newPage;
	}

	PageHeader *AddPageToLinkedList(PageLinkedList *linkedList) {
		return AddPageToLinkedListFromHeader(linkedList, linkedList->tail);
	}

	void RemovePageFromLinkedList(PageLinkedList *linkedList) {
		PageHeader *newTail = linkedList->tail->prev;
		DeinitPageHeader(&linkedList->tail);

		if(newTail == NULL) {
			linkedList->head = NULL;
			return;
		}

		linkedList->tail = newTail;
	}

	uint8_t *AddElementsToPageLinkedList(
		PageLinkedList *linkedList, size_t elementAmount) {
		size_t sizeAfterAddition = (
			linkedList->tail->elementAmount + elementAmount);

		if(linkedList->tail->elementCap < sizeAfterAddition) {
			AddPageToLinkedList(linkedList);
		}

		uint8_t *result = NthElementInPage(
			linkedList->tail, linkedList->tail->elementAmount - 1);

		linkedList->tail->elementAmount += elementAmount;
		return result;
	}

	void RemoveElementsFromPageLinkedList(
		PageLinkedList *linkedList, size_t elementAmount) {
		while(linkedList->tail != NULL && elementAmount > 0) {
			if(linkedList->tail->elementAmount > elementAmount) {
				linkedList->tail->elementAmount -= elementAmount;
				return;
			}

			elementAmount -= linkedList->tail->elementAmount;
			RemovePageFromLinkedList(linkedList);
		}
	}


	PageLinkedListIter InitPageLinkedListIter(PageLinkedList *linkedList) {
		return (PageLinkedListIter) { .linkedList = linkedList };
	}

	Bool NextPageInLinkedListIter(
		PageLinkedListIter *iter, size_t advancePageAmount) {
		if(iter->pageHeader == NULL && advancePageAmount >= 1) {
			iter->pageHeader = iter->linkedList->head;
			advancePageAmount--;
		}

		for(
			;
			advancePageAmount > 0 && iter->pageHeader != NULL;
			advancePageAmount--) {
			iter->pageHeader = iter->pageHeader->next;
		}

		return iter->pageHeader != NULL;
	}

	Bool NextElementInLinkedListIter(
		PageLinkedListIter *iter, size_t advanceElementAmount) {
		if(iter->pageHeader == NULL) {
			NextPageInLinkedListIter(iter, 1);

			// NOTE: Most likely true and safe to assume as an invariant, but
			// semantic wise it sends a message about the intent of this if
			// and the if below this if.
			iter->element = NULL;
		}

		if(iter->element == NULL && advanceElementAmount >= 1) {
			iter->element = FirstElementInPage(iter->pageHeader);
			iter->elementIndex = 0;
			advanceElementAmount--;
		}

		while(advanceElementAmount > 0 && iter->pageHeader != NULL) {
			if(
				iter->elementIndex + advanceElementAmount <
				iter->pageHeader->elementAmount
			) {
				iter->element += (
					iter->pageHeader->elementByteAmount *
					advanceElementAmount);

				iter->elementIndex += advanceElementAmount;
				return TRUE;
			}
		}
	}

	size_t TotalElementAmountOfPageLinkedList(PageLinkedList *linkedList) {
		PageLinkedListIter iter = InitPageLinkedListIter(linkedList);
		size_t result = 0;
		while(NextPageInLinkedListIter(&iter, 1)) {
			result += iter.pageHeader->elementAmount;
		}

		return result;
	}

	uint8_t *NthElementInPageLinkedList(
		PageLinkedList *linkedList, size_t index) {
		PageLinkedListIter iter = InitPageLinkedListIter(linkedList);
		NextElementInLinkedList(&iter, index);
		return iter.element;
	}

	/*
	PageLinkedListIterator InitPageLinkedListIterator(
		PageLinkedList *linkedList) {
		return (PageLinkedListIterator) { .linkedList = linkedList };
	}

	Bool IterateNextCellArrayInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount) {
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
			i++) {
			iterator->currentCellArray = (
				iterator->currentCellArray->nextCellArray);
		}

		if(iterator->currentCellArray == NULL) { return FALSE; }
		return TRUE;
	}

	Bool IterateNextCellInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount) {
		if(increaseAmount == 0) { return FALSE; }
		if(iterator->currentCell == NULL) {
			if(
				iterator->currentCellArray == NULL &&
				!IterateNextCellArrayInPageLinkedList(iterator, 1)
			) { return FALSE; }

			iterator->currentCell = PageCellArrayFirstCell(
				iterator->currentCellArray);

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
			iterator->currentCellArray->cellAmount - 2) {

			// NOTE: This is a while loop that goes through the cells one by
			// one since assumptions about each individual page cell amount
			// can't be made without creating bugs.
			while(
				iterator->currentCellIndex >
				iterator->currentCellArray->cellAmount - 2) {
				iterator->currentCellIndex -= (
					iterator->currentCellArray->cellAmount - 1);

				if(!IterateNextCellArrayInPageLinkedList(iterator, 1)) {
					return FALSE;
				}
			}

			iterator->currentCell = PageCellArrayFirstCell(
				iterator->currentCellArray);

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
			iterator->linkedList->defaultBytesPerElement *
			iterator->currentElementIndex];

		return TRUE;
	}

	PageCell PageCellFromPageLinkedList(PageLinkedList *linkedList) {
		size_t totalSize = GetTotalSizeForPageLinkedList(linkedList);
		PageCell result = {
			.amount = totalSize, .cap = totalSize,

			// NOTE: Since converting a PageLinkedList to a PageCell
			// means losing information about individual bytesPerElement for
			// individual PageCells, the defaultBytesPerElement is used.
			.bytesPerElement = linkedList->defaultBytesPerElement,
			.buffer = PAGE_LINKED_LIST_INIT_PAGE(totalSize)
		};

		size_t currentIndex = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList);

		while(IterateNextCellInPageLinkedList(&iterator, 1)) {
			size_t byteAmount = (
				linkedList->defaultBytesPerElement *
				iterator.currentCell->amount);

			memcpy(
				&result.buffer[currentIndex],
				&iterator.currentCell->buffer[0],
				byteAmount);

			currentIndex += byteAmount;
		}

		return result;
	}

	// TODO: This function will count the total amount of elements, which for
	// varying bytesPerElement, will be an issue. Should have
	// GetTotalElementAmountForPageLinkedList() and
	// GetTotalByteAmountForPageLinkedList().
	size_t GetTotalSizeForPageLinkedList(PageLinkedList *linkedList) {
		size_t result = 0;
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList);

		while(IterateNextCellInPageLinkedList(&iterator, 1)) {
			result += iterator.currentCell->amount;
		}

		return result;
	}

	uint8_t *GetViaIndexForPageLinkedList(
		PageLinkedList *linkedList, size_t index) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList);

		// NOTE: Since PageLinkedListIterator can be initalized with a yet
		// to be initalized PageLinkedList. Calling PageLinkedListNext...()
		// will initalize its corresponding iteration index and element/cell/
		// cell array and decrease the amount to iterate by 1.
		if(!IterateNextInPageLinkedList(&iterator, index + 1)) { return NULL; }
		return iterator.currentElement;
	}
	*/

	void CopySliceRefToRef(SliceRef srcRef, SliceRef destRef) {
		if(srcRef.byteAmount != destRef.byteAmount) { return; }

		size_t nonzeroByteAmount = srcRef.byteAmount - srcRef.zeroByteAmount;
		for(int i = 0; i < nonzeroByteAmount; i++) {
			destRef.buffer[i] = srcRef.buffer[i];
		}

		for(int i = nonzeroByteAmount - 1; i < srcRef.byteAmount; i++) {
			destRef.buffer[i] = 0;
		}

		destRef.zeroByteAmount = 1;
	}

	/*
	static Slice *InitSliceFromData(
		uint8_t *sliceAsData, size_t byteAmount, size_t prevByteAmount) {
		if(sliceAsData == NULL) { return NULL; }

		Slice *slice = (Slice *)(sliceAsData);
		*slice = (Slice) {
			.byteAmount = byteAmount, .prevByteAmount = prevByteAmount
		};

		return slice;
	}

	static Slice *InitSliceFromDataForPageCell(
		PageCell *cell, uint8_t *sliceAsData, size_t byteAmount) {
		return InitSliceFromData(
			sliceAsData, byteAmount, cell->amount - cell->prevAmount);
	}
	*/

	Bool AreSliceRefsEqual(SliceRef a, SliceRef b) {
		if(
			a.byteAmount != b.byteAmount ||
			a.zeroByteAmount != b.zeroByteAmount
		) { return FALSE; }

		for(int i = 0; i < a.byteAmount - sizeof(Slice); i++) {
			if(a.buffer[i] != b.buffer[i]) { return FALSE; }
		}

		return TRUE;
	}

	/*
	SliceRef SliceIntoRef(Slice *element) {
		return (SliceRef) {
			.byteAmount = element->byteAmount,
			.buffer = &element->buffer[0]
		};
	}
	*/

	SliceRef SliceRefFromCString(uint8_t *string) {
		int stringCharAmount = 0;
		for(; string[stringCharAmount] != 0; stringCharAmount++);
		return (SliceRef) {
			.byteAmount = ByteAmountForStringSlice(stringCharAmount),

			// NOTE: This is for the null terminator.
			.zeroByteAmount = 1,
			.buffer = &string[0]
		};
	}


	/*
	// For PageLinkedList.
	Slice *AddSliceToPageLinkedList(
		PageLinkedList *linkedList, size_t bufferByteAmount) {
		if(linkedList->defaultBytesPerElement != sizeof(uint8_t)) {
			return NULL;
		}

		size_t sliceByteAmount = ByteAmountForSlice(bufferByteAmount);
		uint8_t *sliceAsData = AddToPageLinkedList(
			linkedList, sliceByteAmount);

		return InitSliceFromDataForPageCell(
			PageCellArrayLastCell(linkedList->lastCellArray),
			sliceAsData, sliceByteAmount);
	}

	// For PageLinkedListIterator.
	Bool IterateNextSliceInPageLinkedList(
		PageLinkedListIterator *iterator, size_t increaseAmount) {
		if(iterator->currentElement == NULL) {
			if(!IterateNextInPageLinkedList(iterator, 1)) { return FALSE; }
			increaseAmount--;
		}

		if(iterator->currentCell->bytesPerElement != sizeof(uint8_t)) {

			// TODO?: To return false? Or to iterate next page? That is the
			// question. Whether it is nobler in the mind to suffer the slings
			// and arrows of outrageous API. Or to take arms against a sea
			// of troubllles and by opposing, end them. To die. To sleep,
			// no more. - Billy Madison.
			return FALSE;
		}

		for(int i = 0; i < increaseAmount; i++) {
			Slice *element = (Slice *)(iterator->currentElement);
			if(!IterateNextInPageLinkedList(iterator, element->byteAmount)) {
				return FALSE;
			}
		}

		return TRUE;
	}

	Slice *GetSliceFromIndexForPageLinkedList(
		PageLinkedList *linkedList, size_t index) {
		PageLinkedListIterator iterator = InitPageLinkedListIterator(
			linkedList);

		if(!IterateNextSliceInPageLinkedList(&iterator, index + 1)) {
			return NULL;
		}

		return (Slice *)(iterator.currentElement);
	}

	// For PageCell.
	Slice *AddSliceToPageCell(
		PageCell *cell, size_t bufferByteAmount) {
		if(cell->bytesPerElement != sizeof(uint8_t)) { return NULL; }
		size_t sliceByteAmount = ByteAmountForSlice(bufferByteAmount);
		uint8_t *sliceAsData = AddToPageCell(cell, sliceByteAmount);
		return InitSliceFromDataForPageCell(
			cell, sliceAsData, sliceByteAmount);
	}

	SliceRef SliceRefFromPageCell(PageCell *cell) {
		return (SliceRef) {
			.byteAmount = cell->amount * cell->bytesPerElement,
			.buffer = &cell->buffer[0]
		};
	}


	SliceBuilder InitSliceBuilderFromPageCell(PageCell *cell) {
		if(cell->bytesPerElement != sizeof(uint8_t)) {
			return (SliceBuilder) { 0 };
		}

		return (SliceBuilder) { .cell = cell };
	}

	SliceBuilder InitSliceBuilder(PageLinkedList *linkedList) {
		return InitSliceBuilderFromPageCell(
			PageCellArrayLastCell(linkedList->lastCellArray));
	}

	uint8_t *BuildUponSliceBuilder(
		SliceBuilder *builder, size_t byteAmount, uint8_t *src) {
		if(builder->slice == NULL) {
			builder->slice = AddSliceToPageCell(builder->cell, 0);
			if(builder->slice == NULL) { return NULL; }
		}

		// NOTE: Placing this after the inital slice init check is intentional.
		// This means that users can pre-emptively allocate the first bit of a
		// slice without having to add anything.
		if(byteAmount == 0) { return NULL; }

		uint8_t *result = AddToPageCell(builder->cell, byteAmount);
		if(result == NULL) { return NULL; }

		builder->slice->byteAmount += byteAmount;
		if(src != NULL) { memcpy(&result[0], &src[0], byteAmount); }
		return result;
	}

	uint8_t *BuildUponSliceBuilderWithRef(
		SliceBuilder *builder, SliceRef ref) {
		return BuildUponSliceBuilder(builder, ref.byteAmount, &ref.buffer[0]);
	}
	*/
#endif

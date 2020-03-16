
#include "List.h"


void kInitializeList(LIST *pstList)
{
	pstList->iItemCount = 0;
	pstList->pvHeader = NULL;
	pstList->pvTail = NULL;
}

int kGetListCount(const LIST *pstList)
{
	return pstList->iItemCount;
}

void kAddListToTail(LIST *pstList, void *pvItem)
{
	LISTLINK *pstLink;

	// pvItem should be placed in the tail of the LIST
	pstLink = (LISTLINK*)pvItem;
	pstLink->pvNext = NULL;

	// List is empty
	if (pstList->pvHeader == NULL)
	{
		pstList->pvHeader = pvItem;
		pstList->pvTail = pvItem;
		pstList->iItemCount = 1;
		return;
	}

	// List is not empty
	// Add pvItem to the end of the LIST
	pstLink = (LISTLINK*)pstList->pvTail;
	pstLink->pvNext = pvItem;

	pstList->pvTail = pvItem;
	pstList->iItemCount++;
}

void kAddListToHeader(LIST *pstList, void *pvItem)
{
	LISTLINK *pstLink;

	// pvItem should place at the head of the LIST
	pstLink = (LISTLINK*)pvItem;
	pstLink->pvNext = pstList->pvHeader;

	// List is empty
	if (pstList->pvHeader == NULL)
	{
		pstList->pvHeader = pvItem;
		pstList->pvTail = pvItem;
		pstList->iItemCount = 1;
		return;
	}

	pstList->pvHeader = pvItem;
	pstList->iItemCount++;
}

void* kRemoveList(LIST *pstList, QWORD qwID)
{
	LISTLINK *pstLink;
	LISTLINK *pstPreviousLink;

	pstPreviousLink = (LISTLINK*)pstList->pvHeader;
	for (pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext)
	{
		if (pstLink->qwID == qwID)
		{
			if (pstLink == pstList->pvHeader && pstLink == pstList->pvTail)
			{ // iItemCount == 1
				pstList->pvHeader = NULL;
				pstList->pvTail = NULL;
			}
			else if (pstLink == pstList->pvHeader)
			{ // Target link is the first element of the LIST
				pstList->pvHeader = pstLink->pvNext;
			}
			else if (pstLink == pstList->pvTail)
			{ // Target link is the last element of the LIST
				pstList->pvTail = pstPreviousLink;
			}
			else
			{
				pstPreviousLink->pvNext = pstLink->pvNext;
			}

			pstList->iItemCount--;
			return pstLink;
		}

		pstPreviousLink = pstLink;
	}

	return NULL;
}

void* kRemoveListFromHeader(LIST *pstList)
{
	LISTLINK *pstLink;

	// List is empty
	if (pstList->iItemCount == 0)
		return NULL;

	// Leave the rest of the work to the kRemoveList()
	pstLink = (LISTLINK*)pstList->pvHeader;
	return kRemoveList(pstList, pstLink->qwID);
}

void* kRemoveListFromTail(LIST *pstList)
{
	LISTLINK *pstLink;

	// List is empty
	if (pstList->iItemCount == 0)
		return NULL;

	// Leave the rest of the work to the kRemoveList()
	pstLink = (LISTLINK*)pstList->pvTail;
	return kRemoveList(pstList, pstLink->qwID);
}

void* kFindList(const LIST *pstList, QWORD qwID)
{
	LISTLINK *pstLink;

	for (pstLink = (LISTLINK*)pstList->pvHeader; pstLink; pstLink = pstLink->pvNext)
	{
		if (pstLink->qwID == qwID)
			return pstLink;
	}

	return NULL;
}

void* kGetHeaderFromList(const LIST *pstList)
{
	return pstList->pvHeader;
}

void* kGetTailFromList(const LIST *pstList)
{
	return pstList->pvTail;
}

void* kGetNextFromList(const LIST *pstList, void *pstCurrent)
{
	return ((LISTLINK*)pstCurrent)->pvNext;
}

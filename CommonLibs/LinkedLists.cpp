/*
* Copyright 2008 Free Software Foundation, Inc.
*
* This software is distributed under multiple licenses; see the COPYING file in the main directory for licensing information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/




#include "LinkedLists.h"




void PointerFIFO::put(void* val)
{
	ListNode *node = allocate();
	node->data(val);
	node->next(NULL);
	if (mTail!=NULL) mTail->next(node);
	mTail=node;
	if (mHead==NULL) mHead=node;
	mSize++;
}

/** Take an item from the FIFO. */
void* PointerFIFO::get()
{
	// empty list?
	if (mHead==NULL) return NULL;
	// normal case
	ListNode* next = mHead->next();
	void* retVal = mHead->data();
	release(mHead);
	mHead = next;
	if (next==NULL) mTail=NULL;
	mSize--;
	return retVal;
}



ListNode *PointerFIFO::allocate()
{
	if (mFreeList==NULL) return new ListNode;
	ListNode* retVal = mFreeList;
	mFreeList = mFreeList->next();
	return retVal;
}

void PointerFIFO::release(ListNode* wNode)
{
	wNode->next(mFreeList);
	mFreeList = wNode;
}




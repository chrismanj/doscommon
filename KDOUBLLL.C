/******************************************************************************\
				Doubly Linked List Routines
						By John S. Chrisman

 12-09-97: v1.0 created
 10-18-98: Added key member variable to doubly linked lists
		 Added DoublyLinkedListGetItemWKeyOf function
\******************************************************************************/

#include <stdlib.h>
#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\kdoublell.h>
#include <c:\progproj\c\common\include\mem.h>

/******************************************************************************\

 Routine: CreateDoublyLinkedList

		Pass: Nothing

 Returns: A pointer to the linked list structure.

\******************************************************************************/

dlinkedlist *DLLCreate (void)
{
	dlinkedlist *list = malloc(sizeof(dlinkedlist));
	if (list != NULL)
	{
		list->firstElement = NULL;
		list->lastElement = NULL;
		list->lastAccessedElement = NULL;
		list->lastAccessedElementNumber = 0;
		list->numElements = 0;
	}
	return list;
}

/******************************************************************************\

 Routine: DLLAddItem

		Pass: A pointer to the linked list structure.
		A void pointer to the item you want added to the list.
		A DWORD which will be used as the key for this item.

 Returns: 0 - Could not allocate memory to add item
					1 - Item added successfully

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

int DLLAddItem (dlinkedlist *list, void *newItem, DWORD key)
{
	dllelem *newElement;

	newElement = malloc(sizeof(dllelem));
	if (newElement != NULL)
	{
		if (list->lastElement != NULL)
		{
			newElement->prevElement = list->lastElement;
			list->lastElement->nextElement = newElement;
		}
		else
		{
			newElement->prevElement = NULL;
			list->firstElement = newElement;
		}
		newElement->nextElement = NULL;
		newElement->item = newItem;
		newElement->key = key;
		list->lastElement = newElement;
		list->numElements++;
		return 1;
	}
	return 0;
}

/******************************************************************************\

 Routine: DLLGetFirstItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the first item in the list.

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

void *DLLGetFirstItem (dlinkedlist *list)
{
	if (list->firstElement != NULL)
	{
		list->lastAccessedElementNumber = 1;
		list->lastAccessedElement = list->firstElement;
		return list->firstElement->item;
	}
	else
		return NULL;
}

/******************************************************************************\

 Routine: DLLGetLastItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the last item in the list.

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

void *DLLGetLastItem (dlinkedlist *list)
{
	if (list->lastElement != NULL)
	{
		list->lastAccessedElementNumber = list->numElements;
		list->lastAccessedElement = list->lastElement;
		return list->lastElement->item;
	}
	else
		return NULL;
}

/******************************************************************************\

 Routine: DLLGetNextItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the next item in the list or NULL if the end of
		list has been reached.

	 Notes: This routine checks the last element accessed and returns the next
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then the
		first item in the list is returned.

\******************************************************************************/

void *DLLGetNextItem (dlinkedlist *list)
{
	dllelem *element = list->lastAccessedElement;

	if (element != NULL)
		element = element->nextElement;
	else
		element = list->firstElement;
	if (element != NULL)
	{
		list->lastAccessedElement = element;
		list->lastAccessedElementNumber++;
		return element->item;
	}
	return NULL;
}

/******************************************************************************\

 Routine: DLLGetPrevItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the previous item in the list or NULL if the
		beginning of list has been reached.

	 Notes: This routine checks the last element accessed and returns the previous
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then
		the last item in the list is returned.

\******************************************************************************/

void *DLLGetPrevItem (dlinkedlist *list)
{
	dllelem *element = list->lastAccessedElement;

	if (element != NULL)
		element = element->prevElement;
	else
		element = list->lastElement;
	if (element != NULL)
	{
		list->lastAccessedElement = element;
		list->lastAccessedElementNumber--;
		return element->item;
	}
	return NULL;
}

/******************************************************************************\

 Routine: CGetNextItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the next item in the list. When the end of the list
		is reached it returns the first item in the list.

	 Notes: This routine checks the last element accessed and returns the next
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then the
		first item in the list is returned.

\******************************************************************************/

void *CGetNextItem (dlinkedlist *list)
{
	void *item = DLLGetNextItem (list);
	if (item == NULL)
		item = DLLGetFirstItem (list);
	return item;
}

/******************************************************************************\

 Routine: CGetPrevItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the next item in the list. When the beginning of the
		is reached it returns the last item in the list.

	 Notes: This routine checks the last element accessed and returns the previous
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then
		the last item in the list is returned.

\******************************************************************************/

void *CGetPrevItem (dlinkedlist *list)
{
	void *item = DLLGetPrevItem (list);
	if (item == NULL)
		item = DLLGetLastItem (list);
	return item;
}

/******************************************************************************\

 Routine: DLLGetItemByKey

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the item in the list with the specified key.

	 Notes:

\******************************************************************************/

void *DLLGetItemByKey (dlinkedlist *list, DWORD key)
{
	BOOLN done = FALSE;
	void *item = DLLGetFirstItem(list);
	while (done == FALSE)
		if (item == NULL)
			done = TRUE;
		else
			if(list->lastAccessedElement->key != key)
				item = DLLGetNextItem (list);
			else
				done = TRUE;
	return item;
}

/******************************************************************************\

 Routine: DLLResetKey

		Pass: A pointer to the linked list structure.
		A key

 Returns:

	 Notes:

\******************************************************************************/

void DLLResetKey (dlinkedlist *list, DWORD key)
{
	list->lastAccessedElement->key = key;
}

/******************************************************************************\

 Routine: DLLGetCurrentPosition

		Pass: A pointer to the linked list structure.

 Returns: A pointer to the last element accessed.

\******************************************************************************/

BOOLN DLLGetCurrentPosition (dlinkedlist *list, dll_position *position)
{
	if (list->lastAccessedElement != NULL)
	{
		position->element = list->lastAccessedElement;
		position->elementNumber = list->lastAccessedElementNumber;
		return TRUE;
	}
	else
		return FALSE;
}

/******************************************************************************\

 Routine: DLLSetCurrentPosition

		Pass: A pointer to the linked list structure
		A pointer to an s_dll_element structure.

 Returns: A void pointer to the item at the position passed.

\******************************************************************************/

void *DLLSetCurrentPosition (dlinkedlist *list, dll_position *elementPos)
{
	list->lastAccessedElement = elementPos->element;
	list->lastAccessedElementNumber = elementPos->elementNumber;
	return elementPos->element->item;
}

/******************************************************************************\

 Routine: DLLGetElementBefore

		Pass: A pointer to the linked list structure.
		A pointer to the element position that you want to get the element
			before in the list.
		A pointer to an element position structure you want the new element
			position placed into.

 Returns: Nothing

\******************************************************************************/

void DLLGetElementBefore (dll_position *position, dll_position *dest)
{
	dest->element = position->element->prevElement;
	dest->elementNumber = position->elementNumber - 1;
}

/******************************************************************************\

 Routine: DLLGetElementAfter

		Pass: A pointer to the linked list structure.
		A pointer to the element position that you want to get the element
			after in the list.
		A pointer to an element position structure you want the new element
			position placed into.

 Returns: Nothing

\******************************************************************************/

void DLLGetElementAfter (dll_position *position, dll_position *dest)
{
	dest->element = position->element->nextElement;
	if (dest->element != NULL)
		dest->elementNumber = position->elementNumber + 1;
	else
		dest->elementNumber = 0;
}

/******************************************************************************\

 Routine: DLLGetItemNum

		Pass: A pointer to the linked list structure.

 Returns: The order in the list of the last item accessed, with 1 being the
		first element. If no items have been access it returns 0.

\******************************************************************************/

DWORD DLLGetItemNum (dlinkedlist *list)
{
	return list->lastAccessedElementNumber;
}

/******************************************************************************\

 Routine: DLLGetNoOfItems

		Pass: A pointer to the linked list structure.

 Returns: The number of items in the list

	 Notes:

\******************************************************************************/

DWORD DLLGetNoOfItems (dlinkedlist *list)
{
	return list->numElements;
}

/******************************************************************************\

 Routine: DLLDeleteElement

		Pass: A pointer to the linked list structure.

 Returns: Nothing

	 Notes: Deletes the last accessed element

\******************************************************************************/

void *DLLDeleteElement (dlinkedlist *list)
{
	void *item = NULL;

	if (list->lastAccessedElement != NULL)
	{
		dllelem *element = list->lastAccessedElement;
		dllelem *prevElement = element->prevElement;
		dllelem *nextElement = element->nextElement;
		item = element->item;

		prevElement->nextElement = element->nextElement;
		nextElement->prevElement = element->prevElement;
		free(element);
		list->numElements--;
		if (list->lastAccessedElementNumber == 1)
			list->firstElement = nextElement;
		if(list->lastAccessedElementNumber > list->numElements)
		{
			list->lastAccessedElementNumber--;
			list->lastAccessedElement = prevElement;
			list->lastElement = prevElement;
		}
		else
			list->lastAccessedElement = nextElement;
		return item;
	}
	return NULL;
}

/******************************************************************************\

 Routine: DLLDestroy

		Pass: A pointer to the linked list structure.

 Returns: Nothing

	 Notes: This routine frees all memory allocated for the list.

\******************************************************************************/

void DLLDestroy (dlinkedlist *list)
{
	dllelem *curElement;
	dllelem *nextElement = list->firstElement;

	free (list);
	while (nextElement != NULL)
	{
		curElement = nextElement;
		nextElement = curElement->nextElement;
		free (curElement);
	}
}

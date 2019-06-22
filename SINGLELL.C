/* This still needs quite a bit of work */

/******************************************************************************\
				Singly Linked List Routines
						By John S. Chrisman
\******************************************************************************/

#include <stdlib.h>
#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\singlell.h>
#include <c:\progproj\c\common\include\mem.h>

/******************************************************************************\

 Routine: SLLCreate

		Pass: Nothing

 Returns: A pointer to the linked list structure.

\******************************************************************************/

slinkedlist *SLLCreate (void)
{
	slinkedlist *list = malloc(sizeof(slinkedlist));
	if (list != NULL)
	{
		list->firstElement =
		list->lastElement =
		list->lastAccessedElement =
		list->prevAccessedElement = NULL;
		list->lastAccessedElementNumber = 0;
		list->numElements = 0;
	}
	return list;
}

/******************************************************************************\

 Routine: SLLAddItem

		Pass: A pointer to the linked list structure.
		A void pointer to the item you want added to the list.

 Returns: 0 - Could not allocate memory to add item
					1 - Item added successfully

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

int SLLAddItem (slinkedlist *list, void *newItem)
{
	SLLelem *newElement;

	newElement = malloc(sizeof(SLLelem));
	if (newElement != NULL)
	{
		if (list->lastElement != NULL)
			list->lastElement->nextElement = newElement;
    else
    	list->firstElement = newElement;
    newElement->nextElement = NULL;
		newElement->item = newItem;
		list->lastElement = newElement;
		list->numElements++;
		return 1;
	}
	return 0;
}

/******************************************************************************\

 Routine: SLLGetFirstItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the first item in the list.

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

void *SLLGetFirstItem (slinkedlist *list)
{
	if (list->firstElement != NULL)
	{
		list->lastAccessedElementNumber = 1;
		list->lastAccessedElement = list->firstElement;
    list->prevAccessedElement = NULL;
		return list->firstElement->item;
	}
	else
		return NULL;
}

/******************************************************************************\

 Routine: SLLGetLastItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the last item in the list.

	 Notes: This routine updates the last element accessed.

\******************************************************************************/

void *SLLGetLastItem (slinkedlist *list)
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

 Routine: SLLGetNextItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the next item in the list or NULL if the end of
		list has been reached.

	 Notes: This routine checks the last element accessed and returns the next
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then the
		first item in the list is returned.

\******************************************************************************/

void *SLLGetNextItem (slinkedlist *list)
{
	SLLelem *element = list->lastAccessedElement;

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

 Routine: CGetNextItem

		Pass: A pointer to the linked list structure.

 Returns: A void pointer to the next item in the list. When the end of the list
		is reached it returns the first item in the list.

	 Notes: This routine checks the last element accessed and returns the next
		item in the list. It updates the last element accessed. If this is
		the first routine called which returns an item from the list then the
		first item in the list is returned.

\******************************************************************************/

void *CGetNextItem (slinkedlist *list)
{
	void *item = SLLGetNextItem (list);
	if (item == NULL)
		item = SLLGetFirstItem (list);
	return item;
}

/******************************************************************************\

 Routine: SLLGetCurrentPosition

		Pass: A pointer to the linked list structure.

 Returns: A pointer to the last element accessed.

\******************************************************************************/

BOOLN SLLGetCurrentPosition (slinkedlist *list, SLL_position *position)
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

 Routine: SLLSetCurrentPosition

		Pass: A pointer to the linked list structure
		A pointer to an s_SLL_element structure.

 Returns: A void pointer to the item at the position passed.

\******************************************************************************/

void *SLLSetCurrentPosition (slinkedlist *list, SLL_position *elementPos)
{
	list->lastAccessedElement = elementPos->element;
	list->lastAccessedElementNumber = elementPos->elementNumber;
	return elementPos->element->item;
}

/******************************************************************************\

 Routine: SLLGetElementAfter

		Pass: A pointer to the linked list structure.
		A pointer to the element position that you want to get the element
			after in the list.
		A pointer to an element position structure you want the new element
			position placed into.

 Returns: Nothing

\******************************************************************************/

void SLLGetElementAfter (SLL_position *position, SLL_position *dest)
{
	dest->element = position->element->nextElement;
	if (dest->element != NULL)
		dest->elementNumber = position->elementNumber + 1;
	else
		dest->elementNumber = 0;
}

/******************************************************************************\

 Routine: SLLGetItemNum

		Pass: A pointer to the linked list structure.

 Returns: The order in the list of the last item accessed, with 1 being the
		first element. If no items have been access it returns 0.

\******************************************************************************/

DWORD SLLGetItemNum (slinkedlist *list)
{
	return list->lastAccessedElementNumber;
}

/******************************************************************************\

 Routine: SLLGetNoOfItems

		Pass: A pointer to the linked list structure.

 Returns: The number of items in the list

	 Notes:

\******************************************************************************/

DWORD SLLGetNoOfItems (slinkedlist *list)
{
	return list->numElements;
}

/******************************************************************************\

 Routine: SLLDeleteElement

		Pass: A pointer to the linked list structure.

 Returns: Nothing

	 Notes: Deletes the last accessed element

\******************************************************************************/

void *SLLDeleteElement (slinkedlist *list)
{
	void *item = NULL;

	if (list->lastAccessedElement != NULL)
	{
		SLLelem *element = list->lastAccessedElement;
		SLLelem *prevElement = element->prevElement;
		SLLelem *nextElement = element->nextElement;
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
		return element;
	}
	return NULL;
}

/******************************************************************************\

 Routine: SLLDestroy

		Pass: A pointer to the linked list structure.

 Returns: Nothing

	 Notes: This routine frees all memory allocated for the list.

\******************************************************************************/

void SLLDestroy (slinkedlist *list)
{
	SLLelem *curElement;
	SLLelem *nextElement = list->firstElement;

	free (list);
	while (nextElement != NULL)
	{
		curElement = nextElement;
		nextElement = curElement->nextElement;
		free (curElement);
	}
}

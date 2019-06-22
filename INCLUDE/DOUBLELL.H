#ifndef DOUBLELL_INC

#define DOUBLELL_INC 1

#include "c:\progproj\c\common\include\doubllt.h"

struct s_dll_element
{
	struct s_dll_element *nextElement;
	struct s_dll_element *prevElement;
	void *item;
};

struct s_dlinked_list
{
	struct s_dll_element *firstElement;
	struct s_dll_element *lastElement;
	struct s_dll_element *lastAccessedElement;
	DWORD lastAccessedElementNumber;
	DWORD numElements;
};

struct s_dll_position
{
	dllelem *element;
	DWORD elementNumber;
};

dlinkedlist  *DLLCreate (void);
int           DLLAddItem (dlinkedlist *, void *);
BOOLN         DLLGetCurrentPosition (dlinkedlist *, dll_position *);
void         *DLLGetFirstItem (dlinkedlist *);
void         *DLLGetLastItem (dlinkedlist *);
void         *DLLGetPrevItem (dlinkedlist *);
void         *DLLGetNextItem (dlinkedlist *);
void         *DLLCGetPrevItem (dlinkedlist *);
void         *DLLCGetNextItem (dlinkedlist *);
void         *DLLSetCurrentPosition (dlinkedlist *, dll_position *);
void         *DLLDeleteElement (dlinkedlist *);
void          DLLGetElementBefore (dll_position *, dll_position *);
void          DLLGetElementAfter (dll_position *, dll_position *);
DWORD         DLLGetNoOfItems (dlinkedlist *);
DWORD         DLLGetItemNum (dlinkedlist *list);
void          DLLDestroy (dlinkedlist *);
#endif

#define SLLelem struct s_SLL_element
struct s_SLL_element
{
  struct s_SLL_element *nextElement;
  void *item;
};

#define slinkedlist struct s_slinked_list
struct s_slinked_list
{
  struct s_SLL_element *firstElement;
  struct s_SLL_element *lastElement;
  struct s_SLL_element *lastAccessedElement;
  struct s_SLL_element *prevAccessedElement;
  DWORD lastAccessedElementNumber;
  DWORD numElements;
};

#define SLL_position struct s_SLL_position
struct s_SLL_position
{
  SLLelem *element;
  DWORD elementNumber;
};

slinkedlist *SLLCreate(void);
int SLLAddItem(slinkedlist *, void *);
BOOLN SLLGetCurrentPosition(slinkedlist *, SLL_position *);
void *SLLGetFirstItem(slinkedlist *);
void *SLLGetLastItem(slinkedlist *);
void *SLLGetNextItem(slinkedlist *);
void *SLLCGetNextItem(slinkedlist *);
void *SLLSetCurrentPosition(slinkedlist *, SLL_position *);
void *SLLDeleteElement(slinkedlist *);
void SLLGetElementAfter(SLL_position *, SLL_position *);
DWORD SLLGetNoOfItems(slinkedlist *);
DWORD SLLGetItemNum(slinkedlist *list);
void SLLDestroy(slinkedlist *);

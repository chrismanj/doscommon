#define SBQueue struct bqueue
struct bqueue
{
  BYTE *buffer_start;
  BYTE *buffer_end;
  BYTE *head;
  BYTE *tail;
  WORD size;
  WORD count;
};

#define SUCCESS 0
#define QUEUE_FULL 1

SBQueue  *InitByteQueue (WORD);
void      DestroyByteQueue (SBQueue *queue);
int       Enqueue (SBQueue *, BYTE);
BYTE      Dequeue (SBQueue *);
void      FlushQueue (SBQueue *queue);

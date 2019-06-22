#include <stdlib.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\bqueue.h"
#include "..\common\include\keybrd.h"
#include "..\common\include\mem.h"

/******************************************************************************\

   Routine: InitByteQueue
  
  Function: Allocate memory for a new BYTE queue and set all member variables of
            the SBQqueue struct
  
      Pass: Size of the BYTE queue you want to create
    
   Return: Pointer to the new BYTE queue or NULL if new BYTE queue can not be
           created
    
\******************************************************************************/

SBQueue *InitByteQueue(WORD size)
{
  SBQueue *newQueue = malloc(sizeof(SBQueue) + (size_t)size);
  if (newQueue != NULL)
  {
    newQueue->buffer_start = newQueue->head = newQueue->tail =
        (char *)newQueue + sizeof(SBQueue);
    newQueue->buffer_end = newQueue->buffer_start + size;
    newQueue->size = size;
    newQueue->count = 0;
  }
  return newQueue;
}

/******************************************************************************\

   Routine: Enqueue
  
  Function: Insert a BYTE into a BYTE queue
  
      Pass: Pointer to the BYTE queue to insert the BYTE into, the BYTE to insert
    
    Return: SUCCESS, QUEUE_FULL
    
\******************************************************************************/

int Enqueue(SBQueue *queue, BYTE ch)
{
  if (queue->count >= queue->size)
    return QUEUE_FULL;

  *queue->tail = ch;
  if (++queue->tail == queue->buffer_end)
    queue->tail = queue->buffer_start;
  ++queue->count;
  return SUCCESS;
}

/******************************************************************************\

   Routine: Dequeue
  
  Function: Remove and return a BYTE from a BYTE queue
  
      Pass: A pointer to the BYTE queue the BYTE is to be removed from
    
    Return: 0 = Queue empty, 0 - 255 is the value of the BYTE retrieved
    
\******************************************************************************/

BYTE Dequeue(SBQueue *queue)
{
  if (queue->count == 0)
    return 0;

  if (queue->head == queue->buffer_end)
    queue->head = queue->buffer_start;
  queue->count--;
  return *queue->head++;
}

/******************************************************************************\

   Routine: FlushQueue
  
  Function: Empty out a BYTE queue
  
      Pass: A pointer to the BYTE queue to be flushed
    
    Return: Nothing
    
\******************************************************************************/

void FlushQueue(SBQueue *queue)
{
  queue->count = 0;
  queue->head = queue->tail;
}

/******************************************************************************\

   Routine: DestroyByteQueue
  
  Function: Deallocates memory allocated for a BYTE queue
  
      Pass: A pointer to the BYTE queue to be destroyed
    
    Return: Nothing
    
\******************************************************************************/

void DestroyByteQueue(SBQueue *queue)
{
  free(queue);
}

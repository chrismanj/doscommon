#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"

#ifdef DEBUG

#define mem_ll_elem struct s_mem_ll_elem
struct s_mem_ll_elem
{
  mem_ll_elem *next_elem;
  void *memory;
  char *filename;
  WORD line_no;
};

static mem_ll_elem *first_elem = NULL;
static mem_ll_elem *last_elem = NULL;
static DWORD num_malloced = 0;
static DWORD num_freed = 0;

void *mem_malloc(size_t size, char *filename, WORD line_no)
{
  mem_ll_elem *new_elem = malloc(sizeof(mem_ll_elem));
  void *allocated_mem = malloc(size);
  new_elem->next_elem = NULL;
  new_elem->memory = allocated_mem;
  new_elem->filename = filename;
  new_elem->line_no = line_no;
  if (first_elem == NULL)
    first_elem = new_elem;
  else
    last_elem->next_elem = new_elem;
  last_elem = new_elem;
  num_malloced++;
  return allocated_mem;
}

void mem_free(void **ptr, char *filename, WORD line_no)
{
  mem_ll_elem *prev_elem = NULL;
  mem_ll_elem *cur_elem = first_elem;
  mem_ll_elem *matched_elem = NULL;
  void *block = *ptr;
  if (block == NULL)
  {
    printf("***Attempted to free memory already freed from file %s line %u***\n", filename, line_no);
    while (kbhit() == 0)
      ;
    getch();
    return;
  }

  while (cur_elem != NULL && matched_elem == NULL)
  {
    if (cur_elem->memory == block)
      matched_elem = cur_elem;
    else
    {
      prev_elem = cur_elem;
      cur_elem = cur_elem->next_elem;
    }
  }
  if (matched_elem == NULL)
  {
    printf("***Attempted to free unallocated memory from file %s line %u***\n", filename, line_no);
    while (kbhit() == 0)
      ;
    getch();
  }
  else
  {
    if (matched_elem == last_elem)
      last_elem = prev_elem;
    if (prev_elem != NULL)
      prev_elem->next_elem = matched_elem->next_elem;
    else
      first_elem = matched_elem->next_elem;
    num_freed++;
    free(matched_elem);
    free(block);
    *ptr = NULL;
  }
}

void mem_report(void)
{
  mem_ll_elem *cur_elem = first_elem;

  while (cur_elem != NULL)
  {
    printf("***%s %u Allocated***\n", cur_elem->filename, cur_elem->line_no);
    cur_elem = cur_elem->next_elem;
  }
  printf("Malloc called %lu times, free called %lu times.\n", num_malloced, num_freed);
}
#endif

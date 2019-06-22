#ifndef MEM_INC

#define MEM_INC

#ifdef DEBUG
  #define malloc(s) mem_malloc(s, __FILE__, (WORD)__LINE__)
  #define free(p)   mem_free(&p, __FILE__, (WORD)__LINE__)
  void *mem_malloc (size_t, char *, int);
  void mem_free (void *, char *, int);
  void mem_report (void);
#endif
#endif

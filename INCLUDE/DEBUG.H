#ifndef DEBUG_INC

#define DEBUG_INC 1

#define DEBUG

#ifdef DEBUG
#define MEM_DEBUG 1
void OutDebugText(char *);
void DebugPause(void);
#endif

#endif

#include <stdio.h>
#include <c:\progproj\c\common\include\debug.h>

void EndProgram (void);

#ifdef DEBUG

static char debug_str[] = "          ";

#include <c:\progproj\c\common\include\types.h>

extern byte _far *screen_base;

void OutDebugText (char *text)
{
	if (screen_base == (word _far *)0xb0000000L || screen_base == (word _far *)0xb0008000L)
	{
		debug_str[0] = debug_str[1];
		debug_str[1] = debug_str[2];
		debug_str[2] = debug_str[3];
		debug_str[3] = debug_str[4];
		debug_str[4] = debug_str[5];
		debug_str[5] = debug_str[6];
		debug_str[6] = debug_str[7];
		debug_str[7] = debug_str[8];
		debug_str[8] = *text++;
		debug_str[9] = *text;
		
		*(screen_base + 0x00000f8cL) = debug_str[0];
		*(screen_base + 0x00000f8eL) = debug_str[1];
		*(screen_base + 0x00000f90L) = debug_str[2];
		*(screen_base + 0x00000f92L) = debug_str[3];
		*(screen_base + 0x00000f94L) = debug_str[4];
		*(screen_base + 0x00000f96L) = debug_str[5];
		*(screen_base + 0x00000f98L) = debug_str[6];
		*(screen_base + 0x00000f9aL) = debug_str[7];
		*(screen_base + 0x00000f9cL) = debug_str[8];
		*(screen_base + 0x00000f9eL) = debug_str[9];
	}
	else
		printf ("Screen base not initialized\n");
}
#endif

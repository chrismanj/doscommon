#ifndef MESSAGE_INC
#define MESSAGE_INC 1

#include "c:\progproj\c\common\include\wndt.h"
#include "c:\progproj\c\common\include\messaget.h"

struct sMSG
{
	HWND wnd;
	WORD message;
	DWORD param1;
	DWORD param2;
};
void DispatchMessage (HMSG);
void InitMessageSystem (void);
void KillMessageSystem (void);
void PostMessage (HWND, WORD, DWORD, DWORD);
HMSG RetrieveMessage (void);
int GetMessageCount (void);
HMSG DirtyWindows (void);
#endif

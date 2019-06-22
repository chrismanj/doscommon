#include <stdlib.h>
#include <stdio.h>
#include <conio.h>	/* For kbhit() */
#include "c:\progproj\c\common\include\types.h"
#include "c:\progproj\c\common\include\debug.h"
#include "c:\progproj\c\common\include\rect.h"
#include "c:\progproj\c\common\include\doublell.h"
#include "c:\progproj\c\common\include\wnd.h"
#include "c:\progproj\c\common\include\message.h"
#include "c:\progproj\c\common\include\mouse.h"

#define MAXMESSAGES 100

extern HWND gMasterWin;

MSG messages [MAXMESSAGES];
int curMessageHead, curMessageTail, curMessageCount;
#ifdef DEBUG
  void LogMsg (HMSG);
	FILE *messageLog;
#endif

void InitMessageSystem (void)
{
	curMessageHead = 0;
	curMessageTail = 0;
	curMessageCount = 0;
	#ifdef DEBUG
		messageLog = fopen ("C:\\progproj\\c\\os\\mess.log", "w");
		if (messageLog == NULL)
		{
			while (fflush(stdin));
			printf ("Unable to open message log.");
			while (kbhit() == 0);
		}

	#endif
}

void KillMessageSystem (void)
{
	#ifdef DEBUG
		fclose (messageLog);
	#endif
}

void PostMessage (HWND wnd, WORD message, DWORD param1, DWORD param2)
{
	if (curMessageCount == MAXMESSAGES)
		return;
  messages[curMessageTail].wnd = wnd;
	messages[curMessageTail].message = message;
  messages[curMessageTail].param1 = param1;
  messages[curMessageTail].param2 = param2;
	curMessageCount++;
	curMessageTail++;
	if (curMessageTail == MAXMESSAGES)
		curMessageTail = 0;
}

HWND FindDirtyWindow (HWND wnd)
{
	HWND curChildWin;
	HWND winFound = NULL;

	if (IsDirty(wnd))
  	return wnd;
	curChildWin = DLLGetFirstItem (wnd->children);
	while (curChildWin != NULL && winFound == NULL)
	{
		winFound = FindDirtyWindow (curChildWin);
		curChildWin = DLLGetNextItem (wnd->children);
	}
	return winFound;
}

HMSG DirtyWindows ()
{

  HWND wnd;

	wnd = FindDirtyWindow (gMasterWin);
  if (wnd != NULL)
	{
		PostMessage (wnd, SM_SHOWWIN, 0L, 0L);
		return RetrieveMessage();
	}
	return NULL;
}

HMSG RetrieveMessage (void)
{
	HMSG msg = &messages[curMessageHead];

	if (curMessageCount == 0)
		return DirtyWindows();
	curMessageCount--;
	curMessageHead++;
	if (curMessageHead == MAXMESSAGES)
		curMessageHead = 0;
	return msg;
}

int GetMessageCount (void)
{
	return curMessageCount;
}

void DispatchMessage (HMSG msg)
{
	int result = 0;
	HWND origWnd = msg->wnd;

	while ((msg->wnd != NULL) && (result == 0))
	{
  	if (msg->wnd->MessageHandler != NULL)
		{
			result = (*msg->wnd->MessageHandler) (msg);
			#ifdef DEBUG
				LogMsg (msg);
			#endif
		}
		if (result == 0)
			msg->wnd = msg->wnd->parent;
	}
	if (result == 0)
	{
		msg->wnd = origWnd;
		result = DefaultWindowMH (msg);
	}
}

#ifdef DEBUG
void LogMsg (HMSG msg)
{
	char *messageName;

  switch (msg->message)
	{
		case SM_LBUTTONDN:
			messageName = "SM_LBUTTONDN";
			break;

		case SM_LBUTTONUP:
			messageName = "SM_LBUTTONUP";
			break;

		case SM_RBUTTONDN:
			messageName = "SM_RBUTTONDN";
			break;

		case SM_RBUTTONUP:
			messageName = "SM_RBUTTONUP";
			break;

		case SM_MBUTTONDN:
			messageName = "SM_MBUTTONDN";
			break;

		case SM_MBUTTONUP:
			messageName = "SM_MBUTTONUP";
			break;

		case SM_LCLICK:
			messageName = "SM_LCLICK";
			break;

		case SM_RCLICK:
			messageName = "SM_RCLICK";
			break;

		case SM_MCLICK:
      messageName = "SM_MCLICK";
      break;

		case SM_LDCLICK:
			messageName = "SM_LDCLICK";
      break;

		case SM_RDCLICK:
			messageName = "SM_RDCLICK";
      break;

		case SM_MDCLICK:
			messageName = "SM_MDCLICK";
      break;

		case SM_MOUSEON:
			messageName = "SM_MOUSEON";
      break;

		case SM_MOUSEOFF:
			messageName = "SM_MOUSEOFF";
      break;

	case SM_MOUSEMOVE:
      /*messageName = "SM_MOUSEMOVE";*/
			messageName = NULL;
      break;

		case SM_CLOSE:
			messageName = "SM_CLOSE";
      break;

		case SM_MINIMIZE:
			messageName = "SM_MINIMIZE";
      break;

		case SM_TOGGLEMAX:
			messageName = "SM_TOGGLEMAX";
      break;

		case SM_BEGINDRAG:
      messageName = "SM_BEGINDRAG";
      break;

		case SM_DEACTIVATEWIN:
      messageName = "SM_DEACTIVATEWIN";
			break;

		case SM_ACTIVATEWIN:
      messageName = "SM_ACTIVATEWIN";
      break;

		case SM_SHOWWIN:
			messageName = "SM_SHOWWIN";
      break;

    case SM_MOVETOFRONT:
			messageName = "SM_MOVETOFRONT";
      break;

		default:
			messageName = "UNKNOWN MESSAGE      ";
			itoa (msg->message, &messageName[16], 10);
			break;
	}
	if (messageName != NULL)
	{
    fprintf (messageLog, "Message sent to window: %s\n", msg->wnd->title);
  	fprintf (messageLog, "Message: %s\n", messageName);
		fprintf (messageLog, "Param1: %lx\n", msg->param1);
		fprintf (messageLog, "Param2: %lx\n\n", msg->param2);
	}
}
#endif

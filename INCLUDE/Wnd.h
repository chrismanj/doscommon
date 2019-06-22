#ifndef WND_INC

#define WND_INC 1

#include "c:\progproj\c\common\include\wndt.h"
#include "c:\progproj\c\common\include\messaget.h"
#include "c:\progproj\c\common\include\rect.h"
#include "c:\progproj\c\common\include\doubllt.h"

#define MakeNotDirty(x) x->attributes &= NOTDIRTY
#define MakeDirty(x)        \
  x->attributes |= DIRTY; \
  dirtyWindows = TRUE
#define IsDirty(x) (x->attributes & DIRTY) == DIRTY

struct sWndDef
{
  HRECT sizeRect;
  char *title;
  DWORD attributes;
  DWORD attr2;
  HWND parent;
  DWORD bkColor;
  DWORD fgColor;
  DWORD frameColor;
  void *UDA;
  void (*WinDrawRoutine)(HWND);
  int (*FrameDrawRoutine)(HWND);
  int (*MessageHandler)(HMSG);
  BOOLN makeActive;
};

struct sWnd
{
  DWORD id;
  char *title;
  DWORD bkColor;
  DWORD fgColor;
  DWORD frameColor;
  RECT totalRect;
  RECT innerRect;
  DWORD attributes;
  DWORD attr2;
  HWND parent;
  dlinkedlist *children;
  void (*WinDrawRoutine)(HWND);
  int (*FrameDrawRoutine)(HWND);
  int (*MessageHandler)(HMSG);
  void *UDA;
  HWND activeChild;
};

struct sSysBUDA
{
  HWND affectedWindow;
  DWORD attributes;
};

/* SysBUDA constants */
#define BUTTONDN 0x00000001L
#define UPDNMASK 0xFFFFFFFEL

/* Window Attributes Constants */

#define NOFRAME 0x00000001L
#define FRAME 0xFFFFFFFEL
#define NOTITLE 0x00000002L
#define TITLE 0xFFFFFFFDL
#define NOCLOSE 0x00000004L
#define CLOSE 0xFFFFFFFBL
#define NOTOGGLE 0x00000008L
#define TOGGLE 0xFFFFFFF7L
#define NOMINIMIZE 0x00000010L
#define MINIMIZE 0xFFFFFFEFL
#define COPYTITLE 0x00000020L
#define NOCOPYTITLE 0xFFFFFFDFL

/* BUTTONSTYLE = FRAME, NO TITLE, NO CLOSE, NO TOGGLE,
   NO MINIMIZE, COPY TITLE */

#define BUTTONSTYLE 0x0000003EL

/* TITLESTYLE = NO FRAME, NO TITLE, NO CLOSE, NO TOGGLE,
   NO MINIMIZE, COPY TITLE */

#define TITLESTYLE 0x0000003FL

/* Window State Constants */

#define DESTROYING 0x00000000L
#define DESTROYMASK 0xFFFFFF3FL
#define MAXIMIZED 0x00000040L
#define NORMAL 0x00000080L
#define MINIMIZED 0x000000C0L
#define STATEMASK 0x000000C0L
#define DRAGGING 0x00000100L
#define NOTDRAGGING 0xFFFFFEFFL
#define DIRTY 0x00000200L
#define NOTDIRTY 0xFFFFFDFFL

/* Window Message Constants */

#define SM_LBUTTONDN 0
#define SM_LBUTTONUP 1
#define SM_RBUTTONDN 2
#define SM_RBUTTONUP 3
#define SM_MBUTTONDN 4
#define SM_MBUTTONUP 5
#define SM_LCLICK 6
#define SM_RCLICK 7
#define SM_MCLICK 8
#define SM_LDCLICK 9
#define SM_RDCLICK 10
#define SM_MDCLICK 11
#define SM_MOUSEON 12
#define SM_MOUSEOFF 13
#define SM_MOUSEMOVE 14
#define SM_CLOSE 20
#define SM_MINIMIZE 21
#define SM_TOGGLEMAX 22
#define SM_BEGINDRAG 23
#define SM_ENDDRAG 24
#define SM_ACTIVATEWIN 25
#define SM_DEACTIVATEWIN 26
#define SM_EXITOS 27
#define SM_SHOWWIN 28
#define SM_MOVETOFRONT 29

/* Other constants */

#define TITLEHEIGHT 10
#define DEFTITLECOLOR 1
#define DEFINACTTITLECOLOR 6
#define DEFWINBACKCOLOR 7
#define DEFWINFORECOLOR 0
#define DEFWINFRAMECOLOR 6
#define DEFTITLETEXTCOLOR 14
#define DEFBUTTONBACKCOLOR 7
#define DEFBUTTONFORECOLOR 0
#define SYSBUTTONMAX 5

void InitWindowSystem(void);
void KillWindowSystem(void);

HWDEF CreateWindowDef(HWDEF);
void CopyWindowDef(HWDEF, HWDEF);

void ActivateWindow(HWND);
HWND CreateWindow(HWDEF);
void DeactivateWindow(HWND);
void DestroyWindow(HWND);
void MoveWindow(HWND, int, int);
void MoveWindowToFront(HWND);
void RedrawChildren(HWND);
void ShowWindow(HWND);
HWND WindowAtPoint(POINT);

int DrawFlatFrame(HWND);
int Draw3dFrame(HWND);
int DrawInv3dFrame(HWND);
int DrawS3dFrame(HWND);
int DrawInvS3dFrame(HWND);

void DrawCloseButton(HWND);
void DrawToggleButton(HWND);
void DrawMinimizeButton(HWND);
void DrawTitle(HWND);

int DefaultButtonMH(HMSG);
int CloseButtonMH(HMSG);
int MinButtonMH(HMSG);
int NormButtonMH(HMSG);
int TitleBarMH(HMSG);
int MasterMH(HMSG);

int DefaultWindowMH(HMSG);
#endif

/* TODO: Can not activate a window that is not visible */

#include <stdlib.h>
#include <string.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\wnd.h"
#include "..\common\include\doublell.h"
#include "..\common\include\mouse.h"
#include "..\common\include\message.h"
#include "..\common\include\rect.h"
#include "..\common\include\mem.h"
#include "..\common\include\graphics.h"
#include "..\os\globals.h"

extern DWORD iterations;

HWND gCurActiveWin = NULL;    /* Currently active top level window */
HWND gMasterWin;              /* Desktop window */
RECT gDefWinRect;             /* Default window rectangle, size of screen */
RECT gDefButRect;             /* Default button rectangle, 16 X 16 */

BOOLN dirtyWindows = FALSE;    /* TODO: Set this back to false at correct time */

/************************ System Window Definitions ***************************/

WDEF defaultWDef;
WDEF buttonWDef;

/***************************** Window Functions *******************************/

void InitWindowSystem (void)
{
  WDEF masterDef;

  SetRect (&gDefWinRect, 0, 0, 639, 479);
  SetRect (&gDefButRect, 0, 0, 15, 15);

  defaultWDef.sizeRect = &gDefWinRect;
  defaultWDef.title = NULL;
  defaultWDef.attributes = COPYTITLE|NORMAL;
  defaultWDef.attr2 = 0L;
  defaultWDef.parent = NULL;
  defaultWDef.WinDrawRoutine = NULL;
  defaultWDef.FrameDrawRoutine = Draw3dFrame;
  defaultWDef.bkColor = DEFWINBACKCOLOR;
  defaultWDef.fgColor = DEFWINFORECOLOR;
  defaultWDef.frameColor = DEFWINFRAMECOLOR;
  defaultWDef.UDA = NULL;
  defaultWDef.MessageHandler = DefaultWindowMH;
  defaultWDef.makeActive = TRUE;

  buttonWDef.sizeRect = &gDefButRect;
  buttonWDef.title = NULL;
  buttonWDef.attributes = BUTTONSTYLE|NORMAL;
  buttonWDef.attr2 = 0L;
  buttonWDef.parent = NULL;
  buttonWDef.WinDrawRoutine = NULL;
  buttonWDef.FrameDrawRoutine = DrawS3dFrame;
  buttonWDef.bkColor = DEFBUTTONBACKCOLOR;
  buttonWDef.fgColor = DEFBUTTONFORECOLOR;
  buttonWDef.frameColor = DEFWINFRAMECOLOR;
  buttonWDef.UDA = NULL;
  buttonWDef.MessageHandler = DefaultButtonMH;
  /*buttonWDef.makeActive = FALSE;*/

  CopyWindowDef (&defaultWDef, &masterDef);
  masterDef.sizeRect = &screenRect;
  masterDef.MessageHandler = MasterMH;
  masterDef.attributes = NOFRAME|NOTOGGLE|NOMINIMIZE|MAXIMIZED;
  masterDef.makeActive = FALSE;
  gMasterWin = CreateWindow (&masterDef);
  /*PostMessage (gMasterWin, SM_SHOWWIN, 0L, 0L);
  PostMessage (gMasterWin, SM_ACTIVATEWIN, 0L, 0L);*/
  defaultWDef.parent = gMasterWin;
}

void KillWindowSystem (void)
{
}

HWDEF CreateWindowDef (HWDEF templateWDef)
{
  WDEF *newWDef = malloc (sizeof(WDEF));
  if (newWDef == NULL)
    return NULL;
  CopyWindowDef (templateWDef, newWDef);
  return newWDef;
}

void CopyWindowDef (HWDEF sourceWDef, HWDEF destWDef)
{
  destWDef->sizeRect = sourceWDef->sizeRect;
  destWDef->title = sourceWDef->title;
  destWDef->attributes = sourceWDef->attributes;
  destWDef->attr2 = sourceWDef->attr2;
  destWDef->parent = sourceWDef->parent;
  destWDef->WinDrawRoutine = sourceWDef->WinDrawRoutine;
  destWDef->FrameDrawRoutine = sourceWDef->FrameDrawRoutine;
  destWDef->bkColor = sourceWDef->bkColor;
  destWDef->fgColor = sourceWDef->fgColor;
  destWDef->frameColor = sourceWDef->frameColor;
  destWDef->UDA = sourceWDef->UDA;
  destWDef->MessageHandler = sourceWDef->MessageHandler;
  destWDef->makeActive = sourceWDef->makeActive;
}

HWND CreateWindow (HWDEF winDef)
{
  HWND childWindow;
  HWND titleWindow;
  RECT r;
  int x;
  WDEF cWDefs;
  HSYSBA thisWinSysUDA;

  /* Pointers to structure members used to speed dereferencing */

  char *title = winDef->title;
  DWORD attributes = winDef->attributes;

  /* Create window structure, if out of memory return NULL */

  HWND winHdl = malloc (sizeof(WND));
  if (winHdl == NULL)
    return winHdl;

  /* Fill in window structure */

  winHdl->id = 0;  /* TODO */
  winHdl->title = title;
  if ((attributes & COPYTITLE) == COPYTITLE)
  {
    winHdl->title = malloc(strlen(title) + 1);
    /* Where's the error check, John? */
    strcpy (winHdl->title, title);
  }
  winHdl->bkColor = winDef->bkColor;
  winHdl->fgColor = winDef->fgColor;
  winHdl->frameColor = winDef->frameColor;
  winHdl->parent = winDef->parent;
  winHdl->WinDrawRoutine = winDef->WinDrawRoutine;
  winHdl->FrameDrawRoutine = winDef->FrameDrawRoutine;
  winHdl->MessageHandler = winDef->MessageHandler;
  winHdl->UDA = winDef->UDA;
  CopyRect (winDef->sizeRect, &winHdl->totalRect);
  CopyRect (winDef->sizeRect, &winHdl->innerRect);
  winHdl->children = DLLCreate ();
  winHdl->attributes = attributes;
  winHdl->attr2 = winDef->attr2;
  winHdl->activeChild = NULL;

  /* The window is officially created, now we can do some other things with it */

  if ((attributes & NOFRAME) == 0)
    InflateRectEq (&winHdl->innerRect, (*winHdl->FrameDrawRoutine)(NULL));
  if ((attributes & NOTITLE) == 0)
  {
    /* Create the title window */

    CopyRect (&winHdl->innerRect, &r);
    AdjustRectHeight (&r, TITLEHEIGHT);
    CopyWindowDef (&buttonWDef, &cWDefs);
    cWDefs.UDA = malloc(sizeof(SYSBA));
    thisWinSysUDA = cWDefs.UDA;
    thisWinSysUDA->affectedWindow = winHdl;
    thisWinSysUDA->attributes = 0L;
    cWDefs.sizeRect = &r;
    cWDefs.parent = winHdl;
    cWDefs.bkColor = DEFINACTTITLECOLOR;
    cWDefs.attributes = TITLESTYLE;
    cWDefs.MessageHandler = TitleBarMH;
    titleWindow = CreateWindow (&cWDefs);

    cWDefs.parent = titleWindow;
    cWDefs.attributes = BUTTONSTYLE & NOCOPYTITLE;
    cWDefs.bkColor = DEFBUTTONBACKCOLOR;
    SetRect (&r, winHdl->innerRect.right - 9, winHdl->innerRect.top + 1, winHdl->innerRect.right - 1, winHdl->innerRect.top + 9);

    /* Create the close button */

    x = 0;
    if ((attributes & NOCLOSE) == 0)
    {
      cWDefs.UDA = malloc(sizeof(SYSBA));
      thisWinSysUDA = cWDefs.UDA;
      thisWinSysUDA->affectedWindow = winHdl;
      thisWinSysUDA->attributes = 0L;
      cWDefs.WinDrawRoutine = DrawCloseButton;
      cWDefs.MessageHandler = CloseButtonMH;
      cWDefs.title = "Close";
      childWindow = CreateWindow (&cWDefs);
      MoveRect (&r, -10, 0);
    }

    /* Create the toggle button */

    if ((attributes & NOTOGGLE) == 0)
    {
      cWDefs.UDA = malloc(sizeof(SYSBA));
      thisWinSysUDA = cWDefs.UDA;
      thisWinSysUDA->affectedWindow = winHdl;
      thisWinSysUDA->attributes = 0L;
      cWDefs.WinDrawRoutine = DrawToggleButton;
      cWDefs.MessageHandler = NormButtonMH;
      cWDefs.title = "Toggle";
      childWindow = CreateWindow (&cWDefs);
      MoveRect (&r, -10, 0);
    }

    /* Create the minimize button */

    if ((attributes & NOMINIMIZE) == 0)
    {
      cWDefs.UDA = malloc(sizeof(SYSBA));
      thisWinSysUDA = cWDefs.UDA;
      thisWinSysUDA->affectedWindow = winHdl;
      thisWinSysUDA->attributes = 0L;
      cWDefs.WinDrawRoutine = DrawMinimizeButton;
      cWDefs.MessageHandler = MinButtonMH;
      cWDefs.title = "Minimize";
      childWindow = CreateWindow (&cWDefs);
      MoveRect (&r, -10, 0);
    }

    /* Create the title label */

    cWDefs.UDA = NULL;
    cWDefs.WinDrawRoutine = DrawTitle;
    InflateRect (&r, 0, 1);
    r.left = winHdl->innerRect.left + 1;
    cWDefs.attributes = TITLESTYLE & NOCOPYTITLE;
    cWDefs.bkColor = titleWindow->bkColor;
    cWDefs.fgColor = DEFTITLETEXTCOLOR;
    cWDefs.title = winHdl->title;
    cWDefs.MessageHandler = NULL;
    childWindow = CreateWindow (&cWDefs);
    MoveRect (&r, -10, 0);

    AdjustRectTop (&winHdl->innerRect, TITLEHEIGHT);
  }  /*  end if ((attributes & NOTITLE) == 0) */
  if (winHdl->parent != NULL)
    DLLAddItem (winHdl->parent->children, winHdl);
  if (winDef->makeActive == TRUE)
  {
    PostMessage (winHdl, SM_SHOWWIN, 0L, 0L);
    PostMessage (winHdl, SM_ACTIVATEWIN, 0L, 0L);
  }
  return winHdl;
}

void MoveWindowS (HWND wnd, int x, int y)
{
  HWND curChildWin;

  MoveRect (&wnd->totalRect, x, y);
  MoveRect (&wnd->innerRect, x, y);
  curChildWin = DLLGetFirstItem (wnd->children);
  while (curChildWin != NULL)
  {
    MoveWindowS (curChildWin, x, y);
    curChildWin = DLLGetNextItem (wnd->children);
  }
}

void MoveWindow (HWND wnd, int x, int y)
{
  MoveWindowS (wnd, x, y);
  /*if (wnd->parent != NULL)
    ShowWindow (wnd->parent);
  else
    ShowWindow (wnd);*/
  MakeDirty (wnd);
}

void ShowWindow (HWND wnd)
{
  HWND curChildWin;
  dll_position p;

  HideMouse ();
  if ((wnd->attributes & NOFRAME) == 0)
    (*wnd->FrameDrawRoutine) (wnd);
  SetColor (wnd->bkColor);
  WindowRectFilled (wnd, wnd->innerRect);
  curChildWin = DLLGetFirstItem (wnd->children);
  while (curChildWin != NULL)
  {
    DLLGetCurrentPosition (wnd->children, &p);
    ShowWindow (curChildWin);
    DLLSetCurrentPosition (wnd->children, &p);
    curChildWin = DLLGetNextItem (wnd->children);
  }
  if (wnd->WinDrawRoutine != NULL)
  {
    SetViewPortRect (&wnd->innerRect);
    wnd->WinDrawRoutine (wnd);
    SetViewPortRect (&screenRect);
  }
  MakeNotDirty (wnd);
  ShowMouse ();
}

void DeactivateWindow (HWND wnd)
{
  HWND titleWin;
  HWND titleTextWin;
  HWND activeSibling = wnd->parent->activeChild;

  if ((wnd != activeSibling)||(activeSibling == NULL))
    return;
  titleWin = DLLGetFirstItem (activeSibling->children);
  titleWin->bkColor = DEFINACTTITLECOLOR;
  titleTextWin = DLLGetFirstItem (titleWin->children);
  while ((titleTextWin->title != activeSibling->title) && (titleTextWin != NULL))
    titleTextWin = DLLGetNextItem (titleWin->children);
  if (titleTextWin != NULL)
    titleTextWin->bkColor = DEFINACTTITLECOLOR;
  wnd->parent->activeChild = NULL;
}

void ActivateWindow (HWND wnd)
{
  /* For now the title window is always the first child window */

  HWND titleWin;
  HWND titleTextWin;
  HWND activeSibling = wnd->parent->activeChild;
  if ((wnd->parent == NULL)||(wnd == activeSibling))
    return;

  if (activeSibling != NULL)
  {
    PostMessage (activeSibling, SM_DEACTIVATEWIN, (DWORD)wnd, 0L);
    return;
  }
  titleWin = DLLGetFirstItem (wnd->children);
  titleWin->bkColor = DEFTITLECOLOR;
  titleTextWin = DLLGetFirstItem (titleWin->children);
  while ((strcmp(titleTextWin->title, wnd->title) != 0) && titleTextWin != NULL)
    titleTextWin = DLLGetNextItem (titleWin->children);
  if (titleTextWin != NULL)
    titleTextWin->bkColor = DEFTITLECOLOR;
  wnd->parent->activeChild = wnd;
  PostMessage (wnd, SM_MOVETOFRONT, 0L, 0L);
  if (wnd->parent != NULL)
    /* Maybe this should be replaced with something like RedrawChildTitles() */
    RedrawChildren (wnd);
}

void MoveWindowToFront (HWND wnd)
{
  HWND curChildWin;

  if (wnd->parent != NULL)
  {
    /* Delete window from its parent's children list */
    curChildWin = DLLGetFirstItem (wnd->parent->children);
    while (curChildWin != wnd)
      curChildWin = DLLGetNextItem (wnd->parent->children);
    DLLDeleteElement (wnd->parent->children);

    /* Add the window back in to the parent's children last, now it's last */
    DLLAddItem (wnd->parent->children, wnd);

    /*ShowWindow (wnd);*/
    MakeDirty (wnd);
  }
}

void RedrawChildren (HWND wnd)
{
  HWND curChildWin;

  curChildWin = DLLGetFirstItem (wnd->children);
  while (curChildWin != NULL)
  {
    /*ShowWindow (curChildWin);*/
    MakeDirty (curChildWin);
    curChildWin = DLLGetNextItem (wnd->children);
  }
}

HWND WindowAtPointEx (POINT pt, HWND wnd)
{
  HWND curChildWin;
  HWND windowAtPoint = NULL;

  curChildWin = DLLGetLastItem (wnd->children);
  while (curChildWin != NULL && windowAtPoint == NULL)
  {
    if ((curChildWin->attributes & STATEMASK) != MINIMIZED)
      if (PtInRect (&curChildWin->totalRect, pt) == TRUE)
      {
        windowAtPoint = WindowAtPointEx (pt, curChildWin);
        if (windowAtPoint == NULL)
          windowAtPoint = curChildWin;
      }
    curChildWin = DLLGetPrevItem (wnd->children);
  }
  return windowAtPoint;
}

HWND WindowAtPoint (POINT pt)
{
  HWND curChildWin;
  HWND windowAtPoint = NULL;

  iterations++;
  curChildWin = DLLGetLastItem (gMasterWin->children);
  while (curChildWin != NULL && windowAtPoint == NULL)
  {
    if ((curChildWin->attributes & STATEMASK) != MINIMIZED)
      if (PtInRect (&curChildWin->totalRect, pt) == TRUE)
      {
        windowAtPoint = WindowAtPointEx (pt, curChildWin);
        if (windowAtPoint == NULL)
          windowAtPoint = curChildWin;
      }
    curChildWin = DLLGetPrevItem (gMasterWin->children);
  }
  if (windowAtPoint == NULL)
    windowAtPoint = gMasterWin;
  return windowAtPoint;
}

void DestroyWindow (HWND wnd)
{
  HWND curChildWin;
  HWND parentWin = wnd->parent;
  dlinkedlist *children = wnd->children;

  /* Mark window as being destroyed */
  wnd->attributes &= DESTROYMASK;

  /* Delete the children of this window */

  curChildWin = DLLGetFirstItem (children);
  while (curChildWin != NULL)
  {
    DestroyWindow (curChildWin);
    curChildWin = DLLGetFirstItem (children);
  }
  DLLDestroy (children);

  /* Delete this window from its parent's window list */
  if (parentWin != NULL)
  {
    curChildWin = DLLGetFirstItem (parentWin->children);
    while (curChildWin != wnd)
      curChildWin = DLLGetNextItem (parentWin->children);
    DLLDeleteElement (parentWin->children);
  }
  if (wnd->title != NULL)
  {
    if ((wnd->attributes & COPYTITLE) == COPYTITLE)
      free (wnd->title);
  }
  if (wnd->UDA != NULL)
    free (wnd->UDA);
  free (wnd);
  if (parentWin != NULL)
  {
    if ((parentWin->attributes & STATEMASK) != 0)
      MakeDirty (parentWin);
      /*ShowWindow (parentWin);*/
  }
}

/************************* Frame Drawing Routines *****************************/

int DrawFlatFrame (HWND wnd)
{
  if (wnd != NULL)
  {
    SetColor (wnd->frameColor);
    WindowRect (wnd, wnd->totalRect);
  }
  return -1;
}

int Draw3dFrame (HWND wnd)
{
  if (wnd != NULL)
    WinDrawRect3d (wnd, wnd->totalRect);
  return -2;
}

int DrawInv3dFrame (HWND wnd)
{
  if (wnd != NULL)
    WinDrawInvRect3d (wnd, wnd->totalRect);
  return -2;
}

int DrawS3dFrame (HWND wnd)
{
  if (wnd != NULL)
    WinDrawSRect3d (wnd, wnd->totalRect);
  return -1;
}

int DrawInvS3dFrame (HWND wnd)
{
  if (wnd != NULL)
    WinDrawInvSRect3d (wnd, wnd->totalRect);
  return -1;
}

/********************** System Window Drawing Routines ************************/

void DrawCloseButton (HWND wnd)
{
  SetColor (wnd->fgColor);
  _moveto (1, 1);
  _lineto (SYSBUTTONMAX, SYSBUTTONMAX);
  _moveto (SYSBUTTONMAX, 1);
  _lineto (1, SYSBUTTONMAX);
}

void DrawToggleButton (HWND wnd)
{
  RECT r;

  SetColor (wnd->fgColor);
  SetRect (&r, 1, 1, SYSBUTTONMAX, SYSBUTTONMAX);
  DrawRect (&r);
  InflateRectEq (&r, -2);
  DrawRect (&r);
}

void DrawMinimizeButton (HWND wnd)
{
  RECT r;

  SetColor (wnd->fgColor);
  SetRect (&r, 1, SYSBUTTONMAX, SYSBUTTONMAX, SYSBUTTONMAX);
  DrawRect (&r);
}

void DrawTitle (HWND wnd)
{
  SetColor (wnd->fgColor);
  _moveto (0, 0);
  _setfont ("t'helv'h10");
  _outgtext (wnd->title);
}

/* System Message Handlers */

int DefaultButtonMH (HMSG msg)
{
  HWND wnd = msg->wnd;
  HSYSBA bUDA = wnd->UDA;

  switch (msg->message)
  {
    case SM_LBUTTONDN:
      PostMessage (wnd->parent->parent, SM_ACTIVATEWIN, 0L, 0L);
      wnd->FrameDrawRoutine = DrawInvS3dFrame;
      DrawInvS3dFrame (wnd);
      bUDA->attributes |= BUTTONDN;
      SetMouseCapture (wnd);
      return 1;

    case SM_LBUTTONUP:
      wnd->FrameDrawRoutine = DrawS3dFrame;
      DrawS3dFrame (wnd);
      bUDA->attributes &= UPDNMASK;
      SetMouseCapture (NULL);
      return 1;

    case SM_RBUTTONDN:
    case SM_MBUTTONDN:
      PostMessage (wnd->parent->parent, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

  case SM_MOUSEOFF:
      if ((bUDA->attributes & BUTTONDN) == BUTTONDN)
      {
        wnd->FrameDrawRoutine = DrawS3dFrame;
        DrawS3dFrame (wnd);
      }
      return 1;

    case SM_MOUSEON:
      if ((bUDA->attributes & BUTTONDN) == BUTTONDN)
      {
        wnd->FrameDrawRoutine = DrawInvS3dFrame;
        DrawInvS3dFrame (wnd);
      }
      return 1;

    default:
      return 0;
  }
}

int CloseButtonMH (HMSG msg)
{
  HSYSBA bUDA = msg->wnd->UDA;

  switch (msg->message)
  {
    case SM_LCLICK:
      PostMessage (bUDA->affectedWindow, SM_CLOSE, 0L, 0L);
      return 1;

    default:
      return DefaultButtonMH(msg);
  }
}

int MinButtonMH (HMSG msg)
{
  HSYSBA bUDA = msg->wnd->UDA;

  switch (msg->message)
  {
    case SM_LCLICK:
      PostMessage (bUDA->affectedWindow, SM_MINIMIZE, 0L, 0L);
      return 1;

    default:
      return DefaultButtonMH(msg);
  }
}

int NormButtonMH (HMSG msg)
{
  HSYSBA bUDA = msg->wnd->UDA;

  switch (msg->message)
  {
    case SM_LCLICK:
      PostMessage (bUDA->affectedWindow, SM_TOGGLEMAX, 0L, 0L);
      return 1;

    default:
      return DefaultButtonMH(msg);
  }
}

int TitleBarMH (HMSG msg)
{
  HSYSBA bUDA = msg->wnd->UDA;

  switch (msg->message)
  {
    case SM_LBUTTONDN:
      PostMessage (bUDA->affectedWindow, SM_ACTIVATEWIN, 0L, 0L);
      PostMessage (bUDA->affectedWindow, SM_BEGINDRAG, msg->param1, msg->param2);
      return 1;

    case SM_RBUTTONDN:
    case SM_MBUTTONDN:
      PostMessage (bUDA->affectedWindow, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

    default:
      return DefaultWindowMH(msg);
  }
}

int MasterMH (HMSG msg)
{
  if (msg->message == SM_CLOSE)
  {
    DestroyWindow (msg->wnd);
    PostMessage (NULL, SM_EXITOS, 0L, 0L);
    return 1;
  }
  return 0;
}

int DefaultWindowMH (HMSG msg)
{
  HWND wnd = msg->wnd;
  DWORD *attributes = &wnd->attributes;

  switch (msg->message)
  {
    case SM_LBUTTONDN:
      PostMessage (wnd, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

    case SM_LBUTTONUP:
      if ((*attributes & DRAGGING) == DRAGGING)
      {
        int dX, dY;
        dX = (int)(msg->param1) - (int)(wnd->attr2 >> 16);
        dY = (int)(msg->param2) - (int)(wnd->attr2 & 0x0000FFFFL);
        SetMouseCapture (NULL);
        *attributes &= NOTDRAGGING;
        MoveWindow (wnd, dX, dY);
        if (wnd->parent != NULL)
          MakeDirty (wnd->parent);
      }
      return 1;

    case SM_RBUTTONDN:
      PostMessage (wnd, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

    case SM_RBUTTONUP:
      return 1;

    case SM_MBUTTONDN:
      PostMessage (wnd, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

    case SM_MBUTTONUP:
      return 1;

    case SM_LCLICK:
      return 1;

    case SM_RCLICK:
      return 1;

    case SM_MCLICK:
      return 1;

    case SM_LDCLICK:
      return 1;

    case SM_RDCLICK:
      return 1;

    case SM_MDCLICK:
      return 1;

    case SM_MOUSEON:
      return 1;

    case SM_MOUSEOFF:
      return 1;

    case SM_MOUSEMOVE:
      if ((*attributes & DRAGGING) == DRAGGING)
      {
        int dX, dY;
        dX = (int)(msg->param1) - (int)(wnd->attr2 >> 16);
        dY = (int)(msg->param2) - (int)(wnd->attr2 & 0x0000FFFFL);
        MoveWindow (wnd, dX, dY);
        wnd->attr2 = (msg->param1 << 16 | msg->param2);
      }
      return 1;

    case SM_CLOSE:
      DestroyWindow (wnd);
      return 1;

    case SM_MINIMIZE:
      return 1;

    case SM_TOGGLEMAX:
      return 1;

    case SM_BEGINDRAG:
      wnd->attr2 = (msg->param1 << 16) | msg->param2;
      SetMouseCapture (wnd);
      *attributes |= DRAGGING;
      return 1;

    case SM_DEACTIVATEWIN:
      DeactivateWindow (wnd);
      PostMessage ((HWND)msg->param1, SM_ACTIVATEWIN, 0L, 0L);
      return 1;

    case SM_ACTIVATEWIN:
      ActivateWindow (wnd);
      return 1;

    case SM_SHOWWIN:
      ShowWindow (wnd);
      return 1;

    case SM_MOVETOFRONT:
      MoveWindowToFront (wnd);
      return 1;

  }
}

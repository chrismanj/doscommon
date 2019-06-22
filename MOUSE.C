#include <dos.h>
#include <stdlib.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\rect.h"
#include "..\common\include\doublell.h"
#include "..\common\include\wnd.h"
#include "..\common\include\message.h"
#include "..\common\include\mouse.h"

HWND curMouseCapWin = NULL;
HWND lastMouseWin = NULL;
HWND lastMouseLDnWin = NULL;
HWND lastMouseCDnWin = NULL;
HWND lastMouseRDnWin = NULL;

POINT mousePt, lastMousePt;
int buttons, oldButtons;

int ResetMouse(int *numButtons)
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x00;
  int86(MOUSE_INT, &inregs, &outregs);
  *numButtons = outregs.x.bx;
  GetMouseInfo(&mousePt.x, &mousePt.y, &buttons);
  lastMousePt.x = mousePt.x;
  lastMousePt.y = mousePt.y;
  oldButtons = buttons;
  return (outregs.x.ax);
}

void ShowMouse(void)
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x01;
  int86(MOUSE_INT, &inregs, &outregs);
}

void HideMouse(void)
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x02;
  int86(MOUSE_INT, &inregs, &outregs);
}

void GetMouseInfo(short *x, short *y, int *buttons)
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x03;
  int86(MOUSE_INT, &inregs, &outregs);
  *x = outregs.x.cx;
  *y = outregs.x.dx;
  *buttons = outregs.x.bx;
}

void GetMouseMotionRel(int *x, int *y)
{
  union REGS inregs, outregs;

  inregs.x.ax = 0x0B;
  int86(MOUSE_INT, &inregs, &outregs);

  *x = outregs.x.cx;
  *y = outregs.x.dx;
}

void SetMouseSensitivity(int x, int y, int dblClick)
{
  union REGS inregs, outregs;

  inregs.x.bx = x;
  inregs.x.cx = y;
  inregs.x.dx = dblClick;

  inregs.x.ax = 0x1A;
  int86(MOUSE_INT, &inregs, &outregs);
}

void SetMouseCapture(HWND wnd)
{
  curMouseCapWin = wnd;
}

void CheckMouseMessages()
{
  HWND targetWin = NULL;
  HWND actualWin = NULL;

  GetMouseInfo(&mousePt.x, &mousePt.y, &buttons);
  actualWin = WindowAtPoint(mousePt);
  if (curMouseCapWin == NULL)
    targetWin = actualWin;
  else
    targetWin = curMouseCapWin;
  if (mousePt.x != lastMousePt.x || mousePt.y != lastMousePt.y)
  {
    /* Check for MOUSEON/MOUSEOFF messages that need to be sent */

    if (curMouseCapWin != NULL)
    {
      /* If a window has the mouse captured then only the window capturing the
        mouse can receive mouseon, mouseoff, and mousemove messages */
      if (lastMouseWin == curMouseCapWin && actualWin != curMouseCapWin)
        PostMessage(curMouseCapWin, SM_MOUSEOFF, 0L, 0L);
      if (lastMouseWin != curMouseCapWin && actualWin == curMouseCapWin)
        PostMessage(curMouseCapWin, SM_MOUSEON, 0L, 0L);
      PostMessage(curMouseCapWin, SM_MOUSEMOVE, mousePt.x, mousePt.y);
    }
    else
    {

      if (lastMouseWin != NULL && lastMouseWin != targetWin)
        PostMessage(lastMouseWin, SM_MOUSEOFF, 0L, 0L);
      if (actualWin != NULL)
      {
        if (lastMouseWin != NULL && lastMouseWin != targetWin)
          PostMessage(targetWin, SM_MOUSEON, 0L, 0L);
        PostMessage(targetWin, SM_MOUSEMOVE, mousePt.x, mousePt.y);
      }
    }
    lastMouseWin = actualWin;
  }
  if (buttons != oldButtons)
  {
    if (targetWin != NULL)
    {
      if ((buttons & MOUSELBUTTON) != (oldButtons & MOUSELBUTTON))
      {
        if ((buttons & MOUSELBUTTON) == 0)
        {
          if (lastMouseLDnWin == actualWin)
            PostMessage(targetWin, SM_LCLICK, 0L, 0L);
          PostMessage(targetWin, SM_LBUTTONUP, mousePt.x, mousePt.y);
        }
        else
        {
          PostMessage(targetWin, SM_LBUTTONDN, mousePt.x, mousePt.y);
          lastMouseLDnWin = targetWin;
        }
      }
      if ((buttons & MOUSERBUTTON) != (oldButtons & MOUSERBUTTON))
      {
        if ((buttons & MOUSERBUTTON) == 0)
        {
          if (lastMouseRDnWin == actualWin)
            PostMessage(targetWin, SM_RCLICK, 0L, 0L);
          PostMessage(targetWin, SM_RBUTTONUP, mousePt.x, mousePt.y);
        }
        else
        {
          PostMessage(targetWin, SM_RBUTTONDN, mousePt.x, mousePt.y);
          lastMouseRDnWin = targetWin;
        }
      }
      if ((buttons & MOUSECBUTTON) != (oldButtons & MOUSECBUTTON))
      {
        if ((buttons & MOUSECBUTTON) == 0)
        {
          if (lastMouseCDnWin == actualWin)
            PostMessage(targetWin, SM_MCLICK, 0L, 0L);
          PostMessage(targetWin, SM_MBUTTONUP, mousePt.x, mousePt.y);
        }
        else
        {
          PostMessage(targetWin, SM_MBUTTONDN, mousePt.x, mousePt.y);
          lastMouseCDnWin = targetWin;
        }
      }
    }
  }
  lastMousePt.x = mousePt.x;
  lastMousePt.y = mousePt.y;
  oldButtons = buttons;
}

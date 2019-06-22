#include <graphics.h>
#include <stdio.h>
#include <conio.h>

#include "c:\progproj\c\common\include\types.h"
#include "c:\progproj\c\common\include\debug.h"
#include "c:\progproj\c\common\include\rect.h"
#include "c:\progproj\c\common\include\wnd.h"
#include "c:\progproj\c\common\include\graphics.h"
#include "c:\progproj\c\common\include\mouse.h"
#include "c:\progproj\c\common\include\jsctime.h"

extern long fillTimer;
extern long pointTestTimer;
extern RECT screenRect;

int curColor;

void GModeOn (void)
{
	_asm {
		mov ah, 0x00
			mov al, 0x12
			int 0x10
	}
}

void GModeOff (void)
{
	_asm {
		mov ah, 0x00
			mov al, 0x02
			int 0x10
	}
	
}

void PlotPixel (WORD x, WORD y)
{
	putpixel (x, y, curColor);
}

void FillScreen (void)
{
	short x, y;
	
	setcolor (1);
	fillTimer = StartTimer();
	for (y = 0; y < 480; y++)
		for (x = 0; x < 640; x++)
			PlotPixel (x, y);
		fillTimer = TimerValue (fillTimer);
}

void TestEaPtInScreen (void)
{
	POINT p;
	
	pointTestTimer = StartTimer();
	for (p.y = 0; p.y < 480; p.y++)
		for (p.x = 0; p.x < 640; p.x++)
			WindowAtPoint (p);
		pointTestTimer = TimerValue (pointTestTimer);
}


DWORD SetColor (DWORD color)
{
	DWORD curColor = getcolor();
	setcolor ((int)color);
	return curColor;
}

void WindowPlot (HWND wnd, POINT p)
{
	if (WindowAtPoint (p) == wnd)
		PlotPixel (p.x, p.y);
}

void WindowHLine (HWND wnd, short x1, short x2, short y)
{
	POINT lineBeg;
	POINT lineEnd;
	
	lineBeg.x = x1;
	lineBeg.y = y;
	lineEnd.x = x1;
	lineEnd.y = y;
	
	while (lineEnd.x < x2)
	{
		/* Find the beginning of the line */
		while (WindowAtPoint(lineBeg) != wnd && lineBeg.x < x2)
			lineBeg.x++;
		
		/* Find the end of the line */
		lineEnd.x = lineBeg.x;
		while (WindowAtPoint(lineEnd) != wnd && lineEnd.x < x2)
			lineEnd.x++;
		
		moveto (lineBeg.x, lineBeg.y);
		lineto (lineEnd.x, lineEnd.y);
		lineBeg.x = lineEnd.x + 1;
	}
}

void WindowVLine (HWND wnd, short y1, short y2, short x)
{
	POINT p1;
	POINT p2;
	
	p1.x = x;
	p1.y = y2;
	if (y1 < y2)
	{
		p1.y = y1;
		p2.y = y2;
	}
	else
	{
		p1.y = y2;
		p2.y = y1;
	}
	for (; p1.y <= p2.y; p1.y++)
	{
		if (WindowAtPoint (p1) == wnd)
			PlotPixel (p1.x, p1.y);
	}
}

void WindowRect (HWND wnd, RECT r)
{
	WindowHLine (wnd, r.left, r.right, r.top);
	WindowVLine (wnd, r.top, r.bottom, r.right);
	WindowHLine (wnd, r.left, r.right, r.bottom);
	WindowVLine (wnd, r.top, r.bottom, r.left);
}

void WindowRectFilled (HWND wnd, RECT r)
{
	HideMouse();
	while (r.top <= r.bottom)
	{
		WindowHLine (wnd, r.left, r.right, r.top);
		r.top++;
	}
	ShowMouse();
}

void WinDrawRect3d (HWND wnd, RECT r)
{
	DWORD oldColor = SetColor (UL3dCOLOROUT);
	HideMouse ();
	WindowVLine (wnd, r.top, r.bottom, r.left);
	WindowHLine (wnd, r.left, r.right, r.top);
	SetColor (UL3dCOLORIN);
	WindowVLine (wnd, r.top + 1, r.bottom, r.left + 1);
	WindowHLine (wnd, r.left + 1, r.right, r.top + 1);
	SetColor (LR3dCOLOROUT);
	WindowVLine (wnd, r.top + 1, r.bottom, r.right);
	WindowHLine (wnd, r.left, r.right, r.bottom);
	SetColor (LR3dCOLORIN);
	WindowVLine (wnd, r.top + 2, r.bottom - 1, r.right - 1);
	WindowHLine (wnd, r.left + 1, r.right - 1, r.bottom - 1);
	SetColor (oldColor);
	ShowMouse ();
}

void WinDrawInvRect3d (HWND wnd, RECT r)
{
	DWORD oldColor = SetColor (LR3dCOLOROUT);
	HideMouse ();
	WindowVLine (wnd, r.top, r.bottom, r.left);
	WindowHLine (wnd, r.left, r.right, r.top);
	SetColor (LR3dCOLORIN);
	WindowVLine (wnd, r.top + 1, r.bottom, r.left + 1);
	WindowHLine (wnd, r.left + 1, r.right, r.top + 1);
	SetColor (UL3dCOLOROUT);
	WindowVLine (wnd, r.top + 1, r.bottom, r.right);
	WindowHLine (wnd, r.left, r.right, r.bottom);
	SetColor (UL3dCOLORIN);
	WindowVLine (wnd, r.top + 2, r.bottom - 1, r.right - 1);
	WindowHLine (wnd, r.left + 1, r.right - 1, r.bottom - 1);
	SetColor (oldColor);
	ShowMouse ();
}

void WinDrawSRect3d (HWND wnd, RECT r)
{
	DWORD oldColor = SetColor (UL3dCOLORIN);
	HideMouse ();
	WindowVLine (wnd, r.top, r.bottom, r.left);
	WindowHLine (wnd, r.left, r.right, r.top);
	SetColor (LR3dCOLOROUT);
	WindowVLine (wnd, r.top + 1, r.bottom, r.right);
	WindowHLine (wnd, r.left, r.right, r.bottom);
	SetColor (oldColor);
	ShowMouse ();
}

void WinDrawInvSRect3d (HWND wnd, RECT r)
{
	DWORD oldColor = SetColor (LR3dCOLOROUT);
	HideMouse ();
	WindowVLine (wnd, r.top, r.bottom, r.left);
	WindowHLine (wnd, r.left, r.right, r.top);
	SetColor (UL3dCOLORIN);
	WindowVLine (wnd, r.top + 1, r.bottom, r.right);
	WindowHLine (wnd, r.left, r.right, r.bottom);
	SetColor (oldColor);
	ShowMouse ();
}

#include <stdlib.h>
#include "c:\progproj\c\common\include\types.h"
#include "c:\progproj\c\common\include\debug.h"
#include "c:\progproj\c\common\include\rect.h"
#include "c:\progproj\c\common\include\doublell.h"
#include "c:\progproj\c\common\include\wnd.h"
#include "c:\progproj\c\common\include\mouse.h"

void AdjustRectHeight (HRECT r, int height)
{
	r->bottom = r->top + height;
}

void AdjustRectTop (HRECT r, int amnt)
{
	r->top += amnt;
}

void AdjustRectRight (HRECT r, int amnt)
{
	r->right += amnt;
}

void CopyRect (HRECT src, HRECT dest)
{
	dest->top = src->top;
	dest->bottom = src->bottom;
	dest->left = src->left;
	dest->right = src->right;
}

void DrawRect (HRECT r)
{
	HideMouse ();
	_rectangle (_GBORDER, r->left, r->top, r->right, r->bottom);
	ShowMouse ();
}

void DrawFilledRect (HRECT r)
{
  HideMouse ();
	_rectangle (_GFILLINTERIOR, r->left, r->top, r->right, r->bottom);
  ShowMouse ();
}

void DrawRect3d (HRECT r)
{
  int oldColor = _setcolor (UL3dCOLOROUT);
  HideMouse ();
  _moveto (r->left, r->bottom);
	_lineto (r->left, r->top);
	_lineto (r->right, r->top);
	_setcolor (UL3dCOLORIN);
	_moveto (r->left + 1, r->bottom);
	_lineto (r->left + 1, r->top + 1);
	_lineto (r->right, r->top + 1);
	_setcolor (LR3dCOLOROUT);
	_moveto (r->right, r->top + 1);
	_lineto (r->right, r->bottom);
	_lineto (r->left, r->bottom);
	_setcolor (LR3dCOLORIN);
	_moveto (r->right - 1, r->top + 2);
	_lineto (r->right - 1, r->bottom - 1);
	_lineto (r->left + 1, r->bottom - 1);
	_setcolor (oldColor);
  ShowMouse ();
}

void DrawInvRect3d (HRECT r)
{
  int oldColor = _setcolor (LR3dCOLOROUT);
  HideMouse ();
  _moveto (r->left, r->bottom);
	_lineto (r->left, r->top);
	_lineto (r->right, r->top);
	_setcolor (LR3dCOLORIN);
	_moveto (r->left + 1, r->bottom);
	_lineto (r->left + 1, r->top + 1);
	_lineto (r->right, r->top + 1);
	_setcolor (UL3dCOLOROUT);
	_moveto (r->right, r->top + 1);
	_lineto (r->right, r->bottom);
	_lineto (r->left, r->bottom);
	_setcolor (UL3dCOLORIN);
	_moveto (r->right - 1, r->top + 2);
	_lineto (r->right - 1, r->bottom - 1);
	_lineto (r->left + 1, r->bottom - 1);
	_setcolor (oldColor);
  ShowMouse ();
}

void DrawSRect3d (HRECT r)
{
	int oldColor = _setcolor (UL3dCOLORIN);
  HideMouse ();
	_moveto (r->left, r->bottom);
	_lineto (r->left, r->top);
	_lineto (r->right, r->top);
	_setcolor (LR3dCOLOROUT);
	_moveto (r->right, r->top + 1);
	_lineto (r->right, r->bottom);
	_lineto (r->left, r->bottom);
	_setcolor (oldColor);
  ShowMouse ();
}

void DrawInvSRect3d (HRECT r)
{
  int oldColor = _setcolor (LR3dCOLOROUT);
  HideMouse ();
  _moveto (r->left, r->bottom);
	_lineto (r->left, r->top);
	_lineto (r->right, r->top);
	_setcolor (UL3dCOLORIN);
	_moveto (r->right, r->top + 1);
	_lineto (r->right, r->bottom);
	_lineto (r->left, r->bottom);
	_setcolor (oldColor);
  ShowMouse ();
}

BOOLN EqualRect (HRECT r1, HRECT r2)
{
	BOOLN retVal = TRUE;
	if (r1->top != r2->top)
		retVal = FALSE;
	if (r1->bottom != r2->bottom)
		retVal = FALSE;
	if (r1->left != r2->left)
		retVal = FALSE;
	if (r1->right != r2->right)
		retVal = FALSE;
	return retVal;
}

void InflateRect (HRECT r, int dX, int dY)
{
	r->top -= dY;
	r->bottom += dY;
	r->left -= dX;
	r->right += dX;
}

void InflateRectEq (HRECT r, int amt)
{
	r->top -= amt;
	r->bottom += amt;
	r->left -= amt;
	r->right += amt;
}

BOOLN IntersectRect (HRECT dest, HRECT r1, HRECT r2)
{
	SetRectEmpty (dest);
	if (r1->right < r2->left)
		return FALSE;
	if (r1->bottom < r2->top)
		return FALSE;
	dest->left = max (r1->left, r2->left);
	dest->right = min (r1->right, r2->right);
	dest->top = max (r1->top, r2->top);
	dest->bottom = min (r1->bottom, r2->bottom);
	return TRUE;
}

BOOLN IsRectEmpty (HRECT r)
{
	if (r->top == 0)
		return FALSE;
	if (r->bottom == 0)
		return FALSE;
	if (r->left == 0)
		return FALSE;
	if (r->right == 0)
		return FALSE;
	return TRUE;
}

void MoveRect (HRECT r, int dX, int dY)
{
	r->left += dX;
	r->right += dX;
	r->top += dY;
	r->bottom += dY;
}

void MoveRectHorz (HRECT r, int dX)
{
	r->left += dX;
	r->right += dX;
}

void MoveRectVert (HRECT r, int dY)
{
	r->top += dY;
	r->bottom += dY;
}

BOOLN PtInRect (HRECT r, POINT p)
{
	if (p.x < r->left || p.x > r->right)
		return FALSE;
	if (p.y < r->top || p.y > r->bottom)
		return FALSE;
	return TRUE;
}

void SetRect (HRECT r, int left, int top, int right, int bottom)
{
	r->left = left;
	r->top = top;
	r->right = right;
	r->bottom = bottom;
}

void SetRectEmpty (HRECT r)
{
	r->top = r->bottom = r->left = r->right = 0;
}

void UnionRect (HRECT dest, HRECT r1, HRECT r2)
{
	dest->left = min (r1->left, r2->left);
	dest->right = max (r1->right, r2->right);
	dest->top = min (r1->top, r2->top);
	dest->bottom = max (r1->bottom, r2->bottom);
}

void SetClipRect (HRECT r)
{
	_setcliprgn(r->left, r->top, r->right, r->bottom);
}

void SetViewPortRect (HRECT r)
{
	_setviewport(r->left, r->top, r->right, r->bottom);
}

#ifndef RECT_INC

#define RECT_INC 1

#ifdef _MSC_VER
#include <graph.h>
#else
#include <graphics.h>
#endif

#include "c:\progproj\c\common\include\types.h"
#include "c:\progproj\c\common\include\rectt.h"

#define UL3dCOLOROUT 7
#define UL3dCOLORIN 15
#define LR3dCOLOROUT 0
#define LR3dCOLORIN 8

struct sRect
{
  int top;
  int bottom;
  int left;
  int right;
};

struct sPoint
{
  short x;
  short y;
};

void AdjustRectHeight(HRECT, int);
void AdjustRectRight(HRECT, int);
void AdjustRectTop(HRECT, int);
void CopyRect(HRECT, HRECT);
void DrawRect(HRECT);
void DrawFilledRect(HRECT);
void DrawInvRect3d(HRECT);
void DrawInvSRect3d(HRECT);
void DrawRect3d(HRECT);
void DrawSRect3d(HRECT);
BOOLN EqualRect(HRECT, HRECT);
void InflateRect(HRECT, int, int);
void InflateRectEq(HRECT, int);
BOOLN IntersectRect(HRECT, HRECT, HRECT);
BOOLN IsRectEmpty(HRECT r);
void MoveRect(HRECT, int, int);
void MoveRectHorz(HRECT, int);
void MoveRectVert(HRECT, int);
BOOLN PtInRect(HRECT, POINT);
void SetClipRect(HRECT);
void SetRect(HRECT, int, int, int, int);
void SetRectEmpty(HRECT);
void SetViewPortRect(HRECT r);
void UnionRect(HRECT, HRECT, HRECT);
#endif

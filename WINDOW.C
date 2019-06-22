/******************************************************************************\
				  WINDOW.C
 09-03-98: Initial version created
 09-06-98: Added the following code to make line wrap optional
	   line_wrap boolean added to object
	   Code added in WindowPrintChar
	   Routines WindowGetLineWrap and WindowSetLineWrap created
 09-06-98: Added WindowResetAttribute routine
	   Added WindowSetAttribute routine
	   Added WindowGetAttribute routine
 09-18-98: Added WindowGetCharAt routine
 09-23-98: Changed WindowCursorAt to return if the coordinates passed are
	   outside the window
 10-08-98: Added WindowEraseToBOL
	   Added WindowEraseLine
	   Fixed WindowGetCharAt (For some reason it never used the window
	     handle passed to it)
	   Added def_attrib member variable and modified the following
	     routines to use it instead of cur_attrib-
	     WindowScrollUp
	     WindowScrollDown
	     WindowEraseToEnd
	     WindowEraseToEOL
	     WindowEraseToBOL
	     WindowEraseLine
	     WindowClear
	   Modified WindowResetAttrib to actually change the cur_attrib member
	     variable instead of just recoloring the characters in the window
 10-09-98: Modified WindowScrollUp and WindowScrollDown to take new parameters
	     row, col, row2, & col2 instead of just the window handle.
 10-12-98: Added cursor_shape member variable
	   Added WindowSetCursorShape, WindowGetCursorShape Routines
	   Added sound_on member variable
	   Added WindowGetSoundOn & WindowSetSoundOn routines
 10-13-98: Added WindowGetDefAttrib & WindowSetDefAttrib routines
	   Changed WindowPrintChar so it prints nothing if character 0 is passed
	   Added empty TAB handler to WindowPrintChar
 10-15-98: Added WindowPrintString routine
 10-17-98: Added title, attributes, pSaveRectBuff, top_margin, bottom_margin,
	    left_margin, & right_margin member variables
	   Added WindowShowTitle routine
	   Modified almost all routines to use margin variables instead of top,
	    bottom, left, & right.
 10-20-98: Added WindowSave & WindowRestore
           Fixed WindowResetAttribute bug and made it do what I intended for it
	     to do in the first place,
 11-11-98: Added WindowSetLeftMargin & WindowSetRightMargin
 11-17-98: Added WindowMove
 11-18-98: Added the following features to WindowShowTitle:
           Made it redraw the top part of the window frame in case the new title
             is shorter than the old one.
           Made it cut off the end of the title if it would be drawn past the
             right hand side of the window.
\******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\chrgraph.h>
#include <c:\progproj\c\common\include\speaker.h>
#include <c:\progproj\c\common\include\window.h>

#define COLS 80
#define ROWS 25

extern char *screen_base;
extern WORD frameattrib;

/******************************************************************************\

\******************************************************************************/

winhdl *CreateWindow (char *title, int top, int left, int height, int width, WORD attrib, DWORD attributes)
{
  winhdl *new_window = malloc (sizeof(winhdl));
  if (new_window != NULL)
  {
    new_window->top = top;
    new_window->left = left;
    new_window->width = width;
    new_window->height = height;
    new_window->right = left + width - 1;
    new_window->bottom = top + height - 1;
    new_window->cur_attrib = attrib;
    new_window->def_attrib = attrib;
    new_window->frame_attrib = frameattrib;
    new_window->line_wrap = TRUE;
    new_window->cursor_shape = BLOCKCURSOR;
    new_window->sound_on = TRUE;
    new_window->attributes = attributes;
    if (attributes & SAVEUNDER)
      new_window->pSaveRectBuff = SaveRect (top, left, height, width);
    else
      new_window->pSaveRectBuff = NULL;
    if (attributes & FRAME)
    {
      DrawBox(top, left, height, width);
      WindowSetTitle (new_window, title);
      new_window->top_margin = top + 1;
      new_window->bottom_margin = new_window->bottom - 1;
      new_window->left_margin = left + 1;
      new_window->right_margin = new_window->right - 1;
    }
    else
    {
      new_window->top_margin = top;
      new_window->bottom_margin = new_window->bottom;
      new_window->left_margin = left;
      new_window->right_margin = new_window->right;
    }
    new_window->margin_height = new_window->bottom_margin - new_window->top_margin + 1;
    new_window->margin_width = new_window->right_margin - new_window->left_margin + 1;
    new_window->cursor_x = new_window->left_margin;
    new_window->cursor_y = new_window->top_margin;
    new_window->pSavedWindow = NULL;
    if (attributes & CLEAR)
      WindowClear (new_window);
    SetPhysicalCursorPos (new_window->cursor_y, new_window->cursor_x);
  }
  return new_window;
}

/******************************************************************************\

\******************************************************************************/

void DestroyWindow (winhdl *window)
{
  if (window->pSaveRectBuff != NULL)
    RestoreRect (window->pSaveRectBuff);
  free (window);
}

/******************************************************************************\

\******************************************************************************/

void WindowSetTitle (winhdl *window, char *text)
{
  char temp_ch = 0;

  window->title = text;
  HChar (window->top, window->left + 2, window->width - 4, 'Í');
  if (*text)
  {
    if (strlen(text) > window->width - 8)
    {
      temp_ch = text[window->width - 8];
      text[window->width - 8] = 0;
    }
    OutTextAt (window->top, window->left + 2, "µ ");
    OutText (text);
    OutText (" Æ");
    if (temp_ch != 0)
      text[window->width - 8] = temp_ch;
  };
}

/******************************************************************************\

\******************************************************************************/

void WindowPrintChar (winhdl *window, int ch)
{
  switch (ch)
  {

    case 0:
      break;

    case 7:
      if (window->sound_on)
	DoNoteOnce (C4, 10);
      break;

    case 8:
      if (window->cursor_x == window->left_margin && window->cursor_y == window->top_margin)
	break;
      window->cursor_x--;
      /**((WORD *)screen_base + window->cursor_y * COLS + window->cursor_x) = window->cur_attrib<<8 | ' ';*/
      break;

    case 9:	/* Tab */
      break;

    case 10:
      window->cursor_y++;
      if (window->cursor_y > window->bottom_margin)
      {
	window->cursor_y--;
	WindowScrollUp (window, 0, 0, window->margin_height - 1, window->margin_width - 1);
      }
      break;

    case 13:
      window->cursor_x = window->left_margin;
      break;

    default:
      *((WORD *)screen_base + window->cursor_y * COLS + window->cursor_x++) = window->cur_attrib<<8 | (BYTE)ch;
      if (window->cursor_x > window->right_margin)
      {
	if (window->line_wrap == TRUE)
	{
	  window->cursor_x = window->left_margin;
	  window->cursor_y++;
	  if (window->cursor_y > window->bottom_margin)
	  {
	    window->cursor_y--;
	    WindowScrollUp (window, 0, 0, window->margin_height - 1, window->margin_width - 1);
	  }
	}
	else
	  window->cursor_x = window->right_margin;
      }
  }
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

/******************************************************************************\

\******************************************************************************/

void WindowPrintString (winhdl *window, char *string)
{
  while (*string)
    WindowPrintChar (window, *string++);
}

/******************************************************************************\

\******************************************************************************/

void WindowScrollUp (winhdl *window, int row, int col, int row2, int col2)
{
  int x, y;
  WORD *source;
  WORD *dest;
  WORD value = window->def_attrib<<8 | ' ';
  row = row + window->top_margin;
  col = col + window->left_margin;
  row2 = row2 + window->top_margin;
  col2 = col2 + window->left_margin;

  for (y = row; y < row2; y++)
  {
    dest = (WORD *)screen_base + y * COLS + col;
    source = dest + COLS;
    for (x = col; x <= col2; x++)
      *dest++ = *source++;
  }

  dest = (WORD *)screen_base + y * COLS + col;
  for (x = col; x <= col2; x++)
    *dest++ = value;
}

/******************************************************************************\

\******************************************************************************/

void WindowScrollDown (winhdl *window, int row, int col, int row2, int col2)
{
  int x, y;
  WORD *source;
  WORD *dest;
  WORD value = window->def_attrib<<8 | ' ';
  row = row + window->top_margin;
  col = col + window->left_margin;
  row2 = row2 + window->top_margin;
  col2 = col2 + window->left_margin;

  for (y = row2; y > row; y--)
  {
    dest = (WORD *)screen_base + y * COLS + col;
    source = dest - COLS;
    for (x = col; x <= col2; x++)
      *dest++ = *source++;
  }

  dest = (WORD *)screen_base + y * COLS + col;
  for (x = col; x <= col2; x++)
    *dest++ = value;
}

/******************************************************************************\

\******************************************************************************/

void WindowEraseToEnd (winhdl *window)
{
  WORD *dest;
  int x;
  int y = window->cursor_y;
  WORD value = window->def_attrib<<8 | ' ';

  dest = (WORD *)screen_base + y * COLS + window->cursor_x;
  for (x = window->cursor_x; x <= window->right_margin; x++)
    *dest++ = value;

  while (y++ < window->bottom_margin)
  {
    dest = (WORD *)screen_base + y * COLS + window->left_margin;
    for (x = window->left_margin; x <= window->right_margin; x++)
      *dest++ = value;
  }
}

/******************************************************************************\

\******************************************************************************/

void WindowEraseToEOL (winhdl *window)
{
  WORD *dest;
  int x;
  WORD value = window->def_attrib<<8 | ' ';

  dest = (WORD *)screen_base + window->cursor_y * COLS + window->cursor_x;
  for (x = window->cursor_x; x <= window->right_margin; x++)
    *dest++ = value;
}

/******************************************************************************\

\******************************************************************************/

void WindowEraseToBOL (winhdl *window)
{
  WORD *dest;
  int x;
  WORD value = window->def_attrib<<8 | ' ';

  dest = (WORD *)screen_base + window->cursor_y * COLS + window->cursor_x;
  for (x = window->cursor_x; x >= window->left_margin; x--)
    *dest-- = value;
}

/******************************************************************************\

\******************************************************************************/

void WindowEraseLine (winhdl *window)
{
  WORD *dest;
  int x;
  WORD value = window->def_attrib<<8 | ' ';

  dest = (WORD *)screen_base + window->cursor_y * COLS + window->left_margin;
  for (x = window->left_margin; x <= window->right_margin; x++)
    *dest++ = value;
}

/******************************************************************************\

\******************************************************************************/

void WindowClear (winhdl *window)
{
  int x, y;
  WORD *dest;
  WORD value = window->def_attrib<<8 | ' ';

  for (y = window->top_margin; y <= window->bottom_margin; y++)
  {
    dest = (WORD *)screen_base + y * COLS + window->left_margin;
    for (x = window->left_margin; x <= window->right_margin; x++)
      *dest++ =value;
  }
}

/******************************************************************************\

\******************************************************************************/

void WindowCursorAt (winhdl *window, int x, int y)
{
  if ((window->left_margin + x > window->right_margin) || (window->top_margin + y > window->bottom_margin))
    return;
  window->cursor_x = window->left_margin + x;
  window->cursor_y = window->top_margin + y;
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

/******************************************************************************\

\******************************************************************************/

void WindowSetCursorRow (winhdl *window, int y)
{
  if (window->top_margin + y > window->bottom_margin)
    return;
  window->cursor_y = window->top_margin + y;
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

/******************************************************************************\

\******************************************************************************/

void WindowSetCursorCol (winhdl *window, int x)
{
  if (window->left_margin + x > window->right_margin)
    return;
  window->cursor_x = window->left_margin + x;
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

/******************************************************************************\

\******************************************************************************/

int WindowGetCursorY (winhdl *window)
{
  return window->cursor_y - window->top_margin;
}

/******************************************************************************\

\******************************************************************************/

int WindowGetCursorX (winhdl *window)
{
  return window->cursor_x - window->left_margin;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowCursorUp (winhdl *window)
{
  if (window->cursor_y > window->top_margin)
  {
    window->cursor_y--;
    SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowCursorDown (winhdl *window)
{
  if (window->cursor_y < window->bottom_margin)
  {
    window->cursor_y++;
    SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowCursorLeft (winhdl *window)
{
  if (window->cursor_x > window->left_margin)
  {
    window->cursor_x--;
    SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowCursorRight (winhdl *window)
{
  if (window->cursor_x < window->right_margin)
  {
    window->cursor_x++;
    SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowGetLineWrap (winhdl *window)
{
  return window->line_wrap;
}

/******************************************************************************\

\******************************************************************************/

void WindowSetLineWrap (winhdl *window, BOOLN new_line_wrap_val)
{
  window->line_wrap = new_line_wrap_val;
}

/******************************************************************************\

\******************************************************************************/

void WindowResetAttribute (winhdl *window, int attrib)
{
  int x, y;
  BYTE *dest;
  BYTE new_attrib = (BYTE)(window->def_attrib = window->cur_attrib = attrib);

  for (y = window->top_margin; y <= window->bottom_margin; y++)
  {
    dest = screen_base + y * 160 + (window->left_margin<<1) + 1;
    for (x = window->left_margin; x <= window->right_margin; x++)
    {
      *dest = new_attrib;
      dest += 2;
    }
  }
}

/******************************************************************************\

\******************************************************************************/

void WindowSetAttribute (winhdl *window, int attrib)
{
  window->cur_attrib = attrib;
}

/******************************************************************************\

\******************************************************************************/

int WindowGetAttribute (winhdl *window)
{
  return window->cur_attrib;
}

/******************************************************************************\

\******************************************************************************/

int WindowGetCharAt (winhdl *window, int x, int y)
{
  return (int)(*(screen_base + (window->top_margin + y) * 160 + ((window->left_margin + x)<<1)));
}

/******************************************************************************\

\******************************************************************************/

void WindowSetCursorShape (winhdl *window, WORD new_curs_shape)
{
  window->cursor_shape = new_curs_shape;
  SetCursorShape (new_curs_shape);
}

/******************************************************************************\

\******************************************************************************/

WORD WindowGetCursorShape (winhdl *window)
{
  return window->cursor_shape;
}

/******************************************************************************\

\******************************************************************************/

void WindowSetSoundOn (winhdl *window, BOOLN sound_setting)
{
  window->sound_on = sound_setting;
}

/******************************************************************************\

\******************************************************************************/

BOOLN WindowGetSoundOn (winhdl *window)
{
  return window->sound_on;
}

/******************************************************************************\

\******************************************************************************/

void WindowSetDefAttrib (winhdl *window, WORD attrib)
{
  window->def_attrib = attrib;
}

/******************************************************************************\

\******************************************************************************/

WORD WindowGetDefAttrib (winhdl *window)
{
  return window->def_attrib;
}

/******************************************************************************\

\******************************************************************************/

void WindowSave (winhdl *window)
{
  window->pSavedWindow = SaveRect (window->top, window->left, window->height, window->width);
}

/******************************************************************************\

\******************************************************************************/

void WindowRestore  (winhdl *window)
{
  RestoreRect (window->pSavedWindow);
  window->pSavedWindow = NULL;
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
  SetCursorShape (window->cursor_shape);
}

/******************************************************************************\

\******************************************************************************/

void WindowSetLeftMargin (winhdl *window, WORD x)
{
  if (window->attributes & FRAME)
  {
    if (x > window->width - 3)
      x = window->width - 3;
    window->left_margin = window->left + x + 1;
    if (window->cursor_x - window->left - 1 < x)
      window->cursor_x = window->left + x + 1;
  }
  else
  {
    if (x > window->width - 1)
      x = window->width - 1;
    window->left_margin = window->left + x;
    if (window->cursor_x - window->left < x)
      window->cursor_x = window->left + x;
  }
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

/******************************************************************************\

\******************************************************************************/

void WindowSetRightMargin (winhdl *window, WORD x)
{
  window->right_margin = x;

  if (window->cursor_x > x)
    window->cursor_x = x;
}

/******************************************************************************\

\******************************************************************************/

void WindowMove (winhdl *window, int y, int x)
{
  WORD *pSaveWindow = NULL;

  if (!(window->attributes & BUFFERED))
    pSaveWindow = SaveRect (window->top, window->left, window->height, window->width);
  if (window->pSaveRectBuff != NULL)
    RestoreRect (window->pSaveRectBuff);
  else
    DrawRect (window->top, window->left, window->height, window->width, ' ');
  window->top += y;
  window->left += x;
  window->cursor_x += x;
  window->cursor_y += y;
  RestoreRectAt (pSaveWindow, window->top, window->left);
  SetPhysicalCursorPos (window->cursor_y, window->cursor_x);
}

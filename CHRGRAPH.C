/******************************************************************************\
User Interface v2.0
by John Chrisman
03-22-97
09-08-98: Made frameattrib and fillattrib global instead of always passing them
to subroutines
\******************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <dos.h>
#include <stdlib.h>

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\video.h>
#include <c:\progproj\c\common\include\chrgraph.h>
#include <c:\progproj\c\common\include\mem.h>

#define COLS 80
#define ROWS 25

/* textattrib holds the current screen attribute to be used when calling any of
the routines which output color along with characters */
static BYTE textattrib;

/* frameattrib holds the current attribute used to draw a frame */
static BYTE frameattrib;

/* fillattrib holds the current attribute used to fill the inside of a frame */
static BYTE fillattrib;

/* chargraphcursor_x holds the current cursor column */
static WORD chargraphcursor_x;

/* chargraphcursor_y holds the current cursor row */
static WORD chargraphcursor_y;

/* chargraphcursor_loc holds a pointer to the location in screen memory where
the next character is to be placed, this should be changed whenever
chargraphcursor_x ot chargraphcursor_y are changed */
static BYTE _far *chargraphcursor_loc;

/* actual_screen_base holds the address of the display cards screen memory */
static BYTE _far *actual_screen_base;

/* Word which defines the shape of the cursor */
static WORD cursor_shape;

/* screen_base holds the address of the current area of memory output should be
sent to. Normally this would be the same as actual_screen_base to output
directly to the screen; however, it can be changed to output to a buffer
using the RedirectScreen routine. */
BYTE _far *screen_base;

extern union REGS inregs, outregs;

/******************************************************************************\

   Routine: InitCharVideo
  
  Function: Initialize the character drawing routines for use -
            Set textattrib to the default white on black
            Set frameattrib to the default yellow on blue
            Set fillattrib to the default white on black
            Place cursor at location 0,0 (upper left hand corner)
            Initialize variables used by all character drawing routines
            Note: It does NOT clear the screen
  
     Pass: Nothing
    
   Return: Nothing
    
\******************************************************************************/

void InitCharVideo (void)
{
  textattrib  = 0x0f;
  frameattrib = 0x1e;
  fillattrib  = 0x0f;
  
  chargraphcursor_x = chargraphcursor_y = 0;
  
  if (GetDisplayType() == MONO)
    actual_screen_base = (BYTE _far *)0xb0000000L;
  else
    actual_screen_base = (BYTE _far *)0xb0008000L;
  
  screen_base = actual_screen_base;
  chargraphcursor_loc = screen_base;
  SetCursorShape (0x0607);
}

/******************************************************************************\

   Routine: SetTextAttrib
  
  Function: Set the attribute used to draw text.
  
      Pass: Attribute
    
    Return: Nothing
    
\******************************************************************************/

void SetTextAttrib (BYTE attrib)
{
  textattrib = attrib;
}

/******************************************************************************\

   Routine: GetTextAttrib
  
  Function: Returns the current attribute used to draw text.
  
      Pass: Nothing
    
    Return: Current text attribute
    
\******************************************************************************/

BYTE GetTextAttrib (void)
{
  return textattrib;
}

/******************************************************************************\

   Routine: SetFrameAttrib
  
  Function: Set the attribute used to draw frames.
  
      Pass: Attribute
    
    Return: Nothing
    
\******************************************************************************/

void SetFrameAttrib (BYTE attrib)
{
  frameattrib = attrib;
}

/******************************************************************************\

   Routine: GetFrameAttrib
  
  Function: Returns the current attribute used to draw frames.
  
      Pass: Nothing
    
    Return: Current frame attribute
    
\******************************************************************************/

BYTE GetFrameAttrib (void)
{
  return frameattrib;
}

/******************************************************************************\

   Routine: SetFillAttrib
  
  Function: Set the attribute used to fill in rectangles.
  
      Pass: Attribute
    
    Return: Nothing
    
\******************************************************************************/

void SetFillAttrib (BYTE attrib)
{
  fillattrib = attrib;
}

/******************************************************************************\

   Routine: GetFillAttrib
  
  Function: Returns the current attribute used to fill in rectangles.
  
      Pass: Nothing
    
    Return: Current fill attribute
    
\******************************************************************************/

BYTE GetFillAttrib (void)
{
  return fillattrib;
}

/******************************************************************************\

   Routine: SetFGColor
  
  Function: Change the foreground color of the attribute used to draw text. Does
            not change the background color of the attribute used to draw text.
            Only the least significant nibble is used to determine the color.
  
      Pass: Color (0-15)
    
    Return: Nothing
    
\******************************************************************************/

void SetFGColor (BYTE color)
{
  textattrib = textattrib & (BYTE)0xf0 | color;
}

/******************************************************************************\

   Routine: SetBKColor
  
  Function: Change the background color of the attribute used to draw text. Does
            not change the foreground color of the attribute used to draw text.
            Only the least significant nibble is used to determine the color.
  
      Pass: Color (0-15)
    
    Return: Nothing
    
\******************************************************************************/

void SetBKColor (BYTE color)
{
  textattrib = textattrib & (BYTE)0x0f | (BYTE)(color << 4);
}

/******************************************************************************\

   Routine: HChar
  
  Function: Draws a horizontal line of characters on the screen without changing
            the attributes at each character location.
  
      Pass: The row to start drawing on (0 - 49)
            The column to start drawing on (0 - 80)
            The number of characters to draw
            The character to be used to draw the line
    
    Return: Nothing
    
\******************************************************************************/

void HChar (WORD row, WORD column, WORD length, BYTE ch)
{
  BYTE _far *position = screen_base + row * (COLS * 2) + (column << 1);
  
  while (length--)
  {
    *position = ch;
    position += 2;
  }
}

/******************************************************************************\

   Routine: HCharC
  
  Function: Draws a horizontal line of characters on the screen using the
            current text attribute.
  
      Pass: The row to start drawing on (0 - 49)
            The column to start drawing on (0 - 80)
            The number of characters to draw
            The character to be used to draw the line
    
    Return: Nothing
    
\******************************************************************************/

void HCharC (WORD row, WORD column, WORD length, BYTE ch)
{
  WORD _far *position = (WORD _far *)(screen_base + row * (COLS * 2) + (column << 1));
  WORD       value    = (textattrib << 8) + ch;
  
  while (length--)
    *position++ = value;
}

/******************************************************************************\

   Routine: VChar
  
  Function: Draws a vertical line of characters on the screen without changing
  the attributes at each character location.
  
      Pass: The row to start drawing on (0 - 49)
            The column to start drawing on (0 - 80)
            The number of characters to draw (0 - 49)
            The character to be used to draw the line
    
    Return: Nothing
    
\******************************************************************************/

void VChar (WORD row, WORD column, WORD length, BYTE ch)
{
  BYTE _far *position = screen_base + row * (COLS * 2) + (column << 1);
  
  while (length--)
  {
    *position = ch;
    position += COLS * 2;
  }
}

/******************************************************************************\

   Routine: VCharC
  
  Function: Draws a vertical line of characters on the screen using the
  current text attribute.
  
      Pass: The row to start drawing on (0 - 49)
            The column to start drawing on (0 - 80)
            The number of characters to draw
            The character to be used to draw the  line
    
    Return: Nothing
    
\******************************************************************************/

void VCharC (WORD row, WORD column, WORD length, BYTE ch)
{
  WORD _far *position = (WORD _far *)(screen_base + row * (COLS * 2) + (column << 1));
  WORD       value    = (textattrib << 8) | ch;
  
  while (length--)
  {
    *position = value;
    position += COLS;
  }
}

/******************************************************************************\

   Routine: HAttrib
  
  Function: Changes the attributes of characters along a horizontal line. Can
            be used to set the attributes of the entire screen by starting at
            row 0, column 0 and changing enough attributes to cover the entire
            screen.
  
      Pass: The row to start changing attributes on (0 - 49)
            The column to start changing attributes on (0 - 80)
            The number of attributes to change
    
    Return: Nothing
    
\******************************************************************************/

void HAttrib (WORD row, WORD column, WORD length)
{
  BYTE _far *position = screen_base + row * (COLS * 2) + (column << 1) + 1;
  
  while (length--)
  {
    *position = textattrib;
    position += 2;
  }
}

/******************************************************************************\

   Routine: CursorAt
  
  Function: Changes the location of the cursor; which is used by some text
            output functions to determine where to draw.Note: It does not
            actually cause the cursor to be displayed.
  
    Pass: The row to place the cursor on (0 - 49)
          The column to place the cursor on (0 - 80)
    
    Return: Nothing
    
\******************************************************************************/

void CursorAt (WORD row, WORD column)
{
  chargraphcursor_x = column;
  chargraphcursor_y = row;
  chargraphcursor_loc = screen_base + row * (COLS * 2) + (column << 1);
}

/******************************************************************************\

   Routine: SetCursorShape
  
  Function: Changes the shape of the cursor.
  
      Pass: A WORD used to determine the shaoe of the cursor.
            The most significant BYTE determines the top line of the cursor.
            The least significant BYTE determines the bottom lone of the cursor.
            The values BLOCKCURSOR, UNDERLINECURSOR, and HIDECURSOR which are
            defined in chrgraph.h can be used.
    
    Return: A WORD which holds the shape of the cursor before being changed
    
\******************************************************************************/

WORD SetCursorShape (WORD shape)
{
  WORD old_shape = cursor_shape;
  
  inregs.h.ah = 0x01;          /* Subfunction */
  inregs.h.ch = (BYTE)(shape >> 8);
  inregs.h.cl = (BYTE)(shape & 0x0f);
  int86 (0x10, &inregs, &outregs);
  cursor_shape = shape;
  return old_shape;
}

/******************************************************************************\

   Routine: SetPhysicalCursorPos
  
  Function: Changes the location of the cursor and displays the cursor at that
            location.
  
      Pass: The row to place the cursor on
            The column to place the cursor on
    
    Return: Nothing
    
\******************************************************************************/

void SetPhysicalCursorPos (WORD row, WORD column)
{
  chargraphcursor_x = column;
  chargraphcursor_y = row;
  chargraphcursor_loc = screen_base + row * COLS + (column << 1);
  inregs.h.ah = 0x02;        /* Subfunction */
  inregs.h.bh = 0x00;        /* Page Number */
  inregs.h.dh = (BYTE)row;    /* row */
  inregs.h.dl = (BYTE)column;    /* column */
  int86 (0x10, &inregs, &outregs);
}

/******************************************************************************\

   Routine: GetPhysicalCursorPos
  
  Function: Returns the current location of the physical cursor.
  
      Pass: Nothing
    
    Return: An s_point structure which holds the row and column of the current
            location of the cursor.(s_point is defined in chrgraph.h)
    
\******************************************************************************/

s_point GetPhysicalCursorPos (void)
{
  s_point pos;
  
  inregs.h.ah = 0x03;
  inregs.h.bh = 0x00;
  int86 (0x10, &inregs, &outregs);
  pos.row = outregs.h.dh;
  pos.col = outregs.h.dl;
  return pos;
}

/******************************************************************************\

   Routine: OutChar
  
  Function: Displays the passed character at the current cursor location and
            moves the cursor to the position immediately after the character.
            It does not change the attribute at that location. 
  
      Pass: The character to display
    
    Return: Nothing
    
\******************************************************************************/

void OutChar (BYTE ch)
{
  *chargraphcursor_loc = ch;
  chargraphcursor_loc += 2;
  if (++chargraphcursor_x > (COLS - 1))
  {
    chargraphcursor_x = 0;
    chargraphcursor_y++;
  }
}

/******************************************************************************\

   Routine: OutCharC
  
  Function: Displays the passed character at the current cursor location using
            the current text attribute and moves the cursor to the position
            immediately after the character.
  
      Pass: The character to display
    
    Return: Nothing
    
\******************************************************************************/

void OutCharC (BYTE ch)
{
  *(chargraphcursor_loc + 1) = textattrib;
  OutChar (ch);
}

/******************************************************************************\

   Routine: OutCharAt
  
  Function: Displays the passed character at the specified location and moves
            the cursor to the position immediatey after the character. It does
            not change the attribute at that location.
  
      Pass: The row to display the character on
            The column to display the cahracter on
            The character to display
    
    Return: Nothing
    
\******************************************************************************/

void OutCharAt (WORD row, WORD column, BYTE ch)
{
  CursorAt (row, column);
  OutChar (ch);
}

/******************************************************************************\

   Routine: OutCharAtC
  
  Function: Displays the passed character at the specified location using the
            current text attribute and moves the cursor to the position
            immediatey after the character.
  
      Pass: The row to display the character on
            The column to display the cahracter on
            The character to display
    
    Return: Nothing
    
\******************************************************************************/

void OutCharAtC (WORD row, WORD column, BYTE ch)
{
  CursorAt (row, column);
  OutCharC (ch);
}

/******************************************************************************\

   Routine: OutText
  
  Function: Outputs a string to the screen at the current cursor location without
            changing the attributes at each character location and moves the
            cursor to the position immediately following the string.
  
      Pass: char * to the null-terminated string to be displayed
    
    Return: Nothing
    
\******************************************************************************/

void OutText (const char *text)
{
  while (*text)
    OutChar (*text++);
}

/******************************************************************************\

   Routine: OutTextC
  
  Function: Outputs a string to the screen at the current cursor location using
            the current text attribute and moves the cursor to the position
            immediately following the string.
  
      Pass: char * to the null-terminated string to be displayed
    
    Return: Nothing
    
\******************************************************************************/

void OutTextC (const char *text)
{
  while (*text)
    OutCharC (*text++);
}

/******************************************************************************\

   Routine: OutTextAt
  
  Function: Outputs a string to the screen at the specified location without
            changing the attributes at each character location and moves the
            cursor to the position immediately following the string.
  
      Pass: char * to the null-terminated string to be displayed
    
    Return: Nothing
    
\******************************************************************************/

void OutTextAt (WORD row, WORD column, const char *text)
{
  CursorAt (row, column);
  while (*text)
    OutChar (*text++);
}

/******************************************************************************\

   Routine: OutTextAtC
  
  Function: Outputs a string to the screen at the specified location using
            the current text attribute and moves the cursor to the position
            immediately following the string.
  
      Pass: char * to the null-terminated string to be displayed
    
    Return: Nothing
    
\******************************************************************************/

void OutTextAtC (WORD row, WORD column, const char *text)
{
  CursorAt (row, column);
  while (*text)
    OutCharC (*text++);
}

/******************************************************************************\

   Routine: OutTextCentered
  
  Function: Outputs a string centered on the screen at the specified row and
            moves the cursor to the position immediately following the string.
  
      Pass: The row to display the string on
            char * to the null-terminated string to be displayed
    
    Return: Nothing
    
\******************************************************************************/

void OutTextCentered (WORD row, const char *text)
{
  CursorAt (row, (COLS/2) - (strlen(text) >> 1));
  while (*text)
    OutChar (*text++);
}

/******************************************************************************\

   Routine: DrawBox
  
  Function: Draws a box (frame) on the screen using the current frame attribute.
            Note: This does not fill in the box with anything. Whatever
            characters are within the frame when it is drawn will not be changed.

      Pass: The row to draw the frame on
            The column to draw the frame on
            The height of the frame
            The width of the frame
    
    Return: Nothing
    
\******************************************************************************/

void DrawBox (WORD row1, WORD column1, WORD height, WORD width)
{
  BYTE oldtextattrib = textattrib;
  int row2 = row1 + height - 1;
  int column2 = column1 + width - 1;
  
  textattrib = frameattrib;
  HCharC (row1, column1, width, (BYTE)'Í');
  HCharC (row2, column1, width, (BYTE)'Í');
  VCharC (row1, column1, height, (BYTE)'º');
  VCharC (row1, column2, height, (BYTE)'º');
  OutCharAtC (row1, column1, (BYTE)'É');
  OutCharAtC (row1, column2, (BYTE)'»');
  OutCharAtC (row2, column1, (BYTE)'È');
  OutCharAtC (row2, column2, (BYTE)'¼');
  
  textattrib = oldtextattrib;
}

/******************************************************************************\

   Routine: DrawRect
  
  Function: Draws a rectangle on the screen using the specified character and the
            current fill attribute. The entire area of the rectange is filled
            with the character.
  
      Pass: The row to draw the rectangle on
            The column to draw the rectangle on
            The height of the rectangle
            The width of the rectangle
            The character used to draw the rectangle

    Return: Nothing
      
\******************************************************************************/

void DrawRect (WORD row1, WORD column1, WORD height, WORD width, BYTE ch)
{
  BYTE oldtextattrib = textattrib;
  
  textattrib = fillattrib;
  while (height--)
    HCharC (row1++, column1, width, ch);
  textattrib = oldtextattrib;
}

/******************************************************************************\

   Routine: DrawBoxFilled
  
  Function: Draws a rectangle on the screen with a frame around it.The frame is
            drawn using the current frame attribute, the interior of the frame
            is drawn using the current fill attribute.
  
      Pass: The row to draw the box on
            The column to draw the box on
            The height of the box
            The width of the box
            The character used to fill the interior of the box
    
    Return: Nothing
    
\******************************************************************************/

void DrawBoxFilled (WORD row1, WORD column1, WORD height, WORD width, BYTE ch)
{
  DrawBox (row1, column1, height, width);
  width -= 2;
  height -= 2;
  column1++;
  row1++;
  DrawRect (row1, column1, height, width, ch);
}

/******************************************************************************\

   Routine: SaveRect
  
  Function: Saves a rectangular area of the screen to a buffer. The returned
            pointer can be manually freed or freed with a call to RestoreRect.
  
      Pass: The row of the rectangle to save
            The column of the rectangle to save
            The height of the rectangle to save
            The width of the rectangle to save
    
    Return: A WORD * to the buffer created to store the contents of the screen.
            This function will return NULL if a buffer can not be allocated.
    
\******************************************************************************/

WORD *SaveRect (WORD row1, WORD column1, WORD height, WORD width)
{
  WORD *pmemlocation;
  
  pmemlocation = malloc ((size_t)(height * width << 1 + 4));
  
  if (pmemlocation != NULL)
  {
    WORD *pcurlocation = pmemlocation;
    WORD _far *pscnlocation = (WORD _far *)(screen_base + row1 * (COLS * 2) + (column1 << 1));
    int left_right_value = COLS - width;
    int x;
    
    *pcurlocation++ = (WORD)((row1 << 8) | column1);
    *pcurlocation++ = (WORD)((height << 8) | width);
    while (height--)
    {
      x = width;
      while (x--)
        *pcurlocation++ = *pscnlocation++;
      pscnlocation += left_right_value;
    }
  }
  return pmemlocation;
}

/******************************************************************************\

   Routine: RestoreRect
  
  Function: Restores a rectangular area of the screen from a buffer created by
            calling SaveRect and frees the buffer.
  
      Pass: WORD * to the buffer created by SaveRect
    
    Return: Nothing
    
\******************************************************************************/

void RestoreRect (WORD *memlocation)
{
  WORD *pcurlocation = memlocation;
  int row1 = *pcurlocation >> 8;
  int column1 = *pcurlocation++ & 0xff;
  int height = *pcurlocation >> 8;
  int width = *pcurlocation++ & 0xff;
  WORD _far *pscnlocation = (WORD _far *)(screen_base + row1 * (COLS * 2) + (column1 << 1));
  int left_right_value = COLS - width;
  int x;
  
  while (height--)
  {
    x = width;
    while (x--)
      *pscnlocation++ = *pcurlocation++;
    pscnlocation += left_right_value;
  }
  free (memlocation);
}

/******************************************************************************\

   Routine: RestoreRectAt
  
  Function: Restores a rectangular area of the screen from a buffer created by
            calling SaveRect and frees the buffer. Instead of restoring the
            rectangle to its original position on the screen, it uses the
            supplied row and column.
  
      Pass: WORD * to the buffer created by RestoreRect
            row to restore rectangle to
            column to restore rectangle to
    
    Return: Nothing
    
\******************************************************************************/

void RestoreRectAt (WORD *memlocation, WORD row, WORD column)
{
  *memlocation = (WORD)((row << 8) | column);
  RestoreRect (memlocation);
}

/******************************************************************************\

   Routine: SaveAndDrawBox
  
  Function: Draws a rectangle on the screen with a frame around it. The frame is
            drawn using the current frame attribute. The interior of the frame is
            drawn using the current fill attribute. The area of the screen which
            is drawn over is saved to a buffer before drawing the new Filled Box.
  
      Pass: The row to draw the box on
            The column to draw the box on
            The height of the box
            The width of the box
            The character used to fill the interior of the box
    
    Return: Nothing
    
\******************************************************************************/

WORD *SaveAndDrawBox (WORD row1, WORD column1, WORD height, WORD width, BYTE ch)
{
  WORD *pmemlocation = SaveRect (row1, column1, height, width);
  
  DrawBoxFilled (row1, column1, height, width, ch);
  return pmemlocation;
}

/******************************************************************************\

   Routine: RedirectScreen
  
  Function: Redirects all screen output to a buffer.
  
      Pass: A BYTE * to an area of memory large enough to hold the entire
            contents of the screen. (4000 words on a screen set at 80 by 25)
    
    Return: Nothing
    
\******************************************************************************/

void RedirectScreen (BYTE _far *new_screen_base)
{
  screen_base = new_screen_base;
}

/******************************************************************************\

   Routine: ResetScreenBase
  
  Function: Sets all screen output back to the screen.
  
      Pass: Nothing
    
    Return: Nothing
    
\******************************************************************************/

void ResetScreenBase (void)
{
  screen_base = actual_screen_base;
}

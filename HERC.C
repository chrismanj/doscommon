#include <c:\progproj\c\common\include\types.h>

#define HERC_BASE 0xb0000000L

int herccursor_x = 0;                               /* cursor X position */
int herccursor_y = 0;                               /* cursor Y position */
byte _far *herccursor_loc = (byte _far *)HERC_BASE; /* Ptr to cursor location within screen memory */

void ClearHercScreen(void)
{
  word _far *dest = (word _far *)HERC_BASE;

  while (dest < (word _far *)(HERC_BASE + 4002))
    *dest++ = 0x0220;
}

void ScrollHercScreenUp(void)
{
  word _far *dest = (word _far *)HERC_BASE;
  word _far *source = (word _far *)HERC_BASE + 80;

  while (source < (word _far *)(HERC_BASE + 4002))
    *dest++ = *source++;

  while (dest < (word _far *)(HERC_BASE + 4002))
    *dest++ = 0x0220;
}

void HercCursorDown(void)
{
  if (herccursor_y++ == 25)
  {
    herccursor_y--;
    ScrollHercScreenUp();
    herccursor_loc = (char _far *)(HERC_BASE + 3680);
  }
  else
    herccursor_loc += 160;
}

void HercCursorAt(int row, int column)
{
  herccursor_x = column;
  herccursor_y = row;
  herccursor_loc = (char _far *)(HERC_BASE + (row << 7) + (row << 5) + (column << 1));
}

void ShowCharHerc(char ch)
{
  *herccursor_loc = ch;

  herccursor_loc += 2; /* Cursor location must be incremented twice because */

  if (++herccursor_x > 79)
  {
    herccursor_x = 0;
    HercCursorDown();
  }
}

void ShowStringHerc(char *string)
{
  while (*string)
    ShowCharHerc(*string++);
}

void ShowStringHercAt(int y, int x, char *string)
{
  HercCursorAt(y, x);
  ShowStringHerc(string);
}

void PrintStringHerc(char *string)
{
  while (*string)
  {
    switch (*string)
    {
    case '\n':
      herccursor_x = 0;
      herccursor_loc = (char _far *)(HERC_BASE + (long)herccursor_y * 160);
      HercCursorDown();
      break;

    case '\r':
      herccursor_x = 0;
      herccursor_loc = (char _far *)(HERC_BASE + (long)herccursor_y * 160);
      break;

    default:
      ShowCharHerc(*string);
    }
    string++;
  }
}

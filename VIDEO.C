#include <dos.h>

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\video.h>

extern union REGS inregs, outregs;

BYTE GetDisplayType (void)
{
  inregs.h.ah = 0x12;			/* Function */
  inregs.h.bl = 0x10;
  inregs.h.bh = 0xff;
  int86 (0x10, &inregs, &outregs);

  if (outregs.h.bh == 0xff)
    return MONO;
  else
    return COLOR;
}

/******************************************************************************\

\******************************************************************************/

void SetVideoMode (int mode)
{
  inregs.h.ah = 0x00;			/* Function */
  inregs.h.al = (BYTE)mode;
  int86 (0x10, &inregs, &outregs);
}

/******************************************************************************\

\******************************************************************************/

void LoadROMFont (int font)
{

  inregs.h.ah = 0x11;			/* Function */
  inregs.h.al = (BYTE)font;                   /* Subfunction */
  inregs.h.bl = 0;
  int86 (0x10, &inregs, &outregs);
}

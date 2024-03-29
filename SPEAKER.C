#include <stddef.h>
#include <conio.h>
#include <dos.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\speaker.h"
#include "..\common\include\jsctime.h"

void SoundOn(void)
{
  outp(SCNTRL, inp(SCNTRL) | SOUNDON);
}

void SoundOff(void)
{
  outp(SCNTRL, inp(SCNTRL) & SOUNDOFF);
}

void SetFrequency(WORD freq)
{
  outp(C8253, SETIMER);
  outp(F8253, freq % 256);
  outp(F8253, (freq / 256));
}

void DoNote(WORD freq, WORD delay)
{
  SetFrequency(freq);
  Pause(delay);
}

void DoNoteOnce(WORD freq, WORD delay)
{
  SoundOn();
  SetFrequency(freq);
  Pause(delay);
  SoundOff();
}

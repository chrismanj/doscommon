#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\bqueue.h"
#include "..\common\include\jscser.h"
#include "..\common\include\jsctime.h"
#include "..\common\include\chrgraph.h"
#include "..\common\include\doublell.h"
#include "..\common\include\intrface.h"
#include "..\common\include\keybrd.h"
#include "..\common\include\modem.h"

extern menu_frame_attrib;
extern menu_text_attrib;

BOOLN DialModem(s_com_port *port, char *number, int count, long timeout, BOOLN silent)
{
  int dial_tries = 1;
  long timer;
  BOOLN done = FALSE;
  BOOLN connected = FALSE;
  WORD *dial_box;
  char temp_str[28];
  WORD key;
  BOOLN abort;
  BOOLN connect_string;

  if (silent == TRUE)
    SendStringToModem(port, "~^MATM0^M~", FALSE);
  else
  {
    dial_box = SaveAndDrawBox(7, 17, 11, 45, ' ');
    OutTextAt(9, 19, "Dialing Number:");
    OutTextAt(9, 35, number);
    OutTextAt(11, 19, "Try No:");
    OutTextAt(11, 32, "Time left this attempt:");
    OutTextAt(13, 19, "Last attempt:");
    OutTextAt(15, 22, "<Press space to skip this attempt.>");
  }

  while (done == FALSE)
  {
    SendStringToModem(port, "^M~ATDT", FALSE);
    SendStringToModem(port, number, FALSE);
    SerialWrite(port, 13);
    Pause(50L);
    FlushPortInputBuffer(port);
    if (silent == FALSE)
      OutTextAt(11, 27, itoa(dial_tries, temp_str, 10));
    abort = FALSE;
    timer = StartTimer();
    while (!CarrierDetected(port) &&
           TimerValue(timer) < timeout &&
           abort == FALSE &&
           !ReadySerial(port))
    {
      if (silent == FALSE)
      {
        OutTextAt(11, 56, ultoa((timeout - TimerValue(timer)) / 100, temp_str, 10));
        OutChar(' ');
      }
      if (KeyInBuffer())
      {
        key = GetAKey();
        if (key == KEY_SPACE)
          abort = TRUE;
        if (key == KEY_ESC)
          abort = done = TRUE;
      }
    }
    if (dial_tries++ == count)
      done = TRUE;
    if (!ReadySerial(port))
    {
      SerialWrite(port, 0x0d);
      Pause(50l);
    }
    if (CarrierDetected(port))
    {
      done = TRUE; /* Connection made */
      connected = TRUE;
    }
    if (abort == FALSE)
    {
      connect_string = FALSE;
      while (connect_string == FALSE && ReadySerial(port))
      {
        temp_str[0] = 0;
        InputSerial(port, temp_str, 25, 200L, FALSE);
        if (strncmp(temp_str, "CONNECT", 7) == 0)
          done = connected = connect_string = TRUE;
        else
        {
          timer = StartTimer();
          while ((TimerValue(timer) < 200L) && !ReadySerial(port))
            ;
        }
        if (silent == FALSE)
        {
          HChar(13, 33, 25, ' ');
          OutTextAt(13, 33, temp_str);
        }
      }
    }
    else if (silent == FALSE)
      OutTextAt(13, 33, "User Interrupted");
  }
  /*Pause(200L);*/
  FlushPortInputBuffer(port);
  if (silent == FALSE)
    RestoreRect(dial_box);
  return connected;
}

/******************************************************************************\

  Routine: SendStringToModem

 Function:

     Pass:

   Return:

\******************************************************************************/

void SendStringToModem(s_com_port *port, char *string, BOOLN echo)
{
  char dec_value[] = {0, 0, 0, 0};
  char ch;
  WORD *p_rectangle;

  if (echo == TRUE)
  {
    p_rectangle = SaveAndDrawBox(11, 0, 4, 80, ' ');
    /*OutTextCentered (12, "PORT ");*/
    CursorAt(13, 2);
  }

  while (*string)
  {
    ch = *string++;
    if (echo == TRUE)
      OutChar(ch);
    switch (ch)
    {
    case '~':
      Pause(100L);
      break;

    case '\\':
      if (*string == '\\')
      {
        SerialWrite(port, '\\');
        string++;
      }
      else if (*string == '^')
      {
        SerialWrite(port, '^');
        string++;
      }
      else if (*string == '~')
      {
        SerialWrite(port, '~');
        string++;
      }
      else
      {
        if (echo == TRUE)
          OutChar(*string);
        dec_value[0] = *(string++);
        if (echo == TRUE)
          OutChar(*string);
        dec_value[1] = *(string++);
        if (echo == TRUE)
          OutChar(*string);
        dec_value[2] = *(string++);
        ch = (BYTE)atoi(&dec_value[0]);
        SerialWrite(port, ch);
      }
      break;

    case '^':
      if (echo == TRUE)
        OutChar(*string);
      SerialWrite(port, (char)(toupper(*string++) - 64));
      break;

    default:
      SerialWrite(port, ch);
    }
  }
  if (echo == TRUE)
    RestoreRect(p_rectangle);
}

int HangUpModem(s_com_port *port, long time)
{
  long timer;
  WORD *mb_rect;

  mb_rect = MessageBox("Dropping DTR");
  DropDTR(port);
  timer = StartTimer();
  while (TimerValue(timer) < time && CarrierDetected(port))
    ;
  RaiseDTR(port);
  RestoreRect(mb_rect);
  if (CarrierDetected(port))
    SendStringToModem(port, "^M~~+++~~ATH^M", TRUE);
  if (CarrierDetected(port))
    return FALSE;
  return TRUE;
}

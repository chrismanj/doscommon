#include <string.h>
#include <stdlib.h>
#include <dos.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\chrgraph.h"

static char *ZeroPadToTwoPlaces(char *string)
{
  if (strlen(string) == 1)
  {
    *(string + 1) = *string;
    *string = '0';
    *(string + 2) = 0;
  }

  return string;
}

char *ConvertTimeToString(long time, char *time_str)
{
  char temp[3];
  char colon_string[2] = {":"};

  /* Copy hours to time_str and append a colon */
  strcat(strcpy(time_str, ZeroPadToTwoPlaces(ltoa(time / 3600, &temp[0], 10))), colon_string);
  /* Concatenate minutes to time_str and append a colon */
  strcat(strcat(time_str, ZeroPadToTwoPlaces(ltoa(time % 3600 / 60, &temp[0], 10))), colon_string);
  /* Concatenate seconds to time_str */
  strcat(time_str, ZeroPadToTwoPlaces(ltoa(time % 60, &temp[0], 10)));

  return time_str;
}

long ConvertDosTimeToLong(struct dostime_t *time)
{
  return time->hour * 360000L + time->minute * 6000L + time->second * 100L + time->hsecond;
}

char *ConvertDOSTimeToString(char *time_str)
{
  struct dostime_t time;

  _dos_gettime(&time);
  return ConvertTimeToString(ConvertDosTimeToLong(&time) / 100, time_str);
}

long StartTimer(void)
{
  struct dostime_t current_time;

  _dos_gettime(&current_time);
  return ConvertDosTimeToLong(&current_time);
}

long TimerValue(long timer_start_value)
{
  struct dostime_t current_time;
  long value;

  _dos_gettime(&current_time);
  value = ConvertDosTimeToLong(&current_time) - timer_start_value;
  if (value < 0)
    value += 8640000L;
  return value;
}

void Pause(long pause_time)
{
  long timer;

  timer = StartTimer();
  while (TimerValue(timer) < pause_time)
    ;
}

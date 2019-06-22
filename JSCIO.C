#include <stdarg.h>
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\jscio.h>
#include <c:\progproj\c\common\include\chrgraph.h>
#include <c:\progproj\c\common\include\keybrd.h>
#include <c:\progproj\c\common\include\mem.h>

int Input (int row, int cursor_col, int width, int max_length, char *p_dest_arg, char *p_default_string,
       char *p_ctl_codes, char *p_picture)
{
  WORD  key_code;
  int   cur_length   = strlen(p_default_string);
  char *p_next_char = p_dest_arg + cur_length;   /* Pointer to where the next character goes in the string */
  int   i_col       = cursor_col;               /* Starting column */
  BOOLN  insert_on   = FALSE;                    /* insert_on mode active flag */
  int   cur_pos;                                /* Current cursor position. 0 = beginning */
  BOOLN  cursor_within_string;                   /* Cursor is or is not at end of input flag */
  int   chars_to_right;                         /* # characters to the right of the cursor */
  char *p_display_string = malloc(width + 1);
  int   beginning = 0;                          /* Starting position of the display string within the overall string. */
  WORD  oldcursor;                              /* Old cursor shape */
  struct PicReturnStr input_is_good;
  s_point old_cursor_pos;                       /* Old cursor position */

  #ifdef DEBUG
    OutDebugText ("DA");
  #endif

  old_cursor_pos = GetPhysicalCursorPos();

  oldcursor = SetCursorShape (HIDECURSOR);
  cursor_col += cur_length;
  if (cursor_col > i_col + width)
  {
    cursor_col = i_col + width;
    beginning += cur_length - width;
  }
  strcpy (p_dest_arg, p_default_string);
  do
  {
    do
    {
      *(p_dest_arg + cur_length) = 0;    /* Add a null to the end of the string */
      cur_pos = p_next_char - p_dest_arg;

      /* Remove all spaces to the right of the cursor or the end of the
   line, whichever is greater. */

      while (*(p_dest_arg + cur_length - 1) == ' ' && cur_length > cur_pos)
  *(p_dest_arg + cur_length--) = 0;

      cursor_within_string = cur_pos < cur_length ? TRUE : FALSE;
      chars_to_right = cur_length - cur_pos;

      memset(p_display_string, 0, width + 1);
      strncpy(p_display_string, p_dest_arg + beginning, width);

      OutTextAt(row, i_col, p_display_string);
      if (cur_length - beginning < width)
        OutCharAt (row, cur_length - beginning + i_col, ' ');

      SetCursorShape (insert_on == TRUE ? UNDERLINECURSOR : BLOCKCURSOR);
      SetPhysicalCursorPos (row, cursor_col);
      key_code = WaitForKeyPress();

      if (IsASCIIKey (key_code) && IsCtrlCode(key_code) == FALSE)
      {
  if ((cur_length < max_length) ||
      (cur_length == max_length && cursor_within_string == TRUE && insert_on == FALSE))
  {
    if ((insert_on == TRUE) && cursor_within_string == TRUE)
      memmove (p_next_char + 1, p_next_char, chars_to_right);
    *p_next_char++ = (char)(key_code & 255);
    cursor_col++;
    if (cursor_col > i_col + width)
    {
      cursor_col--;
      beginning++;
    }
    if ((insert_on == TRUE) || ((insert_on == FALSE) && cur_pos == cur_length))
    cur_length++;
  };
      }
      else
      {
  switch (key_code)
  {
    case KEY_BS:
    if (p_next_char > p_dest_arg)
    {
      if (cursor_within_string == TRUE)
        memmove (p_next_char - 1, p_next_char, chars_to_right);
      p_next_char--;
      if (beginning)
        beginning--;
      else
        cursor_col--;
      cur_length--;
    };
    break;

    case KEY_LEFT:
      if (cur_pos)
      {
        if (beginning)
        {
    if (cursor_col > i_col + width / 10)
      cursor_col--;
    else
      beginning--;
        }
        else
    cursor_col--;
        p_next_char--;
      };
      break;

    case KEY_RIGHT:
      if (cursor_within_string == TRUE)
      {
        if (cursor_col < i_col + width - width / 10)
    cursor_col++;
        else
    beginning++;
        p_next_char++;
      };
      break;

    case KEY_INSERT:
      insert_on = ~insert_on;
      break;

    case KEY_DEL:
    case KEY_KDEL:
      if (cursor_within_string == TRUE)
      {
        memmove (p_next_char, p_next_char + 1, chars_to_right - 1);
        cur_length--;
      };
      break;

    case KEY_HOME:
      cursor_col = i_col;
      p_next_char = p_dest_arg;
      beginning = 0;
      break;

    case KEY_END:
      if (cur_length > width)
      {
        cursor_col = i_col + width;
        beginning = cur_length - width;
      }
      else
        cursor_col = i_col + cur_length;
      p_next_char = p_dest_arg + cur_length;
      break;

    case KEY_CTL_X:
      HChar (row, i_col, width, ' ');
      cur_length = 0;
      p_next_char = p_dest_arg;
      cursor_col = i_col;
      beginning = 0;
      break;

          case KEY_CTL_LEFT:
          {
      BOOLN moved = FALSE;

            while ((*(p_next_char - 1) != ' ' || moved != TRUE) && p_next_char > p_dest_arg)
            {
              if (beginning)
        {
    if (cursor_col > i_col + width / 10)
      cursor_col--;
    else
      beginning--;
          }
        else
    cursor_col--;
        p_next_char--;
              moved = TRUE;
            }
            break;
          }

          case KEY_CTL_RIGHT:
          {
      BOOLN moved = FALSE;

            while ((*(p_next_char - 1) != ' ' || moved != TRUE) && p_next_char < (p_dest_arg + max_length))
      {
        if (cursor_col < i_col + width - width / 10)
    cursor_col++;
        else
    beginning++;
        p_next_char++;
              moved = TRUE;
      }

            break;
          }
  }
      }
    } while (key_code != KEY_ENTER && key_code != KEY_KENTER && key_code != KEY_ESC);
  input_is_good = VerifyStrToPic (p_dest_arg, p_picture);
  } while (input_is_good.MatchWasGood != 4);

  SetCursorShape (oldcursor);
  SetPhysicalCursorPos (old_cursor_pos.row, old_cursor_pos.col);

  free(p_display_string);
  return cur_length;
};


/*******************************************************************************

Purpose: Verify the characters in a string match a picture of allowed characters
   and are in the proper order.

   Pass: pointer to string to check
   pointer to string with picture

 Return: True if the first string fits the format of the p_picture, false if not.

  Notes: The following characters can be used in the picture string -

   ~   - Any character
   %   - One or more of any character. This picture should only be used
         alone or at the end of a picture.
   &   - Alpha (A - Z, a - z)
   !   - One or more alpha characters.
   $   - Alphanumeric (A - Z, a - z, 0 - 9)
   @   - One or more alphanumeric characters.
   ?   - Alphanumeric or punctuation character.
   =   - One or more alphanumeric or punctuation characters.
   #   - Digit (0 - 9)
   ^   - One or more digits.
   *   - Number which must contain between 1 and the number of asterisks
         in digits
   {}  - The enclosed picture is optional but must be in this format if
         present.
   []  - The character must be one of the characters within the bracket.
   ()  - One or more of the characters within the parenthesis.
   /   - The following character is a literal.
   X (where X is any other character) - Literal character.

*******************************************************************************/

struct PicReturnStr VerifyStrToPic (char *p_dest_arg, char *p_picture)
{
  int X = 0;           /* */
  int Y = 0;           /* */
  int input_is_good = 1; /* */
  int DestArgLength = strlen (p_dest_arg);      /* */
  int PictureLength = strlen (p_picture);      /* */
  struct PicReturnStr ReturnVal;               /* */

  #ifdef DEBUG
    OutDebugText ("DB");
  #endif

  while ( ((*(p_picture + Y) != 0) && X < DestArgLength && input_is_good) || (*(p_picture + Y) == '{')) {
    switch (*(p_picture + Y))
    {
      case '~':
  X++;
  break;

      case '%':
  if ((input_is_good = CheckForMultChars (4, (p_dest_arg + X))) != 0)
    X += input_is_good;
  break;

      case '&':
  if ((input_is_good = CheckForChar (0, *(p_dest_arg + X))) != 0)
    X++;
  break;

      case '!':
  if ((input_is_good = CheckForMultChars (0, (p_dest_arg + X))) != 0)
    X += input_is_good;
  break;

      case '$':
  if ((input_is_good = CheckForChar (1, *(p_dest_arg + X))) != 0)
    X++;
  break;

      case '@':
  if ((input_is_good = CheckForMultChars (1, (p_dest_arg + X))) != 0)
    X += input_is_good;
  break;

      case '?':
  if ((input_is_good = CheckForChar (2, *(p_dest_arg + X))) != 0)
    X++;
  break;

      case '=':
  if ((input_is_good = CheckForMultChars (2, (p_dest_arg + X))) != 0)
    X += input_is_good;
  break;

      case '#':
  if ((input_is_good = CheckForChar (3, *(p_dest_arg + X))) != 0)
    X++;
  break;

      case '^':
  if ((input_is_good = CheckForMultChars (3, (p_dest_arg + X))) != 0)
    X += input_is_good;
  break;

      case '*':
      {
  int NumDigitsAllowed = 0;
  int NumDigitsInString = 0;

  while (*(p_picture + Y + NumDigitsAllowed) == '*')
    NumDigitsAllowed++;
  Y += NumDigitsAllowed - 1; /* - 1 because Y is incremented below */

  while (isdigit (*(p_dest_arg + X + NumDigitsInString)))
    NumDigitsInString++;
  X += NumDigitsInString;

  if (NumDigitsInString > NumDigitsAllowed || !NumDigitsInString)
    input_is_good = 0;
      }
  break;

      case '{':
      {
  int NumBraces = 1;
  int NumCharsInPic = -1;
  int Start = Y;
  struct PicReturnStr CompRetVal;
  char *Substr;

  while (NumBraces)
  {
    Y++;
    if (*(p_picture + Y) == '{')
      NumBraces++;
    if (*(p_picture + Y) == '}')
      NumBraces--;
    NumCharsInPic++;
  };

  if ((Substr = (char *) malloc ((size_t)(NumCharsInPic + 1))) != NULL)
  {
    memcpy (Substr, p_picture + Start + 1, NumCharsInPic);
    *(Substr + NumCharsInPic) = 0;
    CompRetVal = VerifyStrToPic (p_dest_arg + X, Substr);
    if (CompRetVal.MatchWasGood < 2)
      input_is_good = 0;
    X += CompRetVal.NumCharsMatched;
    free (Substr);
  }
  else
    input_is_good = 0; /* May need to change this later, out of memory */
      }
  break;

      case '[':
  input_is_good = 0;
  while (*(p_picture + ++Y) != ']')
    if (toupper (*(p_dest_arg + X)) == toupper (*(p_picture + Y)))
      input_is_good = 1;
  if (input_is_good)
    X++;
  break;

      case '(':
      {
  int Start = Y;
  int CharFound = 1;

  input_is_good = 0;
  while (CharFound)
  {
    CharFound = 0;
    Y = Start;
    while (*(p_picture + ++Y) != ')')
      if (toupper (*(p_dest_arg + X)) == toupper (*(p_picture + Y)))
      {
        CharFound = 1;
        input_is_good = 1;
        X++;
      }
  };
      }
  break;

      case '/':
  Y++;

      default:
  if (toupper (*(p_dest_arg + X)) != toupper(*(p_picture + Y)))
    input_is_good = 0;
  if (input_is_good)
    X++;
  break;
    };
    Y++;
  };

  if (!input_is_good)
    ReturnVal.MatchWasGood = 0;

 /*---------------------------------------------------------------------------*\
   We still have some p_picture left, so the string does not match.
 \*---------------------------------------------------------------------------*/

  if (input_is_good && Y < PictureLength)
    ReturnVal.MatchWasGood = 1;

 /*---------------------------------------------------------------------------*\
   The p_picture was run all the way through but there is still some of
   the comparison string left.
 \*---------------------------------------------------------------------------*/

  if (input_is_good && X < DestArgLength)
    ReturnVal.MatchWasGood = 2;

 /*---------------------------------------------------------------------------*\
   Not even one character matched against the p_picture. Note: This will
   override being set to value 1 above.
 \*---------------------------------------------------------------------------*/

  if (!X && PictureLength)
    ReturnVal.MatchWasGood = 3;

 /*---------------------------------------------------------------------------*\
   A perfect match.
 \*---------------------------------------------------------------------------*/

  if ((input_is_good && X == DestArgLength && Y == PictureLength) || !PictureLength)
    ReturnVal.MatchWasGood = 4;

  ReturnVal.NumCharsMatched = X;
  return ReturnVal;
}

int isalnpn (int CharToCheck)
{
  return (isalnum (CharToCheck) || ispunct (CharToCheck));
}

int isanych (int CharToCheck)
{
  return CharToCheck;
}

int CheckForChar (int TypeOfChar , char CharToCheck)
{
  int (*TypeToSearchFor[]) (int) = { isalpha, isalnum, isalnpn, isdigit };

  return ((*TypeToSearchFor[TypeOfChar]) (CharToCheck)) ? 1 : 0;
}

int CheckForMultChars (int TypeOfChar, char *StrToCheck)
{
  int X = 0;
  int (*TypeToSearchFor[]) (int) = { isalpha, isalnum, isalnpn, isdigit,
       isanych};

  while ((*TypeToSearchFor[TypeOfChar]) ((int) *(StrToCheck + X)))
    X++;
  return X;
}

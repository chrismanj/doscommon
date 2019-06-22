#if defined SHOWSCAN || SHOWVALUE
#include <stdio.h>
#endif

#include <conio.h>
#include <dos.h>
#include <ctype.h>
#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\keybrd.h"

#define KEYBOARD_BUFFER_SIZE 32

/******************************************************************************/

union {
  struct s_ext_key_info
  {
    WORD value : 8;
    WORD is_not_ascii : 1;
    WORD is_keypad : 1;
    WORD is_extended : 1;
    WORD shift : 1;
    WORD alt : 1;
    WORD ctrl : 1;
    WORD unused : 2;
  } ext_key_parts;
  WORD ext_key_word;
} ext_key_info;

int scroll_lock;
int num_lock;
int caps_lock;
BYTE keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int kbbuffer_head;
int kbbuffer_tail;
int keycount;
void(_interrupt _far *Old_KB_ISR)(void);
BYTE NoCtrlKeys[] = {27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
           9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,
           'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39,
           '`', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
           ' '};
BYTE ShiftedKeys[] = {27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8,
            9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10,
            'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
            '~', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
            ' '};
BYTE CtrledKeys[] = {32, 33, 34, 35, 36, 37, 30, 39, 40, 41, 42, 31, 43, 44,
           45, 17, 23, 5, 18, 20, 25, 21, 9, 15, 16, 27, 29, 46,
           1, 19, 4, 6, 7, 8, 10, 11, 12, 47, 48,
           49, 28, 26, 24, 3, 22, 2, 14, 13, 50, 51, 52, 53,
           54};

BYTE KeyPad[] = {'*', 71, 72, 73, '-', 75, '5', 77, '+', 79, 80, 81, 82, 127};
BYTE ShiftedKeyPad[] = {'*', '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'};
BYTE CtrledKeyPad[] = {55, 39, 40, 41, 56, 36, 37, 57, 43, 33, 34, 35, 42, 52};

void FlushKeyBuffer(void)
{
  kbbuffer_head = 0;
  kbbuffer_tail = 0;
  keycount = 0;
}

/******************************************************************************/

void InitKeyboard(void);
void _far _interrupt Keyboard_ISR(void);

/******************************************************************************/

void InitKeyboard(void)
{
  scroll_lock = 0;
  num_lock = 2;
  caps_lock = 0;
  outp(0x60, 0xed);
  outp(0x60, 0x02);
  ext_key_info.ext_key_word = 0;
  FlushKeyBuffer();
  Old_KB_ISR = _dos_getvect(0x09);
  _dos_setvect(0x09, Keyboard_ISR);
}

/******************************************************************************/

void DeInitKeyboard(void)
{
  _dos_setvect(0x09, Old_KB_ISR);
}

/******************************************************************************/

void _far _interrupt Keyboard_ISR(void)
{
  int scancode = inp(0x60);
  outp(0x61, inp(0x61) | 0x82 & 0x7f);
  outp(0x20, 0x20);

  if (keycount < KEYBOARD_BUFFER_SIZE)
  {
    keycount++;
    keyboard_buffer[kbbuffer_head++] = (BYTE)scancode;
    kbbuffer_head = kbbuffer_head & KEYBOARD_BUFFER_SIZE - 1;
  }
};

/******************************************************************************/

WORD GetKeyFromBuffer(void)
{
  if (keycount)
  {
    WORD key = keyboard_buffer[kbbuffer_tail++];
    keycount--;
    kbbuffer_tail = kbbuffer_tail & KEYBOARD_BUFFER_SIZE - 1;
    return key;
  }
  else
    return 0;
}

WORD GetKeyFromBufferWithWait(void)
{
  while (!keycount)
    ;
  return GetKeyFromBuffer();
}

WORD NextKeyInBuffer(void)
{
  if (keycount)
    return keyboard_buffer[kbbuffer_tail];
  else
    return 0;
}

int KeyInBuffer(void)
{
  return keycount;
}

/******************************************************************************/

WORD GetAKey(void)
{
  WORD key = GetKeyFromBuffer();
  ext_key_info.ext_key_parts.is_extended = 0;
  ext_key_info.ext_key_parts.is_not_ascii = 0;
  ext_key_info.ext_key_parts.is_keypad = 0;
  if (key)
  {
#ifdef SHOWSCAN
    printf("Value: %u\n", key);
#endif
    if (key == 224) /* Extended keyscan code */
    {
      ext_key_info.ext_key_parts.is_extended = 1;
      ext_key_info.ext_key_parts.is_not_ascii = 1;
      key = GetKeyFromBufferWithWait();
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      if (key < 128)
      {
        if (key == 42) /* Extended extended keyscan code */
        {
          key = GetKeyFromBufferWithWait(); /* This should always return 224 */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
          key = GetKeyFromBufferWithWait(); /* This is the actual key */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
          ext_key_info.ext_key_parts.value = key;
          if (key == 83)
          {
            ext_key_info.ext_key_parts.is_not_ascii = 0;
            ext_key_info.ext_key_parts.value = 127;
          }
#ifdef SHOWVALUE
          printf("Value: %x", ext_key_info.ext_key_word);
#endif
          return ext_key_info.ext_key_word;
        }
        else
        {
          if (key == 29) /* Right CTRL key */
          {
            ext_key_info.ext_key_parts.ctrl = 1;
            return 0;
          }
          else if (key == 56)
          {
            ext_key_info.ext_key_parts.alt = 1; /* Right ALT key */
            return 0;
          }
          else if (key == 28) /* ENTER on keypad */
          {
            ext_key_info.ext_key_parts.is_keypad = 1;
            ext_key_info.ext_key_parts.is_extended = 0;
            ext_key_info.ext_key_parts.is_not_ascii = 0;
            if (ext_key_info.ext_key_parts.alt)
              ext_key_info.ext_key_parts.value = 13;
            else
              ext_key_info.ext_key_parts.value = 13;
#ifdef SHOWVALUE
            printf("Value: %x", ext_key_info.ext_key_word);
#endif
            return ext_key_info.ext_key_word;
          }
          else if (key == 53) /* / on keypad */
          {
            ext_key_info.ext_key_parts.is_keypad = 1;
            ext_key_info.ext_key_parts.is_extended = 0;
            ext_key_info.ext_key_parts.is_not_ascii = 0;
            ext_key_info.ext_key_parts.value = '/';
#ifdef SHOWVALUE
            printf("Value: %x", ext_key_info.ext_key_word);
#endif
            return ext_key_info.ext_key_word;
          }
          else if (key == 70) /* CTRL-BREAK */
            return 0;
          else if (key == 83)
          {
            ext_key_info.ext_key_parts.is_not_ascii = 0;
            ext_key_info.ext_key_parts.value = 127;
            return ext_key_info.ext_key_word;
          }
          else if (key == 93) /* Menu key on Windows 95 keyboards */
          {
            ext_key_info.ext_key_parts.is_extended = 1;
            ext_key_info.ext_key_parts.value = key;
#ifdef SHOWVALUE
            printf("Value: %x", ext_key_info.ext_key_word);
#endif
            return ext_key_info.ext_key_word;
          }
          else
          {
            ext_key_info.ext_key_parts.value = key;
#ifdef SHOWVALUE
            printf("Value: %x", ext_key_info.ext_key_word);
#endif
            return ext_key_info.ext_key_word;
          }
        }
      }
      else
      {
        if (key == 170) /* Extended extended keyscan code */
        {
          key = GetKeyFromBufferWithWait(); /* This should always return 224 */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
          key = GetKeyFromBufferWithWait(); /* This is the actual key */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
          ext_key_info.ext_key_parts.value = key;
#ifdef SHOWVALUE
          printf("Value: %x", ext_key_info.ext_key_word);
#endif
          return ext_key_info.ext_key_word;
        }
        if (key == 157)
          ext_key_info.ext_key_parts.ctrl = 0; /* Right CTRL key released */
        else if (key == 184)
          ext_key_info.ext_key_parts.alt = 0; /* Right ALT key released */
        if (NextKeyInBuffer() == 224)
        {
          key = GetKeyFromBufferWithWait(); /* This should always return 224 */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
          key = GetKeyFromBufferWithWait(); /* This should always return 170 */
#ifdef SHOWSCAN
          printf("Value: %u\n", key);
#endif
        }
        return 0;
      }
    }
    if (key == 225) /* Pause/Break key pressed or released */
    {
      key = GetKeyFromBufferWithWait(); /* This should always return 29 */
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      key = GetKeyFromBufferWithWait(); /* This should always return 69 */
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      key = GetKeyFromBufferWithWait(); /* This should always return 225 */
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      key = GetKeyFromBufferWithWait(); /* This should always return 157 */
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      key = GetKeyFromBufferWithWait(); /* This should always return 197 */
#ifdef SHOWSCAN
      printf("Value: %u\n", key);
#endif
      ext_key_info.ext_key_parts.is_extended = 1;
      ext_key_info.ext_key_parts.is_not_ascii = 1;
      ext_key_info.ext_key_parts.value = 69;
#ifdef SHOWVALUE
      printf("Value: %x", ext_key_info.ext_key_word);
#endif
      return ext_key_info.ext_key_word;
    }
    if (key < 128)
    {
      if (key == 29) /* Left CTRL */
      {
        ext_key_info.ext_key_parts.ctrl = 1;
        return 0;
      }
      if (key == 42) /* Left SHIFT */
      {
        ext_key_info.ext_key_parts.shift = 1;
        return 0;
      }
      if (key == 54) /* Right SHIFT */
      {
        ext_key_info.ext_key_parts.shift = 1;
        return 0;
      }
      if (key == 56) /* Left ALT */
      {
        ext_key_info.ext_key_parts.alt = 1;
        return 0;
      }
      if (key == 58) /* CAPS LOCK */
      {
        ToggleCapsLock();
        return 0;
      }
      if (key == 55 || (key > 70 && key < 84)) /* Numeric keypad */
      {
        if (key > 55)
          key -= 15;
        key -= 55;
        ext_key_info.ext_key_parts.is_keypad = 1;
        if (ext_key_info.ext_key_parts.alt)
        {
          ext_key_info.ext_key_parts.is_not_ascii = 1;
          ext_key_info.ext_key_parts.value = key;
        }
        else if (ext_key_info.ext_key_parts.ctrl)
        {
          ext_key_info.ext_key_parts.value = CtrledKeyPad[key];
          ext_key_info.ext_key_parts.is_not_ascii = 1;
        }
        else if (ext_key_info.ext_key_parts.shift)
        {
          if (num_lock)
          {
            ext_key_info.ext_key_parts.value = KeyPad[key];
            if (key != 0 && key != 4 && key != 6 && key != 8 && key != 13)
              ext_key_info.ext_key_parts.is_not_ascii = 1;
          }
          else
            ext_key_info.ext_key_parts.value = ShiftedKeyPad[key];
        }
        else
        {
          if (num_lock)
            ext_key_info.ext_key_parts.value = ShiftedKeyPad[key];
          else
          {
            ext_key_info.ext_key_parts.value = KeyPad[key];
            if (key != 0 && key != 4 && key != 6 && key != 8 && key != 13)
              ext_key_info.ext_key_parts.is_not_ascii = 1;
          }
        }
#ifdef SHOWVALUE
        printf("Value: %x", ext_key_info.ext_key_word);
#endif
        return ext_key_info.ext_key_word;
      }                            /* end */
      if ((key > 58 && key < 69) || key == 87 || key == 88) /* F1 - F12 */
      {
        ext_key_info.ext_key_parts.is_not_ascii = 1;
        ext_key_info.ext_key_parts.value = key;
#ifdef SHOWVALUE
        printf("Value: %x", ext_key_info.ext_key_word);
#endif
        return ext_key_info.ext_key_word;
      }
      if (key == 69) /* NUMLOCK */
      {
        num_lock = num_lock ^ 2;
        outp(0x60, 0xed);
        outp(0x60, scroll_lock | num_lock | caps_lock);
        *((unsigned char _far *)0x00000417L) = *((unsigned char _far *)0x00000417L) & (unsigned char)0x20;
        return 0;
      }
      if (key == 70) /* SCROLL LOCK */
      {
        scroll_lock = !scroll_lock;
        outp(0x60, 0xed);
        outp(0x60, scroll_lock | num_lock | caps_lock);
        *((unsigned char _far *)0x00000417L) = *((unsigned char _far *)0x00000417L) & (unsigned char)0x10;
        return 0;
      }
      if (key > 70)
        key -= 10;
      else if (key > 58)
        key -= 8;
      else if (key > 56)
        key -= 6;
      else if (key > 42)
        key -= 3;
      else if (key > 29)
        key -= 2;
      else
        key--;
      if (ext_key_info.ext_key_parts.alt)
      {
        ext_key_info.ext_key_parts.is_not_ascii = 1;
        ext_key_info.ext_key_parts.value = key;
      }
      else if (ext_key_info.ext_key_parts.ctrl)
      {
        ext_key_info.ext_key_parts.value = CtrledKeys[key];
        if (ext_key_info.ext_key_parts.value > 31)
          ext_key_info.ext_key_parts.is_not_ascii = 1;
      }
      else if (ext_key_info.ext_key_parts.shift)
      {
        ext_key_info.ext_key_parts.value = ShiftedKeys[key];
        if (caps_lock)
          if (isupper(ext_key_info.ext_key_parts.value))
          {
            if (ext_key_info.ext_key_parts.value > 64 && ext_key_info.ext_key_parts.value < 91)
              ext_key_info.ext_key_parts.value += 32;
          }
          else
          {
            if (ext_key_info.ext_key_parts.value > 96 && ext_key_info.ext_key_parts.value < 123)
              ext_key_info.ext_key_parts.value -= 32;
          }
      }
      else
      {
        ext_key_info.ext_key_parts.value = NoCtrlKeys[key];
        if (caps_lock && ext_key_info.ext_key_parts.value > 96 && ext_key_info.ext_key_parts.value < 123)
          ext_key_info.ext_key_parts.value -= 32;
      }
#ifdef SHOWVALUE
      printf("Value: %x", ext_key_info.ext_key_word);
#endif
      return ext_key_info.ext_key_word;
    }
    else
    {
      if (key == 157) /* Left CTRL released */
        ext_key_info.ext_key_parts.ctrl = 0;
      else if (key == 170) /* Left SHIFT released */
        ext_key_info.ext_key_parts.shift = 0;
      else if (key == 182) /* Right SHIFT released */
        ext_key_info.ext_key_parts.shift = 0;
      else if (key == 184) /* Left ALT released */
        ext_key_info.ext_key_parts.alt = 0;
      return 0;
    }
  }
  else
    return 0;
}

int IsASCIIKey(WORD key)
{
  if (key & 256)
    return 0;
  else
    return 1;
}

int IsNotASCIIKey(WORD key)
{
  if (key & 256)
    return 1;
  else
    return 0;
}

BOOLN IsCtrlCode(WORD key)
{
  BYTE low_val;

  if (key & 256)
    return FALSE;
  low_val = key & 0xff;
  if ((low_val < 32) || (low_val == 127))
    return TRUE;
  else
    return FALSE;
}

WORD WaitForKeyPress(void)
{
  WORD key = 0;
  do
  {
    key = GetAKey();
  } while (key == 0);
  return key;
}

static void SetKBStatusLights(void)
{
  outp(0x60, 0xed);
  outp(0x60, scroll_lock | num_lock | caps_lock);
  *((unsigned char _far *)0x00000417L) = *((unsigned char _far *)0x00000417L) & (unsigned char)0x40;
}

void CapsLockOn(void)
{
  caps_lock = 4;
  SetKBStatusLights();
}

void CapsLockOff(void)
{
  caps_lock = 0;
  SetKBStatusLights();
}

void ToggleCapsLock(void)
{
  caps_lock = caps_lock ^ 4;
  SetKBStatusLights();
}

void NumLockOn(void)
{
  num_lock = 2;
  SetKBStatusLights();
}

void NumLockOff(void)
{
  num_lock = 0;
  SetKBStatusLights();
}

BOOLN IsShifted(WORD key)
{
  if (key & 2048)
    return TRUE;
  else
    return FALSE;
}

BOOLN IsAlted(WORD key)
{
  if (key & 4096)
    return TRUE;
  else
    return FALSE;
}

BOOLN IsCtrled(WORD key)
{
  if (key & 8192)
    return TRUE;
  else
    return FALSE;
}

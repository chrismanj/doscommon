#define FRAME 1L
#define SAVEUNDER 2L
#define CLEAR 4L
#define BUFFERED 8L

#define winhdl struct whandle
struct whandle
{
  char *title;
  WORD def_attrib;
  WORD cur_attrib;
  WORD frame_attrib;
  int top;
  int left;
  int width;
  int height;
  int top_margin;
  int bottom_margin;
  int left_margin;
  int right_margin;
  int margin_height;
  int margin_width;
  int cursor_x;
  int cursor_y;
  int right;
  int bottom;
  BOOLN line_wrap;
  WORD cursor_shape;
  BOOLN sound_on;
  DWORD attributes;
  WORD *pSaveRectBuff;
  WORD *pSavedWindow;
};

winhdl *CreateWindow      (char *, int, int, int, int, WORD, DWORD);
void DestroyWindow 	  (winhdl *);
void WindowClear 	  (winhdl *);
void WindowCursorAt 	  (winhdl *, int, int);
BOOLN WindowCursorUp       (winhdl *);
BOOLN WindowCursorDown     (winhdl *);
BOOLN WindowCursorLeft     (winhdl *);
BOOLN WindowCursorRight    (winhdl *);
void WindowEraseToEnd     (winhdl *);
void WindowEraseToEOL 	  (winhdl *);
void WindowEraseToBOL 	  (winhdl *);
void WindowEraseLine 	  (winhdl *);
int  WindowGetAttribute   (winhdl *);
int  WindowGetCharAt      (winhdl *, int, int);
WORD WindowGetCursorShape (winhdl *);
int  WindowGetCursorY 	  (winhdl *);
int  WindowGetCursorX 	  (winhdl *);
WORD WindowGetDefAttrib   (winhdl *);
BOOLN WindowGetLineWrap    (winhdl *);
BOOLN WindowGetSoundOn     (winhdl *);
void WindowMove           (winhdl *, int, int);
void WindowPrintChar	  (winhdl *, int);
void WindowPrintString    (winhdl *, char *);
void WindowResetAttribute (winhdl *, int);
void WindowRestore  	  (winhdl *);
void WindowScrollUp 	  (winhdl *, int, int, int, int);
void WindowScrollDown	  (winhdl *, int, int, int, int);
void WindowSave 	  (winhdl *);
void WindowSetAttribute   (winhdl *, int);
void WindowSetCursorRow   (winhdl *, int);
void WindowSetCursorCol   (winhdl *, int);
void WindowSetCursorShape (winhdl *, WORD);
void WindowSetDefAttrib   (winhdl *, WORD);
void WindowSetSoundOn     (winhdl *, BOOLN);
void WindowSetLeftMargin  (winhdl *, WORD);
void WindowSetLineWrap    (winhdl *, BOOLN);
void WindowSetRightMargin (winhdl *, WORD);
void WindowSetTitle       (winhdl *, char *);


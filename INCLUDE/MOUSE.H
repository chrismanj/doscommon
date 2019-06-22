#ifndef MOUSE_INC

#define MOUSE_INC 1

#define MOUSE_INT 0x33 /* mouse interrupt number */

/* Defines to make reading buttons easier */

#define MOUSELBUTTON 0x01 /* left button mask */
#define MOUSERBUTTON 0x02 /* right button mask */
#define MOUSECBUTTON 0x04 /* center button mask */

/* Function Prototypes */

void CheckMouseMessages(void);
int ResetMouse(int *);
void ShowMouse(void);
void HideMouse(void);
void GetMouseInfo(short *, short *, int *);
void GetMouseMotionRel(int *, int *);
void SetMouseSensitivity(int, int, int);
void SetMouseCapture(HWND);
#endif

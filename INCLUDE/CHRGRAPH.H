#ifndef CHRGRAPH
	#define CHRGRAPH

	#define BLACK          0
	#define BLUE           1
	#define GREEN          2
	#define CYAN           3
	#define RED            4
	#define MAGENTA        5
	#define BROWN          6
	#define WHITE          7
	#define GRAY           8
	#define LIGHTBLUE      9
	#define LIGHTGREEN     10
	#define LIGHTCYAN      11
	#define LIGHTRED       12
	#define LIGHTMAGENTA   13
	#define YELLOW         14
	#define BRIGHTWHITE    15

	#define BLOCKCURSOR 0x0007
	#define UNDERLINECURSOR  0x0607
	#define HIDECURSOR  0x2000

	#define s_point struct point
	struct point
	{
		WORD row;
		WORD col;
	};

	void      InitCharVideo        (void);
	void      SetTextColor         (int);
	void      HChar                (WORD, WORD, WORD, BYTE);
	void      HCharC               (WORD, WORD, WORD, BYTE);
	void      HAttrib              (WORD, WORD, WORD);
	void      VChar                (WORD, WORD, WORD, BYTE);
	void      VCharC               (WORD, WORD, WORD, BYTE);
	void      CursorAt             (WORD, WORD);
	s_point   GetPhysicalCursorPos (void);
	void      OutChar              (BYTE);
	void      OutCharC             (BYTE);
	void      OutCharAtC           (WORD, WORD, BYTE);
	void      OutText              (const char *);
	void      OutTextC             (const char *);
	void      OutTextAt            (WORD, WORD, const char *);
	void      OutTextAtC           (WORD, WORD, const char *);
	void      SetTextAttrib        (BYTE);
	void      SetFrameAttrib       (BYTE);
	void      SetFillAttrib        (BYTE);
	s_point   GetPhysicalCursorPos (void);
	BYTE      GetTextAttrib        (void);
	BYTE      GetFrameAttrib       (void);
	BYTE      GetFillAttrib        (void);
	void      SetFGColor           (BYTE);
	void      SetBKColor           (BYTE);
	void      DrawBox              (WORD, WORD, WORD, WORD);
	void      DrawBoxFilled        (WORD, WORD, WORD, WORD, BYTE);
	void      DrawRect             (WORD, WORD, WORD, WORD, BYTE);
	void      OutTextCentered      (WORD, const char *);
	WORD      SetCursorShape       (WORD);
	void      SetPhysicalCursorPos (WORD, WORD);
	WORD      *SaveRect            (WORD, WORD, WORD, WORD);
	void      OutCharAt            (WORD, WORD, BYTE);
	void      RestoreRect          (WORD *);
	void      RestoreRectAt        (WORD *, WORD, WORD);
	WORD     *SaveAndDrawBox       (WORD, WORD, WORD, WORD, BYTE);
	void      RedirectScreen       (BYTE _far *);
	void      ResetScreenBase      (void);
#endif

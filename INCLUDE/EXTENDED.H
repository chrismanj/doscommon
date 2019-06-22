#define keyRETURN 13
#define keyBACKSPACE 8
#define keySPACE 32

/* If a function key is pressed, the first byte of the two-byte code will
   return one of the following: */

#define keyINTRO 0      /* For keys common to all keyboards */
#define keyINTRO2 224   /* For keypad and other keys unique to */
			/* 101-key keyboard */

/* Second byte will be one of the following keyended codes: */

/* Numeric keypad */
#define keyUP 72
#define keyDOWN 80
#define keyLEFT 75
#define keyRIGHT 77
#define keyPGUP 73
#define keyPGDN 81
#define keyHOME 71
#define keyEND 79
#define keyINS 82
#define keyDEL 83
#define keyCTRL_PRTSC 114
#define keyCTRL_LEFT 115
#define keyCTRL_RIGHT 116
#define keyCTRL_PGUP 132
#define keyCTRL_PGDN 118
#define keyCTRL_HOME 119
#define keyCTRL_END 117
#define NullKey 3
#define keySH_Tab 15

 /* ALT+letter keys */
#define keyALT_A 30
#define keyALT_B 48
#define keyALT_C 46
#define keyALT_D 32
#define keyALT_E 18
#define keyALT_F 33
#define keyALT_G 34
#define keyALT_H 35
#define keyALT_I 23
#define keyALT_J 36
#define keyALT_K 37
#define keyALT_L 38
#define keyALT_M 50
#define keyALT_N 49
#define keyALT_O 24
#define keyALT_P 25
#define keyALT_Q 16
#define keyALT_R 19
#define keyALT_S 31
#define keyALT_T 20
#define keyALT_U 22
#define keyALT_V 47
#define keyALT_W 17
#define keyALT_X 45
#define keyALT_Y 21
#define keyALT_Z 44

 /* keyALT+number keys */
#define keyALT_1 120
#define keyALT_2 121
#define keyALT_3 122
#define keyALT_4 123
#define keyALT_5 124
#define keyALT_6 125
#define keyALT_7 126
#define keyALT_8 127
#define keyALT_9 128
#define keyALT_0 129

#define keyALT_minus130
#define keyALT_equals

 /* Function keys */
#define keyF1 59
#define keyF2 60
#define keyF3 61
#define keyF4 62
#define keyF5 63
#define keyF6 64
#define keyF7 65
#define keyF8 66
#define keyF9 67
#define keyF10 68
#define keyF11 133
#define keyF12 134

 /* SHIFT+function keys */
#define keySH_F1 84
#define keySH_F2 85
#define keySH_F3 86
#define keySH_F4 87
#define keySH_F5 88
#define keySH_F6 89
#define keySH_F7 90
#define keySH_F8 91
#define keySH_F9 92
#define keySH_F10 93
#define keySH_F11 137
#define keySH_F12  138

 /* CTRL+function keys */
#define keyCTRL_F1 94
#define keyCTRL_F2 95
#define keyCTRL_F3 96
#define keyCTRL_F4 97
#define keyCTRL_F5 98
#define keyCTRL_F6 99
#define keyCTRL_F7 100
#define keyCTRL_F8 101
#define keyCTRL_F9 102
#define keyCTRL_F10 103
#define keyCTRL_F11 137
#define keyCTRL_F12 138

 /* ALT+function keys */
#define keyALT_F1 104
#define keyALT_F2 105
#define keyALT_F3 106
#define keyALT_F4 107
#define keyALT_F5 108
#define keyALT_F6 109
#define keyALT_F7 110
#define keyALT_F8 111
#define keyALT_F9  112
#define keyALT_F10 113
#define keyALT_F11 139
#define keyALT_F12 140



#define COLOR 0
#define MONO 1

#define ROM8BY8  0x12
#define ROM8BY14 0x11
#define ROM8BY16 0x14

BYTE GetDisplayType (void);
void SetVideoMode   (int);
void LoadROMFont    (int);

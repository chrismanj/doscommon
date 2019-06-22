#include <stdio.h>
#include <string.h>
#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\chrgraph.h>

FILE *current_file;

void SetCurrentFile (FILE *file)
{
	#ifdef DEBUG
		OutDebugText ("BA ");
	#endif

	current_file = file;
}

void WriteByteToFile (BYTE byte_to_write)
{
	#ifdef DEBUG
		OutDebugText ("BB ");
	#endif

	fprintf(current_file, "%c", byte_to_write);
}

void WriteStringToFile (char *string_to_write)
{
	#ifdef DEBUG
		OutDebugText ("BC ");
	#endif

	do
		fprintf(current_file, "%c", *string_to_write++);
	while(*(string_to_write - 1));
}

void WriteWordToFile (WORD word_to_write)
{
	#ifdef DEBUG
		OutDebugText ("BD ");
	#endif

	fprintf(current_file, "%c",(word_to_write & 0xff00) >> 8);
	fprintf(current_file, "%c", word_to_write & 0x00ff);
}

void WriteLongToFile (long long_to_write)
{
	fprintf(current_file, "%c",(long_to_write & 0xff000000l) >> 24);
	fprintf(current_file, "%c",(long_to_write & 0x00ff0000l) >> 16);
	fprintf(current_file, "%c",(long_to_write & 0x0000ff00l) >> 8);
	fprintf(current_file, "%c", long_to_write & 0x000000ffl);
}

BYTE ReadByteFromFile (void)
{
	#ifdef DEBUG
		OutDebugText ("BE ");
	#endif

	return (BYTE)fgetc(current_file);
}

void ReadStringFromFile (char *string)
{
	char ch;

	#ifdef DEBUG
		OutDebugText ("BF ");
	#endif

	do
	{
		ch = (char)fgetc(current_file);
		*string++ = ch;
	}
	while (ch);
}

WORD ReadWordFromFile (void)
{
	#ifdef DEBUG
		OutDebugText ("BG ");
	#endif

	return (fgetc(current_file) << 8) | fgetc(current_file);
}

long ReadLongFromFile (void)
{
	return (fgetc(current_file) << 24) | (fgetc(current_file) << 16) | (fgetc(current_file) << 8) | fgetc(current_file);
}

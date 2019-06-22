#include <stdio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>
#include <c:\clang\jscio.h>

enum DataTypes {
   DBTUndefined, DBTInt, DBTDouble, DBTFloat, DBTChar, DBTTime, DBTDate
};

struct TimeStr {
   unsigned char Hour;    /* Hour; 0-23 */
   unsigned char Minute;  /* Minute; 0-59 */
   unsigned char Second;  /* Second; 0-59 */
   unsigned char HSecond; /* 1/100 second; 0-99 */
};

struct DateStr {
   unsigned char Day;        /* Day of the month; 1-31 */
   unsigned char Month;      /* Month of the year; 1-12 */
   unsigned int  Year;       /* Year; 1980 - 2099 */
   unsigned char DayOfWeek;  /* Day of the week, 0-6; Sunday = 0 */
};

struct FieldDStr {
   char             Label [9];
   unsigned char    Datatype;
   unsigned char    Length;
   char             Picture [41];
};

struct FieldStr2 {
   char              Label [9];
   unsigned char     Datatype;
   unsigned char     Length;
   char             *Picture;
   void             *Storage;
   struct FieldStr2 *PrevField;
   struct FieldStr2 *NextField;
};

void EditDBStructure ( char *DBName ) {

   FILE *DBFile;                             /* Database filename */
   struct FieldDStr IOBuff;                  /* Field definition I/O buffer */
   struct FieldStr2 *CurrentFieldPtr = NULL; /* Field currently being edited */
   struct FieldStr2 *FirstFieldPtr = NULL;   /* First field in the list */

   if ( ( FirstFieldPtr = mem_malloc (sizeof(struct FieldStr2)) ) == NULL ) {
   _clearscreen (_GCLEARSCREEN);
   printf ( "Could not allocate memory for variable FirstFieldPtr." );
   GetAChar;
   return;
   };
   CurrentFieldPtr = FirstFieldPtr;

   /* Initialize the first field */

   FirstFieldPtr->Label[0]  = 0;
   FirstFieldPtr->Datatype  = DBTUndefined;
   FirstFieldPtr->Length    = 0;
   FirstFieldPtr->Picture   = NULL;
   FirstFieldPtr->Storage   = NULL;
   FirstFieldPtr->PrevField = NULL;
   FirstFieldPtr->NextField = NULL;

   if ( ( DBFile = fopen (DBName, "wb+") ) == NULL ) {
      _clearscreen (_GCLEARSCREEN);
      printf ( "Could not open file \"%s\".", DBName );
      GetAChar;
      return;
   };

   /* This part of the code will read in the current structure when implemented
   do {
      fread ();
   } while ( !feof ( DBFile ) );
   */

   _clearscreen (_GCLEARSCREEN);
   PrintAt ( 2, 2, "Field    Type   Len Picture" );
   DrawBox ( 1, 1, 80, 48 );
   VLineDblEnd ( 10, 1, 48 );
   VLineDblEnd ( 17, 1, 48 );
   VLineDblEnd ( 21, 1, 48 );
   HLineDblEnd ( 1, 3, 80 );

   fclose ( DBFile );
};

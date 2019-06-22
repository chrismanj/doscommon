static char *ZeroPadToTwoPlaces   (char *);
char        *ConvertTimeToString  (long, char *);
long         ConvertDosTimeToLong (struct dostime_t *);
char        *ConvertDOSTimeToString (char *);
long         StartTimer (void);
long         TimerValue (long);
void         Pause (long);

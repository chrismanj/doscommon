struct PicReturnStr
{
  int MatchWasGood;
  int NumCharsMatched;
};

int                 Input (int, int, int, int, char *, char *, char *, char *);
struct PicReturnStr VerifyStrToPic (char *, char *);
int                 isalnpn (int);
int                 isanych (int CharToCheck);
int                 CheckForChar (int, char);
int                 CheckForMultChars (int, char *);
int                 TypeToSearchFor (int);

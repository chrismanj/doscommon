#define dlinkedlist struct s_dlinked_list

#define BORDER 1
#define NOBORDER 0
#define HORIZONTAL 2
#define VERTICAL 0
#define IS_SUB_MENU 4

#define FUNCTION 0
#define GREYED 1
#define CHECKED 2
#define SUB_MENU 4
#define RUN_FUNC_IMMEDIATELY 8

#define STRING_GADGET 0
#define WORD_GADGET   1
#define CHOICE_GADGET 2
#define ON_OFF_GADGET 3

#define s_menu struct s_mnu
#define s_menuitem struct s_mnu_item

#define s_dialog_box struct s_db
#define s_db_item struct s_dialog_box_item
#define s_string_gadget struct s_str_gad
#define s_choice_gadget struct s_choice_gad
#define s_choice_items struct s_c_i
#define s_on_off_gadget struct s_on_off_gad
#define s_word_gadget struct s_word_gad

struct s_c_i
{
  char *text;
  char hotkey;
};

struct s_mnu
{
  char *title;
  int  row;
  int  col;
  int  height;
  int  width;
  int  flags;
  dlinkedlist *menuitems;
};

struct s_mnu_item
{
  int  row;
  int  col;
  int  flags;
  char *text;
  char  hotkeychar;
  char *shortcut_text;
  void *target;
  void (*function) (void *);
  void *parameter;
};

struct s_db
{
  char *title;
  int  row;
  int  col;
  int  height;
  int  width;
  dlinkedlist *dbitems;
};

struct s_dialog_box_item
{
  int  type;
  char hotkeychar;
  void *gadget;
};

struct s_str_gad
{
  int  desc_row;
  int  desc_col;
  char *description;
  int  data_row;
  int  data_col;
  int  data_width;
  char *data;
  int  length;
};

struct s_choice_gad
{
  int   desc_row;
  int   desc_col;
  char *desc_text;
  int   data_row;
  int   data_col;
  char *menu_title;
  int   numchoices;
  int   menu_row;
  int   menu_col;
  int   menu_height;
  int   menu_width;
  s_choice_items *choices;
  int  *data;
  size_t max_length;
};

struct s_on_off_gad
{
  int   desc_row;
  int   desc_col;
  char *desc_text;
  int   data_row;
  int   data_col;
  BOOLN  *data;
};

struct s_word_gad
{
  int   desc_row;
  int   desc_col;
  char *desc_text;
  int   data_row;
  int   data_col;
  WORD *data;
};

void    SetMenuColors  (BYTE, BYTE);
s_menu *CreateMenu     (char *, int, int, int, int, int);
int     ChoiceMenu     (char *, int, int, int, int, int, s_choice_items *,
			int *);
int     AddMenuItem    (s_menu *, int, int, int, char *, char, char *, void *,
			void (*)(void *), void *);
int     DoMenu         (s_menu *);
void    DestroyMenu    (s_menu *dmenu);
void    ErrorBox       (char *);
void   *ListBox        (char *, WORD, WORD, WORD, WORD, dlinkedlist *, char *(*)(void *));
/*WORD    ListBox        (char *, int, int, int, int, int, void *[], char *(*)(void *));*/
WORD   *MessageBox     (char *text);
void    MessageBoxPause     (char *);
s_dialog_box *CreateDialogBox (char *, int, int, int, int);
int AddDBItem (s_dialog_box *, int, int, ...);
void DoDialogBox (s_dialog_box *);
void DestroyDialogBox (s_dialog_box *);

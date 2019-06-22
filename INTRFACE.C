/******************************************************************************\
            User Interface v2.0
        by John Chrisman
            12-8-97
 09-08-98: Made modifications to color routines because of modifications in
     CHRGRAPH
\******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __BORLANDC__
  #include <ctype.h>
  #include <dos.h>
#endif

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\doublell.h>
#include <c:\progproj\c\common\include\chrgraph.h>
#include <c:\progproj\c\common\include\jscio.h>
#include <c:\progproj\c\common\include\jsctime.h>
#include <c:\progproj\c\common\include\window.h>
#include <c:\progproj\c\common\include\intrface.h>
#include <c:\progproj\c\common\include\keybrd.h>
#include <c:\progproj\c\common\include\speaker.h>
#include <c:\progproj\c\common\include\mem.h>

BYTE menu_hotkeyattrib = 0x2f;
BYTE menu_highlightattrib = 0x35;

void (*TargetFunction) (void *);
void *target_func_param;

/******************************************************************************\

\******************************************************************************/

void SetMenuColors (BYTE highlightattrib, BYTE hotkeyattrib)
{

  menu_hotkeyattrib = hotkeyattrib;
  menu_highlightattrib = highlightattrib;
}

/******************************************************************************\

\******************************************************************************/

WORD HexToWord (char *string)
{
  WORD value = 0;
  WORD x;
  
  while (*string)
  {
    x = (WORD)toupper (*string++);
    value = (value << 4) | (WORD)(x - ((x > 64 && x < 71) ? 55 : 48));
  };
  return value;
}

/******************************************************************************\

\******************************************************************************/

void ShowTitle (int row, int col, char *text)
{

  if (*text)
  {
    OutTextAt (row, col + 2, "µ ");
    OutText (text);
    OutText (" Æ");
  }
}

/******************************************************************************\

\******************************************************************************/

void OutHotKeyText (int row, int col, char *string, char hotkeychar)
{
  BYTE attrib = GetTextAttrib();
  int highlightdone = 0;

  CursorAt (row, col);
  while (*string)
    if (!highlightdone && (*string == hotkeychar))
    {
      SetTextAttrib (menu_hotkeyattrib);
      OutCharC (*string++);
      SetTextAttrib (attrib);
      highlightdone++;
    }
    else
      OutCharC (*string++);
}

/******************************************************************************\

\******************************************************************************/

int ChoiceMenu (char *menu_title, int numchoices, int row, int col,
    int height, int width, s_choice_items *choice_items, int *data)
{
  int counter;
  int oldchoice;
  int menuchoice;
  WORD *pSaveRectBuff;
  int done = 0;
  WORD key;
  s_point old_cursor_pos;
  WORD oldcursor;
  BYTE menu_textattrib = GetFillAttrib();

  oldchoice = menuchoice = *data;
  pSaveRectBuff = SaveRect (row, col, height, width);
  
  if (pSaveRectBuff != NULL)
  {
    old_cursor_pos = GetPhysicalCursorPos();
    
    oldcursor = SetCursorShape (0x2000);
    DrawBoxFilled (row, col, height, width, ' ');
    ShowTitle (row, col, menu_title);
    row += 2;
    col += 2;
    
    SetTextAttrib (menu_textattrib);
    for (counter = 0; counter < numchoices; counter++)
      OutHotKeyText (row + counter, col, choice_items[counter].text,
      choice_items[counter].hotkey);
    
    while (!done)
    {
      OutHotKeyText (row + oldchoice, col, choice_items[oldchoice].text,
        choice_items[oldchoice].hotkey);
      oldchoice = menuchoice;
      SetTextAttrib (menu_highlightattrib);
      OutHotKeyText (row + menuchoice, col, choice_items[menuchoice].text,
        choice_items[menuchoice].hotkey);
      SetTextAttrib (menu_textattrib);
      key = WaitForKeyPress();
      switch (key)
      {
      case KEY_UP:
      case KEY_SH_TAB:
        if (menuchoice == 0)
          menuchoice = numchoices;
        menuchoice--;
        break;
        
      case KEY_DN:
      case KEY_TAB:
        if (menuchoice == numchoices - 1)
          menuchoice = -1;
        menuchoice++;
        break;
        
      case KEY_ESC:
        done = -1;
        break;
        
      case KEY_ENTER:
      case KEY_SPACE:
        done = 1;
        break;
        
      default:
        if (IsASCIIKey(key))
        {
          for (counter = 0; counter < numchoices; counter++)
            if (toupper(key & 0xff) == toupper(choice_items[counter].hotkey))
            {
              menuchoice = counter;
              done = 1;
            }
        }
      }
    }
    RestoreRect (pSaveRectBuff);
    
    SetCursorShape (oldcursor);
    SetPhysicalCursorPos (old_cursor_pos.row, old_cursor_pos.col);
  }
  if (done >= 0)
  {
    *data = menuchoice;
    return menuchoice;
  }
  else
    return -1;
}

/******************************************************************************\

\******************************************************************************/

void *ListBox (char *title, WORD row, WORD col, WORD height, WORD width,
         dlinkedlist *items, char *(*display)(void *))
{
  WORD counter;
  void *curr_item;
  BOOLN done = FALSE;
  WORD key;
  WORD line;
  dll_position top;
  dll_position choice_pos;
  void *choice;
  s_point old_cursor_pos;
  WORD oldcursor;
  WORD menu_textattrib = GetFillAttrib();
  winhdl *list_box_window;

  old_cursor_pos = GetPhysicalCursorPos();
  oldcursor = SetCursorShape (0x2000);
  list_box_window = CreateWindow (title, row, col, height, width, menu_textattrib, FRAME | CLEAR | SAVEUNDER);
  if (list_box_window != NULL)
  {
    WindowSetLineWrap (list_box_window, FALSE);
    height -= 3;
    if((choice = DLLGetFirstItem (items)) != NULL)
    {
      DLLGetCurrentPosition (items, &top);
      while (done == FALSE)
      {
        DLLGetCurrentPosition (items, &choice_pos);
        curr_item = DLLSetCurrentPosition (items, &top);
        WindowCursorAt (list_box_window, 0, 0);
        for (counter = 0; counter <= height && curr_item != NULL; counter++)
        {
          if (curr_item == choice)
          {
            WindowSetAttribute (list_box_window, menu_highlightattrib);
            line = counter;
          }
          else
            WindowSetAttribute (list_box_window, menu_textattrib);
          WindowPrintString (list_box_window, (*display)(curr_item));
          WindowEraseToEOL (list_box_window);
          if (counter < height)
            WindowPrintString (list_box_window, "\n\r");
          curr_item = DLLGetNextItem(items);
        }
        DLLSetCurrentPosition (items, &choice_pos);
        key = WaitForKeyPress();
        switch (key)
        {
        case KEY_UP:
        case KEY_SH_TAB:
          if (DLLGetItemNum(items) > 1)
          {
            choice = DLLGetPrevItem (items);
            if (line == 0)
              DLLGetElementBefore (&top, &top);
          }
          break;
          
        case KEY_DN:
        case KEY_TAB:
          {
            if(DLLGetItemNum(items) < DLLGetNoOfItems(items))
            {
              choice = DLLGetNextItem (items);
              if (line >= height)
                DLLGetElementAfter (&top, &top);
            }
          }
          break;
          
        case KEY_ESC:
          done = TRUE;
          choice = NULL;
          break;
          
        case KEY_ENTER:
        case KEY_SPACE:
          done = TRUE;
          break;
        }
      }
    }
    DestroyWindow (list_box_window);
  }
  SetCursorShape (oldcursor);
  SetPhysicalCursorPos (old_cursor_pos.row, old_cursor_pos.col);
  return choice;
}

/******************************************************************************\

\******************************************************************************/

s_menu *CreateMenu (char *mtitle, int mrow, int mcol, int mheight,
        int mwidth, int mflags)
{
  s_menu *menu = malloc (sizeof(s_menu));

  if (menu != NULL)
  {
    menu->title = mtitle;
    menu->row = mrow;
    menu->col = mcol;
    menu->height = mheight;
    menu->width = mwidth;
    menu->menuitems = DLLCreate ();
    menu->flags = mflags;
    if (menu->menuitems == NULL)
    {
      free (menu);
      menu = NULL;
    }
  }
  return menu;
}

/******************************************************************************\

\******************************************************************************/

int AddMenuItem (s_menu *menu, int row, int col, int flags, char *text,
     char hotkeychar, char *shortcut_text, void *target,
     void (*function) (void *), void *parameter)
{
  s_menuitem *menuitem = malloc (sizeof(s_menuitem));

  if (menuitem != NULL)
  {
    menuitem->row = row;
    menuitem->col = col;
    menuitem->flags = flags;
    menuitem->text = text;
    menuitem->hotkeychar = hotkeychar;
    menuitem->shortcut_text = shortcut_text;
    menuitem->target = target;
    menuitem->function = function;
    menuitem->parameter = parameter;
    DLLAddItem (menu->menuitems, menuitem);
    return 1;
  }
  return 0;
};

/******************************************************************************\

\******************************************************************************/

void ShowMenuItem (s_menu *menu, s_menuitem *item)
{

  OutHotKeyText (item->row + menu->row, item->col + menu->col, item->text, item->hotkeychar);
  if ((item->flags & SUB_MENU) && !(menu->flags & HORIZONTAL))
    OutCharAtC (item->row + menu->row, menu->col + menu->width - 2, '');
  if (item->shortcut_text != 0)
    OutTextAtC (item->row + menu->row, menu->col + menu->width - 2 - strlen(item->shortcut_text), item->shortcut_text);
  if (item->flags & CHECKED)
   if(*((BOOLN *)item->target) == TRUE)
     OutCharAt (item->row + menu->row, item->col + menu->col - 1, (BYTE)'û');
   else
     OutCharAt (item->row + menu->row, item->col + menu->col - 1, ' ');
}

/******************************************************************************\

\******************************************************************************/

int DoMenuItem (s_menuitem *item)
{
  int something_was_selected;

  if (item->flags & CHECKED)
  {
    *((int *)item->target) = ~*((int *)item->target);
    something_was_selected = 1;
  }
  if (item->flags & SUB_MENU)
    something_was_selected = DoMenu (item->target);
  if (item->function != NULL)
  {
    TargetFunction = item->function;
    target_func_param = item->parameter;
    something_was_selected = 2;
  }
  return something_was_selected;
}

/******************************************************************************\

\******************************************************************************/

int DoMenu (s_menu *menu)
{
  WORD *prectbuff;
  int something_was_selected = 0;
  WORD key;
  s_menuitem *curmenuitem;
  s_menuitem *prevmenuitem;
  s_menuitem *curscanmenuitem;
  WORD oldcursor;
  BYTE menu_textattrib = GetFillAttrib();

  /* Save area under menu, draw border if needed */

  oldcursor = SetCursorShape (0x2000);
  if (menu->flags & BORDER)
  {
    prectbuff = SaveAndDrawBox (menu->row, menu->col, menu->height, menu->width, ' ');
    ShowTitle (menu->row, menu->col, menu->title);
  }
  else
  {
    prectbuff = SaveRect (menu->row, menu->col, menu->height, menu->width);
    DrawRect (menu->row, menu->col, menu->height, menu->width, ' ');
  }

  /* Show menu items */
  SetTextAttrib (menu_textattrib);
  curmenuitem = DLLGetFirstItem (menu->menuitems);
  while (curmenuitem != NULL)
  {
    ShowMenuItem (menu, curmenuitem);
    curmenuitem = DLLGetNextItem (menu->menuitems);
  }
  curmenuitem = DLLGetFirstItem (menu->menuitems);
  prevmenuitem = curmenuitem;

  /* Allow user to make a choice from the menus */
  while (!something_was_selected)
  {
    OutHotKeyText (prevmenuitem->row + menu->row, prevmenuitem->col + menu->col, prevmenuitem->text, prevmenuitem->hotkeychar);
    prevmenuitem = curmenuitem;
    SetTextAttrib (menu_highlightattrib);
    OutHotKeyText (curmenuitem->row + menu->row, curmenuitem->col + menu->col, curmenuitem->text, curmenuitem->hotkeychar);
    SetTextAttrib (menu_textattrib);
    key = WaitForKeyPress();
    if (IsASCIIKey (key))
    {
      switch (key)
      {
        case KEY_ESC:
          something_was_selected = 1;
          break;

        case KEY_ENTER:
        case KEY_SPACE:
          something_was_selected = DoMenuItem (curmenuitem);
          break;

        default:
          curscanmenuitem = DLLGetFirstItem (menu->menuitems);
          while (curscanmenuitem != NULL)
          {
            if (toupper(key & 255) == toupper(curscanmenuitem->hotkeychar))
            {
              curmenuitem = curscanmenuitem;
              OutHotKeyText (prevmenuitem->row + menu->row, prevmenuitem->col + menu->col, prevmenuitem->text, prevmenuitem->hotkeychar);
              prevmenuitem = curmenuitem;
              SetTextAttrib (menu_highlightattrib);
              OutHotKeyText (curmenuitem->row + menu->row, curmenuitem->col + menu->col, curmenuitem->text, curmenuitem->hotkeychar);
              SetTextAttrib (menu_textattrib);
              something_was_selected = DoMenuItem (curmenuitem);
              break;
            }
            curscanmenuitem = DLLGetNextItem (menu->menuitems);
          }
      }
    }
    else
    {
      switch (menu->flags & HORIZONTAL)
      {
        case VERTICAL:
          switch (key)
          {
            case KEY_UP:
              curmenuitem = DLLCGetPrevItem (menu->menuitems);
              break;

            case KEY_DN:
              curmenuitem = DLLCGetNextItem (menu->menuitems);
              break;

            case KEY_LEFT:
            case KEY_RIGHT:
              if (curmenuitem->flags & SUB_MENU)
                something_was_selected = DoMenu (curmenuitem->target);
              else if (menu->flags & IS_SUB_MENU)
                something_was_selected = -1;
              break;
          }
          break;

        case HORIZONTAL:
          switch (key)
          {
            case KEY_LEFT:
              curmenuitem = DLLCGetPrevItem (menu->menuitems);
              break;

            case KEY_RIGHT:
              curmenuitem = DLLCGetNextItem (menu->menuitems);
              break;

            case KEY_UP:
            case KEY_DN:
              if (curmenuitem->flags & SUB_MENU)
                something_was_selected = DoMenu (curmenuitem->target);
              else if (menu->flags & IS_SUB_MENU)
                something_was_selected = -1;
              break;
          }
      }
    }
  }
  RestoreRect (prectbuff);
  if (!(menu->flags & IS_SUB_MENU))
  {
    DestroyMenu (menu);
    if (something_was_selected == 2)
      (*TargetFunction) (target_func_param);
  }
  if (something_was_selected == -1)
    something_was_selected++;
  SetCursorShape (oldcursor);
  return something_was_selected;
}

/******************************************************************************\

\******************************************************************************/

void DestroyMenu (s_menu *menu)
{
  s_menuitem *curmenuitem = DLLGetFirstItem (menu->menuitems);

  while (curmenuitem != NULL)
  {
    if (curmenuitem->flags & SUB_MENU)
      DestroyMenu (curmenuitem->target);

    free (curmenuitem);
    curmenuitem = DLLGetNextItem (menu->menuitems);
  }
  DLLDestroy (menu->menuitems);

  free (menu);
}

/******************************************************************************\

\******************************************************************************/

WORD *MessageBox (char *text)
{
  int col = 38 - strlen (text) / 2;
  int width = strlen (text) + 4;
  WORD *prectbuff = SaveAndDrawBox (10, col, 5, width, ' ');

  OutTextAtC (12, col + 2, text);

  return prectbuff;
}


/******************************************************************************\

\******************************************************************************/

void MessageBoxPause (char *text)
{
  WORD *mb_rect;

  mb_rect = MessageBox(text);
  Pause (75L);
  RestoreRect (mb_rect);
}

/******************************************************************************\

\******************************************************************************/

void ErrorBox (char *text)
{
  WORD col;
  WORD width;
  WORD *prectbuff;

  if (strlen(text) >= 28)
  {
    col = 38 - strlen (text) / 2;
    width = strlen (text) + 4;
  }
  else
  {
    col = 24;
    width = 32;
  }

  prectbuff = SaveAndDrawBox (10, col, 6, width, ' ');
  OutTextAtC (12, col + 2, text);
  OutTextAtC (13, 26, "<Press any key to continue>.");
  DoNoteOnce (C4, 25);
  Pause (25L);
  FlushKeyBuffer();
  while (GetAKey() == 0);
  RestoreRect (prectbuff);
}

/******************************************************************************\

\******************************************************************************/

s_dialog_box *CreateDialogBox (char *dbtitle, int dbrow, int dbcol,
             int dbheight, int dbwidth)
{
  s_dialog_box *db;

  db = malloc (sizeof(s_dialog_box));
  if (db != NULL)
  {
    db->title = dbtitle;
    db->row = dbrow;
    db->col = dbcol;
    db->height = dbheight;
    db->width = dbwidth;
    db->dbitems = DLLCreate ();
    if (db->dbitems == NULL)
    {
      free (db);
      db = NULL;
    }
  }
  return db;
}

/******************************************************************************\

\******************************************************************************/

int AddDBItem (s_dialog_box *db, int type, int hotkeychar, ...)
{
  void *gadget;
  va_list marker;
  s_db_item *dbitem;

  dbitem = malloc (sizeof(s_db_item));
  if (dbitem != NULL)
  {
    dbitem->type = type;
    dbitem->hotkeychar = (char)hotkeychar;
    va_start (marker, hotkeychar);
    switch (type)
    {
      case STRING_GADGET:
        gadget = malloc (sizeof(s_string_gadget));
        if (gadget != NULL)
        {
          s_string_gadget *str_gad = (s_string_gadget *)gadget;
          str_gad->desc_row = va_arg (marker, int);
          str_gad->desc_col = va_arg (marker, int);
          str_gad->description = va_arg (marker, char *);
          str_gad->data_row = va_arg (marker, int);
          str_gad->data_col = va_arg (marker, int);
          str_gad->data_width = va_arg (marker, int);
          str_gad->data = va_arg (marker, char *);
          str_gad->length = va_arg (marker, int);
        }
        break;

      case CHOICE_GADGET:
        gadget = malloc (sizeof(s_choice_gadget));
        if (gadget != NULL)
        {
          int x;

          s_choice_gadget *choice_gad = (s_choice_gadget *)gadget;
          choice_gad->desc_row = va_arg (marker, int);
          choice_gad->desc_col = va_arg (marker, int);
          choice_gad->desc_text = va_arg (marker, char *);
          choice_gad->data_row = va_arg (marker, int);
          choice_gad->data_col = va_arg (marker, int);
          choice_gad->menu_title = va_arg (marker, char *);
          choice_gad->numchoices = va_arg (marker, int);
          choice_gad->menu_row = va_arg (marker, int);
          choice_gad->menu_col = va_arg (marker, int);
          choice_gad->menu_height = va_arg (marker, int);
          choice_gad->menu_width = va_arg (marker, int);
          choice_gad->choices = va_arg (marker, s_choice_items *);
          choice_gad->data = va_arg (marker, int *);
          choice_gad->max_length = 0;
          for (x = 0; x < choice_gad->numchoices; x++)
            if (strlen(choice_gad->choices[x].text) > choice_gad->max_length)
              choice_gad->max_length = strlen(choice_gad->choices[x].text);
        }
        break;

      case ON_OFF_GADGET:
        gadget = malloc (sizeof(s_on_off_gadget));
        if (gadget != NULL)
        {
          s_on_off_gadget *on_off_gad = (s_on_off_gadget *)gadget;
          on_off_gad->desc_row = va_arg (marker, int);
          on_off_gad->desc_col = va_arg (marker, int);
          on_off_gad->desc_text = va_arg (marker, char *);
          on_off_gad->data_row = va_arg (marker, int);
          on_off_gad->data_col = va_arg (marker, int);
          on_off_gad->data = va_arg (marker, BOOLN *);
        }
        break;

      case WORD_GADGET:
      {
        gadget = malloc (sizeof(s_word_gadget));
        if (gadget != NULL)
        {
          s_word_gadget *word_gad = (s_word_gadget *)gadget;
          word_gad->desc_row = va_arg (marker, int);
          word_gad->desc_col = va_arg (marker, int);
          word_gad->desc_text = va_arg (marker, char *);
          word_gad->data_row = va_arg (marker, int);
          word_gad->data_col = va_arg (marker, int);
          word_gad->data = va_arg (marker, WORD *);
        }
      }
    }
    va_end(marker);
    if (gadget == NULL)
      return 0;
    dbitem->gadget = gadget;
    DLLAddItem (db->dbitems, dbitem);
    return 1;
  }
  return 0;
};

/******************************************************************************\

\******************************************************************************/

void DestroyDialogBox (s_dialog_box *db)
{
  s_db_item *cur_db_item;

  cur_db_item = DLLGetFirstItem (db->dbitems);
  while (cur_db_item != NULL)
  {
    free (cur_db_item->gadget);
    free (cur_db_item);
    cur_db_item = DLLGetNextItem (db->dbitems);
  }
  DLLDestroy (db->dbitems);

  free (db);
}

/******************************************************************************\

\******************************************************************************/

void ShowGadgetData (s_dialog_box *db, s_db_item *item)
{

  switch (item->type)
  {
    case STRING_GADGET:
      s_string_gadget *str_gad = (s_string_gadget *)item->gadget;

      char *temp_string_space = malloc ((size_t)(str_gad->data_width + 1));
      *(temp_string_space + str_gad->data_width) = 0;
      strncpy(temp_string_space, str_gad->data, (size_t)str_gad->data_width);
      HAttrib (db->row + str_gad->data_row,
         db->col + str_gad->data_col, str_gad->data_width);
      OutTextAt (db->row + str_gad->data_row, db->col + str_gad->data_col, temp_string_space);

      free(temp_string_space);
      break;

    case CHOICE_GADGET:
      s_choice_gadget *choice_gad = (s_choice_gadget *)item->gadget;
      HCharC (db->row + choice_gad->data_row,
        db->col + choice_gad->data_col,
        choice_gad->max_length, ' ');
      OutTextAtC (db->row + choice_gad->data_row,
      db->col + choice_gad->data_col,
      choice_gad->choices[*choice_gad->data].text);
      break;

    case ON_OFF_GADGET:
      s_on_off_gadget *on_off_gad = (s_on_off_gadget *)item->gadget;
      OutTextAtC (db->row + on_off_gad->data_row,
      db->col + on_off_gad->data_col,
      *on_off_gad->data == TRUE ? "On " : "Off");
      break;

    case WORD_GADGET:
      char work_string[6];
      s_word_gadget *w_gad = (s_word_gadget *)item->gadget;
      HCharC (db->row + w_gad->data_row,
        db->col + w_gad->data_col, 4, ' ');

      OutTextAtC (db->row + w_gad->data_row, db->col + w_gad->data_col, ultoa(*w_gad->data, work_string, 16));
      break;

  }
}

/******************************************************************************\

\******************************************************************************/

void ShowGadget (s_dialog_box *db, s_db_item *item)
{

  switch (item->type)
  {
    case STRING_GADGET:
       s_string_gadget *str_gad = (s_string_gadget *)item->gadget;
      OutHotKeyText (db->row + str_gad->desc_row,
         db->col + str_gad->desc_col,
         str_gad->description, item->hotkeychar);
      ShowGadgetData (db, item);
       break;

    case CHOICE_GADGET:
      s_choice_gadget *choice_gad = (s_choice_gadget *)item->gadget;
      OutHotKeyText (db->row + choice_gad->desc_row,
         db->col + choice_gad->desc_col,
         choice_gad->desc_text, item->hotkeychar);
      ShowGadgetData (db, item);
       break;

    case ON_OFF_GADGET:
      s_on_off_gadget *on_off_gad = (s_on_off_gadget *)item->gadget;
      OutHotKeyText (db->row + on_off_gad->desc_row,
        db->col + on_off_gad->desc_col,
        on_off_gad->desc_text, item->hotkeychar);
      ShowGadgetData (db, item);
      break;

    case WORD_GADGET:
      s_word_gadget *word_gad = (s_word_gadget *)item->gadget;
      OutHotKeyText (db->row + word_gad->desc_row,
        db->col + word_gad->desc_col,
        word_gad->desc_text, item->hotkeychar);

      ShowGadgetData (db, item);
   }
}

/******************************************************************************\

\******************************************************************************/

void EditGadget (s_dialog_box *db, s_db_item *item)
{

  switch (item->type)
  {
    case STRING_GADGET:
      s_string_gadget *str_gad = (s_string_gadget *)item->gadget;
      Input (db->row + str_gad->data_row,
        db->col + str_gad->data_col, str_gad->data_width,
        str_gad->length, str_gad->data, str_gad->data, "", "");

      SetCursorShape (0x2000);
      break;

    case CHOICE_GADGET:
      s_choice_gadget *choice_gad = (s_choice_gadget *)item->gadget;
      ChoiceMenu (choice_gad->menu_title, choice_gad->numchoices,
      choice_gad->menu_row, choice_gad->menu_col,
      choice_gad->menu_height, choice_gad->menu_width,
      choice_gad->choices, choice_gad->data);
      break;

    case ON_OFF_GADGET:
      s_on_off_gadget *on_off_gad = (s_on_off_gadget *)item->gadget;
      *on_off_gad->data = ~*on_off_gad->data;
      break;

    case WORD_GADGET:
      s_word_gadget *w_gad = (s_word_gadget *)item->gadget;
      char work_string[6];

      ultoa(*w_gad->data, work_string, 16);

      Input (db->row + w_gad->data_row, db->col + w_gad->data_col, 4, 4, work_string, work_string, "", "");
      *w_gad->data = HexToWord(work_string);
      break;
  }
}

/******************************************************************************\

\******************************************************************************/

void DoDialogBox (s_dialog_box *db)
{
  s_db_item *old_db_item;
  s_db_item *cur_db_item;
  s_db_item *cur_scan_db_item;
  WORD *pSaveRectBuff;
  int done = 0;
  WORD key;
  s_point old_cursor_pos;
  BYTE menu_textattrib;
  WORD oldcursor;

  menu_textattrib = GetFillAttrib();
  pSaveRectBuff = SaveRect (db->row, db->col, db->height, db->width);
  if (pSaveRectBuff != NULL)
  {
    old_cursor_pos = GetPhysicalCursorPos();

    oldcursor = SetCursorShape (0x2000);
    DrawBoxFilled (db->row, db->col, db->height, db->width, ' ');
    ShowTitle (db->row, db->col, db->title);

    /* Display gadgets in dialog box */
    SetTextAttrib (menu_textattrib);
    cur_db_item = DLLGetFirstItem (db->dbitems);
    old_db_item = cur_db_item;
    while (cur_db_item != NULL)
    {
      ShowGadget (db, cur_db_item);
      cur_db_item = DLLGetNextItem (db->dbitems);
    }

    cur_db_item = DLLGetFirstItem (db->dbitems);
    while (!done)
    {
      ShowGadgetData (db, old_db_item);
      old_db_item = cur_db_item;
      SetTextAttrib (menu_highlightattrib);
      ShowGadgetData (db, cur_db_item);
      SetTextAttrib (menu_textattrib);
      key = WaitForKeyPress();
      if (IsASCIIKey(key))
      {
  switch (key)
  {
    case KEY_TAB:
      cur_db_item = DLLCGetNextItem (db->dbitems);
      break;

    case KEY_SH_TAB:
      cur_db_item = DLLCGetPrevItem (db->dbitems);
      break;

    case KEY_ESC:
      done = 1;
      break;

    case KEY_ENTER:
    case KEY_SPACE:
      EditGadget (db, cur_db_item);
      break;

    default:
      cur_scan_db_item = DLLGetFirstItem (db->dbitems);
      while (cur_scan_db_item != NULL)
      {
        if (toupper(key & 255) == toupper(cur_scan_db_item->hotkeychar))
        {
    cur_db_item = cur_scan_db_item;
    ShowGadgetData (db, old_db_item);
    old_db_item = cur_db_item;
    SetTextAttrib (menu_highlightattrib);
    ShowGadgetData (db, cur_db_item);
    SetTextAttrib (menu_textattrib);
    EditGadget (db, cur_db_item);
    break;
        }
        cur_scan_db_item = DLLGetNextItem (db->dbitems);
      }
  }
      }
      else
      {
  switch (key)
  {
    case KEY_DN:
      cur_db_item = DLLCGetNextItem (db->dbitems);
      break;

    case KEY_UP:
      cur_db_item = DLLCGetPrevItem (db->dbitems);
      break;
  }
      }
    }
    RestoreRect (pSaveRectBuff);
    SetCursorShape (oldcursor);
    SetPhysicalCursorPos (old_cursor_pos.row, old_cursor_pos.col);
  }
}

/******************************************************************************\

\******************************************************************************/

void MPrintAt (int NoOfStrings, ...)
{
  va_list marker;

  va_start (marker, NoOfStrings);
  while (NoOfStrings--)
  {
    CursorAt (va_arg (marker,int), va_arg (marker,int));
    OutText (va_arg (marker, char *));
  };
  va_end (marker);
};

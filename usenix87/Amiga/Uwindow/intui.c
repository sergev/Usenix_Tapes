
#include "exec/types.h"
#include "exec/ports.h"
#include "exec/devices.h"
#include "exec/io.h"
#include "exec/memory.h"

#include "libraries/dos.h"
#include "graphics/text.h"
#include "libraries/diskfont.h"
#include "intuition/intuition.h"
 
/* Dynamic Intuition Text functions */

struct IntuiText *NewIText(text, left, top)
char *text;
int left, top;
{
   struct IntuiText *newtext = NULL;

   if (newtext = (struct IntuiText *)AllocMem(sizeof(*newtext),
                                              MEMF_PUBLIC | MEMF_CLEAR))
      {
      newtext->IText = (UBYTE *)text;
      newtext->FrontPen = 0;
      newtext->BackPen = 1;
      newtext->DrawMode = JAM2;
      newtext->LeftEdge = left;
      newtext->TopEdge = top;
      newtext->ITextFont = NULL;
      newtext->NextText = NULL;
      }
   return(newtext);
}

struct IntuiText *AddIText(IText, text)
struct IntuiText *IText;
char *text;
{
   struct IntuiText *newtext = NULL;

   if (IText) {
      if (newtext = (struct IntuiText *)AllocMem(sizeof(*newtext),
                                                 MEMF_PUBLIC | MEMF_CLEAR))
         {
         newtext->IText = (UBYTE *)text;
            newtext->FrontPen = IText->FrontPen;
         newtext->BackPen  = IText->BackPen;
         newtext->DrawMode = IText->DrawMode;
         newtext->LeftEdge = IText->LeftEdge;
         newtext->TopEdge  = IText->TopEdge + 11;
         newtext->ITextFont = IText->ITextFont;
         newtext->NextText = NULL;
         IText->NextText   = newtext;
         }
      }
   return(newtext);
}

DisposeIText(IText)
struct IntuiText *IText;
{
   struct IntuiText *current, *next;

   current = IText;
   for(next = current->NextText; current != NULL; next = next->NextText)
      {
      FreeMem(current,sizeof(*current));
      current = next;
      }
}


/* Dynamic Menu Constructor Functions */

#define InterMenuWidth 15

struct Menu *NewMenu(menuname, width, height)
char *menuname;
int width, height;
{
   struct Menu *menu = NULL;

   if(menu = (struct Menu *)AllocMem(sizeof(*menu),
                                     MEMF_PUBLIC | MEMF_CLEAR))
      {
      menu->NextMenu = NULL;
      menu->LeftEdge = 0;
      menu->TopEdge = 0;
      menu->Width = width;
      menu->Height = height;
      menu->Flags = MENUENABLED;
      menu->MenuName = menuname;
      menu->FirstItem = NULL;
      }
   return(menu);
}

struct Menu *AddMenu(menus, MenuName, width, height)
struct Menu *menus;
char *MenuName;
int width, height;
{
   struct Menu *newmenu;

   if (menus) {
      if (newmenu = NewMenu(MenuName, width, height)) {
         newmenu->LeftEdge = menus->LeftEdge + menus->Width + InterMenuWidth;
         menus->NextMenu = newmenu;
         }
      }
   return(newmenu);
}

struct MenuItem *NewMenuItem(name, width, height)
char *name;
int width, height;
{
   struct MenuItem *newitem = NULL;
   struct IntuiText *NewIText(), *newtext = NULL;

   if (newitem = (struct MenuItem *)AllocMem(sizeof(*newitem),
                                             MEMF_PUBLIC | MEMF_CLEAR))
      {
      if (newtext = NewIText(name,0,1)) {
         newitem->NextItem = NULL;
         newitem->ItemFill = (APTR) newtext;
         newitem->LeftEdge = 0;
         newitem->TopEdge = 0;
         newitem->Width = width;
         newitem->Height = height;
         newitem->Flags = ITEMTEXT | ITEMENABLED | HIGHCOMP;
         newitem->MutualExclude = 0;
         newitem->SelectFill = NULL;
         newitem->Command = 0;
         newitem->SubItem = NULL;
         newitem->NextSelect = 0;
         } else {
         DisposeItem(newitem);
         newitem = NULL;
         }
      }
   return(newitem);
}

struct MenuItem *AddNewMenuItem(menu, name, width, height)
struct Menu *menu;
char *name;
int width, height;
{
   struct MenuItem *newitem = NULL, *NewMenuItem();

   if (menu) {
      if (newitem = NewMenuItem(name, width, height)) {
         menu->FirstItem = newitem;
         }
      }
   return(newitem);
}

struct MenuItem *AddItem(items, name)
struct MenuItem *items;
char *name;
{
   struct MenuItem *newitem = NULL, *NewMenuItem();

   if (items) {
      if (newitem = NewMenuItem(name, items->Width, items->Height))
         {
         newitem->TopEdge = items->TopEdge + items->Height;
         newitem->LeftEdge = items->LeftEdge;
         items->NextItem = newitem;
         if (items->MutualExclude) {
            newitem->MutualExclude = (~((~items->MutualExclude) << 1));
            newitem->Flags |= CHECKIT;
            }
         }
      }
   return(newitem);
}

struct MenuItem *AddNewSubItem(item, name, width, height)
struct MenuItem *item;
char *name;
int width, height;
{
   struct MenuItem *newitem = NULL, *NewMenuItem();

   if (item) {
      if (newitem = NewMenuItem(name, width, height)) {
         item->SubItem = newitem;
         newitem->LeftEdge = item->Width;
         }
      }
   return(newitem);
}

DisposeItem(item)
struct MenuItem *item;
{
   DisposeIText((struct ItuiText *)item->ItemFill);
   FreeMem(item,sizeof(*item));
}

DisposeItems(items)
struct MenuItem *items;
{
   struct MenuItem *current, *next;

   current = items;
   for(next = current->NextItem; current != NULL; next = next->NextItem){
      DisposeItem(current);
      current = next;
   }
}

DisposeMenu(menu)
struct Menu *menu;
{
   DisposeItems(menu->FirstItem);
   FreeMem(menu,sizeof(*menu));
}

DisposeMenus(menus)
struct Menu *menus;
{
   struct Menu *current, *next;

   current = menus;
   for(next = current->NextMenu; current != NULL; next = next->NextMenu){
      DisposeMenu(current);
      current = next;
   }
}

/* modified autorequester */


USHORT   OKBoolV1[18] = {
   0,0,  35,0, 35,17,   0,17,
   0,0,  34,0, 34,17,   1,17, 1,0
   };

USHORT OKBoolV2[18] = {
   0,0,  31,0, 31,15,   0,15,
   0,0,  30,0, 30,15,   1,15, 1,0
   };

struct Border OKBoolBorder2 = {
   0, 0,
   0, 1, JAM1,
   9,
   OKBoolV2,
   NULL
   };

struct Border OKBoolBorder1 = {
   -2, -1,
   3, 1, JAM1,
   9,
   OKBoolV1,
   &OKBoolBorder2
   };

struct IntuiText OKText = {
   0, 1, JAM2,
   8, 4,
   NULL,
   "OK",
   NULL
   };

struct Gadget OKBoolGadget = {
   NULL,
   -46, -24, 32, 16,
   GADGHCOMP| GRELRIGHT | GRELBOTTOM,
   TOGGLESELECT | RELVERIFY | ENDGADGET,
   BOOLGADGET | REQGADGET,
   (APTR)&OKBoolBorder1,
   NULL,
   &OKText,
   0, NULL, 0, NULL
   };

USHORT CancelBoolV1[18] = {
   0,0,  67,0, 67,17,   0,17,
   0,0,  66,0, 66,17,   1,17, 1,0
   };

USHORT CancelBoolV2[18] = {
   0,0,  63,0, 63,15,   0,15,
   0,0,  62,0, 62,15,   1,15, 1,0
   };

struct Border CancelBoolBorder2 = {
   0, 0,
   0, 1, JAM1,
   9,
   CancelBoolV2,
   NULL
   };

struct Border CancelBoolBorder1 = {
   -2, -1,
   3, 1, JAM1,
   9,
   CancelBoolV1,
   &CancelBoolBorder2
   };

struct IntuiText CancelText = {
   0, 1, JAM2,
   7, 4,
   NULL,
   "Cancel",
   NULL
   };

struct Gadget CancelBoolGadget = {
   NULL,
   14, -24, 64, 16,
   GADGHCOMP| GRELBOTTOM,
   TOGGLESELECT | RELVERIFY | ENDGADGET,
   BOOLGADGET | REQGADGET,
   (APTR)&CancelBoolBorder1,
   NULL,
   &CancelText,
   0, NULL, 0, NULL
   };

USHORT ReqV1[18], ReqV2[18];

struct Border ReqBorder2 = {
   6, 3,
   0, 1, JAM1,
   9,
   ReqV2,
   NULL
   };

struct Border ReqBorder1 = {
   2, 1,
   0, 1, JAM1,
   9,
   ReqV1,
   &ReqBorder2
   };

struct Requester autoreq;

initrequester(width, height)
int width, height;
{
   ReqV1[0] = 0;
   ReqV1[1] = 0;
   ReqV1[2] = width-5;
   ReqV1[3] = 0;
   ReqV1[4] = width-5;
   ReqV1[5] = height-3;
   ReqV1[6] = 0;
   ReqV1[7] = height-3;
   ReqV1[8] = 0;
   ReqV1[9] = 0;
   ReqV1[10] = width-6;
   ReqV1[11] = 0;
   ReqV1[12] = width-6;
   ReqV1[13] = height-3;
   ReqV1[14] = 1;
   ReqV1[15] = height-3;
   ReqV1[16] = 1;
   ReqV1[17] = 0;
   ReqV2[0] = 0;
   ReqV2[1] = 0;
   ReqV2[2] = width-14;
   ReqV2[3] = 0;
   ReqV2[4] = width-14;
   ReqV2[5] = height-7;
   ReqV2[6] = 0;
   ReqV2[7] = height-7;
   ReqV2[8] = 0;
   ReqV2[9] = 0;
   ReqV2[10] = width-13;
   ReqV2[11] = 0;
   ReqV2[12] = width-13;
   ReqV2[13] = height-7;
   ReqV2[14] = 1;
   ReqV2[15] = height-7;
   ReqV2[16] = 1;
   ReqV2[17] = 0;
   InitRequester(&autoreq);
   autoreq.LeftEdge = (640 - width)/2;
   autoreq.TopEdge = (200 - height)/2;
   autoreq.Width = width;
   autoreq.Height = height;
   autoreq.RelLeft = 0;
   autoreq.RelTop = 0;
   autoreq.ReqBorder = &ReqBorder1;
   autoreq.Flags = NULL;
   autoreq.BackFill = 1;
}

autorequest(window, body, cancel, width, height)
struct Window *window;
struct IntuiText *body;
int cancel, width, height;
{
   struct IntuiMessage *IntuiMsg;
   USHORT idcmpflags;            /* holding place for window's flags */

   idcmpflags = window->IDCMPFlags;
   window->IDCMPFlags = GADGETUP;

   initrequester(width, height);
   autoreq.ReqText = body;
   autoreq.ReqGadget = &OKBoolGadget;
   OKBoolGadget.NextGadget = NULL;
   if (cancel) {
      OnGadget(&CancelBoolGadget, window, &autoreq);
      CancelBoolGadget.Flags = GADGHCOMP | GRELBOTTOM;
      OKBoolGadget.NextGadget = &CancelBoolGadget;
      }
   OKBoolGadget.Flags = GADGHCOMP | GRELRIGHT | GRELBOTTOM;
   OnGadget(&OKBoolGadget, window, &autoreq);

   Request(&autoreq, window);

   while ((IntuiMsg = (struct IntuiMessage *)
                      GetMsg(window->UserPort)) == 0)
      Wait(1 << window->UserPort->mp_SigBit);
   ReplyMsg(IntuiMsg);

   window->IDCMPFlags = idcmpflags;
   return((CancelBoolGadget.Flags & SELECTED) == 0);
}

length(s)
char *s;
{
   char *p = s;

   while(*p++);
   return(p-s);
}

Notify(window, message)
struct Window *window;
char *message;
{
   int c;
   struct IntuiText *MsgText;

   MsgText = NewIText(message, 14, 7);
   c = length(message) * 8 + 14;
   autorequest(window, MsgText, FALSE, c, 50);
}

Query(window, message)
struct Window *window;
char *message;
{
   int c;
   struct IntuiText *MsgText;

   MsgText = NewIText(message, 14, 7);
   c = length(message) * 8 + 14;
   return(autorequest(window, MsgText, TRUE, c, 50));
}


/*  Standard File Package */

USHORT   SaveBoolV1[18] = {
   0,0,  51,0, 51,17,   0,17,
   0,0,  50,0, 50,17,   1,17, 1,0
   };

USHORT SaveBoolV2[18] = {
   0,0,  47,0, 47,15,   0,15,
   0,0,  46,0, 46,15,   1,15, 1,0
   };

struct Border SaveBoolBorder2 = {
   0, 0,
   0, 1, JAM1,
   9,
   SaveBoolV2,
   NULL
   };

struct Border SaveBoolBorder1 = {
   -2, -1,
   3, 1, JAM1,
   9,
   SaveBoolV1,
   &SaveBoolBorder2
   };

struct IntuiText SaveText = {
   0, 1, JAM2,
   8, 4,
   NULL,
   "Save",
   NULL
   };

struct Gadget SaveBoolGadget = {
   &CancelBoolGadget,
   -62, -24, 48, 16,
   GADGHCOMP| GRELRIGHT | GRELBOTTOM,
   TOGGLESELECT | RELVERIFY | ENDGADGET,
   BOOLGADGET | REQGADGET,
   (APTR)&SaveBoolBorder1,
   NULL,
   &SaveText,
   0, NULL, 0, NULL
   };

USHORT   OpenBoolV1[18] = {
   0,0,  51,0, 51,17,   0,17,
   0,0,  50,0, 50,17,   1,17, 1,0
   };

USHORT OpenBoolV2[18] = {
   0,0,  47,0, 47,15,   0,15,
   0,0,  46,0, 46,15,   1,15, 1,0
   };

struct Border OpenBoolBorder2 = {
   0, 0,
   0, 1, JAM1,
   9,
   OpenBoolV2,
   NULL
   };

struct Border OpenBoolBorder1 = {
   -2, -1,
   3, 1, JAM1,
   9,
   OpenBoolV1,
   &OpenBoolBorder2
   };

struct IntuiText OpenText = {
   0, 1, JAM2,
   8, 4,
   NULL,
   "Open",
   NULL
   };

struct Gadget OpenBoolGadget = {
   &CancelBoolGadget,
   -62, -24, 48, 16,
   GADGHCOMP| GRELRIGHT | GRELBOTTOM,
   TOGGLESELECT | RELVERIFY | ENDGADGET,
   BOOLGADGET | REQGADGET,
   (APTR)&OpenBoolBorder1,
   NULL,
   &OpenText,
   0, NULL, 0, NULL
   };

char GFNBuffer[255];
char GFNUBuffer[255];

struct StringInfo GFNStrInfo =
   {
   (UBYTE *)GFNBuffer,
   (UBYTE *)GFNUBuffer,
   0, 255, 0,
   0, 0, 0, 0, 0,
   NULL, 0, NULL
   };

struct Gadget GFNStrGad =
   {
   NULL,
   14, 19, 270, 11,
   GADGHCOMP | SELECTED,
   RELVERIFY | ENDGADGET,
   STRGADGET | REQGADGET,
   NULL,       /* Border */
   NULL,
   NULL,       /* To point to Gadget text */
   0,
   (APTR)&GFNStrInfo,
   0, NULL
   };

GFN(window, openflag, message)
struct Window *window;
int openflag;
char *message;
{
   struct IntuiText *GFNText, *NewIText(), *AddIText();
   struct IntuiMessage *IntuiMsg;

   GFNText = NewIText( message, 14, 7);
   initrequester(300, 60);
   autoreq.ReqText = GFNText;

   autoreq.ReqGadget = &GFNStrGad;
   if (openflag) {
      GFNStrGad.NextGadget = &OpenBoolGadget;
      OnGadget(&OpenBoolGadget, window, &autoreq);
      OpenBoolGadget.Flags = GADGHCOMP | GRELBOTTOM | GRELRIGHT;
      } else {
      GFNStrGad.NextGadget = &SaveBoolGadget;
      OnGadget(&SaveBoolGadget, window, &autoreq);
      SaveBoolGadget.Flags = GADGHCOMP | GRELBOTTOM | GRELRIGHT;
      }
   OnGadget(&GFNStrGad, window, &autoreq);
   OnGadget(&CancelBoolGadget, window, &autoreq);
   CancelBoolGadget.Flags = GADGHCOMP | GRELBOTTOM;

   Request(&autoreq, window);
   while ((IntuiMsg = (struct IntuiMessage *)GetMsg(window->UserPort)) == 0)
      Wait(1 << window->UserPort->mp_SigBit);
   ReplyMsg(IntuiMsg);
   DisposeIText(GFNText);
   return((CancelBoolGadget.Flags & SELECTED) == 0);
}

struct FileHandle *OpenFile(window, prompt)
struct Window *window;
char *prompt;
{
   struct FileLock *lock, *Lock();
   struct FileHandle *file, *Open();

   if (GFN(window, TRUE, prompt)) {
      UnLock(lock = Lock(GFNBuffer, ACCESS_READ));
      if (lock != 0) {
         file = Open(GFNBuffer, MODE_OLDFILE);
         if (file != 0) {
            return(file);
            }
         Notify(window, "Can't open file.");
         return(NULL);
         }
      Notify(window, "File not found.");
   }
   return(NULL);
}

struct FileHandle *SaveFile(window, prompt)
struct Window *window;
char *prompt;
{
   struct FileLock *lock, *Lock();
   struct FileHandle *file, *Open();

   if(GFN(window, FALSE, prompt)) {
      UnLock(lock = Lock(GFNBuffer, ACCESS_WRITE));
      if (lock == 0) {
         file = Open(GFNBuffer, MODE_NEWFILE);
         if (file != 0) {
            return(file);
            }
         Notify(window,"Can't open file.");
         return(NULL);
         }
      if (Query(window,"Overwrite file?")) {
         file = Open(GFNBuffer, MODE_NEWFILE);
         if (file != 0) {
            return(file);
            }
         Notify(window,"Can't open file.", 14, 7);
         return(NULL);
         }
      }
   return(NULL);
}


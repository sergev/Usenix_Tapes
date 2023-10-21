/*
 *  Simple menu package.  Needs lotsa work to handle some cases.
 *
 *  Copyright 1985
 *  Louis A. Mamakos
 *  Software & Stuff
 *  14813 Ashford Place
 *  Laurel, MD  20707
 *
 *  For non-commerical use only.  This program, or any modifications, may not
 *  be sold or incorporated into any product without prior permission from the
 *  author.
 *
 *  Modified by mwm to handle "stacking" menus.
 *	NB - adding item to a menu that's been "popped" back to doesn't work,
 *	and probably never will.
 * 	I took at the subitem code at the same time - mwm
 */

/* Change if your terminal isn't * x 400 */
#define SCREENHEIGHT		200

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <hardware/blit.h>
#include <graphics/copper.h>
#include <graphics/regions.h>
#include <graphics/rastport.h>
#include <graphics/gfxbase.h>
#include <graphics/gels.h>
#include <intuition/intuition.h>

#define MNUM(menu,item,sub) (SHIFTMENU(menu)|SHIFTITEM(item)|SHIFTSUB(sub))

struct	Mem_Node {
   struct Node mn_Node;
   struct Remember mn_Memory;
   struct Menu *mn_Menu;
   } *Top, *RemHead();

static struct List Memory;
static int Cur_Menu, Cur_MenuItem, Cur_SubItem;
static struct Menu *LastMenu;
static struct MenuItem *LastMenuItem;

struct Menu *AutoMenu;      /* menu struct being dynamically built */

char *strsave();	    /* Save a string in the remember list */

Menu_Init()
{
   Memory.lh_Head = (struct Node *) &(Memory.lh_Tail);
   Memory.lh_TailPred = (struct Node *) &(Memory.lh_Head);
   Memory.lh_Tail = NULL;
   Memory.lh_Type = NT_MEMORY;
   Top = NULL;
   Cur_Menu = Cur_MenuItem = -1;
   AutoMenu = LastMenu = NULL;     /* no menu chain yet */
   LastMenuItem = NULL;
}

Menu_Clear()
{

   while ((Top = RemHead(&Memory)) != NULL) {
      FreeRemember(&(Top->mn_Memory), TRUE);
      FreeMem(Top, sizeof(struct Mem_Node));
  }
  Menu_Init();			/* Just for safeties sake */
}

Menu_Pop()
{

   if ((Top = RemHead(&Memory)) == NULL) return NULL;
   FreeRemember(&(Top->mn_Memory), TRUE);
   FreeMem(Top, sizeof(struct Mem_Node));
   /* Now, set Top back to the real list head */
   Top = (struct Mem_Node *) Memory.lh_Head;
   LastMenu = Top->mn_Menu;
   LastMenuItem = NULL;		/* Wrong, but you can't add items here anyway */
   Cur_Menu--;
}
/*
 *  Add a MENU item.  Args are the text of the menu item, and an enable
 *  flag.  Returns an Intuition type menu number, with the MenuItem and
 *  Menu SubItem being NOITEM and NOSUB.  The MENUITEM part is valid.
 */
Menu_Add(name, enabled)
   char *name;
   int enabled;
{
   register struct Menu *m;

   if ((Top = (struct Mem_Node *) AllocMem(
		 sizeof(struct Mem_Node), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
      return NULL;
   Top->mn_Node.ln_Type = NT_MEMORY;

   if ((m = (struct Menu *)AllocRemember(&(Top->mn_Memory),
                 sizeof (struct Menu), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
      return NULL;
   Top->mn_Menu = m;

   if (LastMenu == NULL)
      AutoMenu = m;     /* first menu on list */
   else
      LastMenu->NextMenu = m;      /* link it in */

   LastMenuItem = NULL;            /* end of previous MenuItem list */
   Cur_MenuItem = -1; /* reset item numbers */
   if (LastMenu == NULL)
      m->LeftEdge = 0;
   else
      m->LeftEdge = LastMenu->LeftEdge + LastMenu->Width;
   m->TopEdge = 0;
   m->Width = strlen(name) * 8;
   Top->mn_Node.ln_Name = m->MenuName = strsave(name);
   m->Height = 0;
   m->Flags = enabled ? MENUENABLED : 0;
   m->FirstItem = NULL;
   LastMenu = m;

   AddHead(&Memory, Top);
   return MNUM(++Cur_Menu, NOITEM, NOSUB);
}

/*
 *  Add a menu item to the current MENU.  Note that Add_Menu *must* be
 *  called before this function.
 */
Menu_Item_Add(name, flags, mux, ch)
   char *name;       /* name of menu item */
   USHORT flags;
   LONG mux;         /* mutual exclusion mask */
   BYTE ch;        /* command sequence character, if COMMSEQ */
{
   register struct MenuItem *m, *n;
   register struct IntuiText *it;

   flags &= CHECKIT|CHECKED|COMMSEQ|MENUTOGGLE|ITEMENABLED|HIGHCOMP|HIGHBOX;
   if (LastMenu == NULL)
      return MNUM(NOMENU, NOITEM, NOSUB);

   if ((m = (struct MenuItem *) AllocRemember(&(Top->mn_Memory),
                 sizeof(struct MenuItem), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
      return MNUM(NOMENU, NOITEM, NOSUB);

   if (LastMenuItem == NULL)
      LastMenu->FirstItem  = m;
   else
      LastMenuItem->NextItem = m;
   m->Flags = flags | ITEMTEXT;
   /*
    *  Check for highlight mode:  if none selected, use HIGHCOMP
    */
   if ((m->Flags & (HIGHCOMP | HIGHBOX)) == 0)
      m->Flags |= HIGHCOMP;
   m->Command = ch;
   m->MutualExclude = mux;
   m->SubItem = NULL;
   m->ItemFill = (APTR) AllocRemember(&(Top->mn_Memory),
                      sizeof(struct IntuiText), MEMF_PUBLIC | MEMF_CLEAR);
   it = (struct IntuiText *) m->ItemFill;
   it->FrontPen = AUTOFRONTPEN;
   it->BackPen = AUTOBACKPEN;
   it->DrawMode = JAM2;
   if (flags & CHECKIT)
      it->LeftEdge = CHECKWIDTH + 1;
   else
      it->LeftEdge = 1;
   it->TopEdge = 1;
   it->ITextFont = NULL;      /* default font */
   it->IText = strsave(name);
   it->NextText = NULL;
   m->Width = 0;
   if (LastMenuItem == NULL) {
      m->TopEdge = 2;
      m->LeftEdge = 0;
   } else if (LastMenuItem->TopEdge + 40 > SCREENHEIGHT) {
      m->TopEdge = 2;
      m->LeftEdge = LastMenuItem->LeftEdge + LastMenuItem->Width + 12;
   } else {
      m->TopEdge = LastMenuItem->TopEdge + 12;
      m->LeftEdge = LastMenuItem->LeftEdge;
   }
   if (flags & CHECKIT)
      m->Width += CHECKWIDTH;
   if (flags & COMMSEQ)
      m->Width += COMMWIDTH + 20;
   m->Width += IntuiTextLength(m->ItemFill);
   m->Height = 10;
   /*
    *  Check last menu item's width to see if it is larger than this
    *  item's.  If new item is larger, then update width of all other
    *  items.
    */
   if (LastMenuItem) {
      if (LastMenuItem->Width > m->Width)
        m->Width = LastMenuItem->Width;
      else {
         register short delta = m->Width - LastMenuItem->Width;

	 for (n = LastMenu->FirstItem; n != m; n = n->NextItem) {
	    n->Width = m->Width;
	    if (n->LeftEdge > 0) n->LeftEdge += delta;
	 }
	 if (m->LeftEdge > 0) m->LeftEdge += delta;
      }
   }
   LastMenuItem = m;
   return MNUM(Cur_Menu, ++Cur_MenuItem, NOSUB);
}



char *
strsave(string) char *string; {
   char *out ;

   out = (char *) AllocRemember(&(Top->mn_Memory), strlen(string) + 1,
	MEMF_PUBLIC) ;
   if (out == NULL) return NULL ;
   (void) strcpy(out, string) ;
   return out ;
}

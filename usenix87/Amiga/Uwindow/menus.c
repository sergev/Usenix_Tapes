/* menu function calls and handlers */

#include "term.h"
#include "devices/serial.h"

/* GLOBALS ****************************************************** */
extern long IntuitionBase;
extern long GfxBase;

extern struct IOExtSer *SerReadReq, *SerWriteReq;
extern char serin;
extern int FullDuplex;

struct Menu *MenuHead;

InitMenus()
{
   struct Menu *CurrentMenu, *NewMenu(), *AddMenu();
   struct MenuItem *CurrentItem, *SubItem,
                   *AddNewMenuItem(), *AddItem(), *AddNewSubItem();

   CurrentMenu    = NewMenu("Project", 60, 10);
   MenuHead       = CurrentMenu;
      CurrentItem = AddNewMenuItem(CurrentMenu, "About...", 100,11);
         SubItem  = AddNewSubItem(CurrentItem, "UWTerm 1.00", 92,11);
         SubItem  = AddItem(SubItem,"HackerWare");
      CurrentItem = AddItem(CurrentItem, "New");
      CurrentItem = AddItem(CurrentItem, "Window");
         SubItem  = AddNewSubItem(CurrentItem, "to Back",68,11);
         SubItem  = AddItem(SubItem,"to Front");
      CurrentItem = AddItem(CurrentItem, "Quit");
   CurrentMenu    = AddMenu(CurrentMenu, "Settings",68,10);
      CurrentItem = AddNewMenuItem(CurrentMenu,"Baud",52,11);
         SubItem  = AddNewSubItem(CurrentItem, "   300   ",76,11);
         if (SubItem) {
            SubItem->MutualExclude = ~1;
            SubItem->Flags |= CHECKIT;
            }
         SubItem  = AddItem(SubItem,"  1200   ");
         SubItem  = AddItem(SubItem,"  2400   ");
         SubItem  = AddItem(SubItem,"  4800   ");
         SubItem  = AddItem(SubItem,"  9600   ");
         if (SubItem) SubItem->Flags |= CHECKED;
      CurrentItem = AddItem(CurrentItem,"Length");
         SubItem  = AddNewSubItem(CurrentItem,"   7 bits   ",100,11);
         if (SubItem) {
            SubItem->MutualExclude = ~1;
            SubItem->Flags |= CHECKIT;
            SubItem->Flags |= CHECKED;
            }
         SubItem  = AddItem(SubItem,"   8 bits   ");
      CurrentItem = AddItem(CurrentItem,"Duplex");
         SubItem  = AddNewSubItem(CurrentItem,"   Full   ",100,11);
         if (SubItem) {
            SubItem->MutualExclude = ~1;
            SubItem->Flags |= CHECKIT | CHECKED;
            }
         SubItem  = AddItem(SubItem,"   Half   ");

   if (SubItem) return(TRUE);
     else return(FALSE);
}

MenuSwitch(code)
USHORT code;
{
   USHORT menunum;
   struct MenuItem *item;
   int error;

   error = TRUE;
   while(code != MENUNULL ) {
      item = (struct MenuItem *)ItemAddress(MenuHead, code);
      menunum = MENUNUM( code );
      switch( menunum ) {
         case 0:
            error &= ProjectMenu(code);
            break;
         case 1:
            error &= SettingsMenu(code);
            break;
      } /* end of switch ( menunum ) */
      code = item->NextSelect;
   } /* end of while (code != MENUNULL) */
   return(error);
} /* end of MenuSwitch */


ProjectMenu(code)
USHORT code;
{
   USHORT itemnum;
   itemnum = ITEMNUM( code );
   switch( itemnum ) {
      case 0:  /* About */
         return(AboutMenu(code));
         break;
      case 1:  /* New */
	 MkNewWin();
	 return(TRUE);
	 break;
      case 2:  /* Window */
         return(ArrangeMenu(code));
         break;
      case 3: /* Quit */
         return( FALSE );
         break;
   } /* end of switch ( itemnum ) */
   return( TRUE );
} /* end of ProjectMenu */

AboutMenu(code)
USHORT code;
{
   struct IntuiText *InfoText, *NewIText(), *AddIText();
   USHORT subitem;
   subitem = SUBNUM( code );
   switch( subitem ) {
      case 0:
         InfoText = NewIText( "Unix Windows pcl1 !Beta!",12,5);
         AddIText(
         AddIText(
         AddIText(
         AddIText(InfoText,   "       Copyright ) 1987"),
                              "      by Michael McInerny"),
                              "       140 Spruce Ave"),
                              "      Rochester, NY  14611");
         autorequest(uw[uw_write].win, InfoText, FALSE, 276, 70);
         DisposeIText(InfoText);
         break;
      case 1:
         InfoText = NewIText( " Hackerware - Software for the People",12,5);
         AddIText(
         AddIText(
         AddIText(
         AddIText(
         AddIText(
         AddIText(InfoText,   " Feel free to use this software and"),
                              "  source--just don't sell it. "),
                              "Please distribute source with binary."),
                              "If you like this program, please send"),
                              "whatever you think it's worth to the"),
                              "     author.  Thank-you!");
         autorequest(uw[uw_write].win, InfoText, FALSE, 330, 108);
         DisposeIText(InfoText);
         break;
   } /* end of switch ( subitem ) */
   return( TRUE );
} /* end of AboutMenu */

ArrangeMenu(code)
USHORT code;
{
   USHORT subitem;
   subitem = SUBNUM( code );
   switch( subitem ) {
      case 0:
         WindowToBack( uw[uw_read].win );
         break;
      case 1:
         WindowToFront( uw[uw_read].win );
         break;
   } /* end of switch ( subitem ) */
   return( TRUE );
} /* end of ArrangeMenu */


SettingsMenu(code)
USHORT code;
{
   USHORT itemnum;
   itemnum = ITEMNUM( code );
   AbortIO(SerReadReq);
   switch ( itemnum ) {
      case 0:
         BaudMenu(code);
         break;
      case 1:
         LengthMenu(code);
         break;
      case 2:
         DuplexMenu(code);
         break;
   } /* end of switch ( itemnum ) */
   SerReadReq->IOSer.io_Command = SDCMD_SETPARAMS;
   DoIO(SerReadReq);
   QueueSerRead(SerReadReq, &serin);
   return( TRUE );
} /* end of SettingsMenu */

BaudMenu(code)
USHORT code;
{
   USHORT subitem;
   subitem = SUBNUM( code );
   switch( subitem ) {
      case 0:
         SerReadReq->io_Baud = 300;
         break;
      case 1:
         SerReadReq->io_Baud = 1200;
         break;
      case 2:
         SerReadReq->io_Baud = 2400;
         break;
      case 3:
         SerReadReq->io_Baud = 4800;
         break;
      case 4:
         SerReadReq->io_Baud = 9600;
         break;
   } /* end of switch ( subitem ) */
} /* end of BaudMenu */

LengthMenu(code)
USHORT code;
{
   USHORT subitem;
   subitem = SUBNUM( code );
   switch( subitem ) {
      case 0:
         SerReadReq->io_ReadLen = 0x07;
         SerReadReq->io_WriteLen = 0x07;
         break;
      case 1:
         SerReadReq->io_ReadLen = 0x08;
         SerReadReq->io_WriteLen = 0x08;
         break;
   } /* end of switch ( subitem ) */
} /* end of LengthMenu */

DuplexMenu(code)
USHORT code;
{
   USHORT subitem;
   subitem = SUBNUM( code );
   switch( subitem ) {
      case 0:
         FullDuplex = TRUE;
         break;
      case 1:
         FullDuplex = FALSE;
         break;
   } /* end of switch ( subitem ) */
} /* end of DuplexMenu */


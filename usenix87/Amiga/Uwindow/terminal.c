
/*  Unix Windows Client for the Amiga (ANSI Terminal Emulation only)

    ) 1987 by  Michael J. McInerny   12-May-87 version 1.00

*/

#define INTUITION_MESSAGE(x) (1<<uw[x].winsig)
#define TYPED_CHARACTER(x) (1<<uw[x].consig)
#define INPUT_CHARACTER (1<<serReadBit)

#define CloseConsole(x) CloseDevice(x)

#include "term.h"
#include "devices/serial.h"

/* GLOBALS ****************************************************** */
extern long IntuitionBase;
extern long GfxBase;

extern UWwrite();

struct uw_struct uw[8];
int uw_read, uw_write, uw_count = 0;

extern struct Window *NewTermWin();
extern struct Menu *MenuHead;

int waitmask;				/* what are we waiting for? */
struct MsgPort *serReadPort = NULL;
struct MsgPort *serWritePort = NULL;
struct IOExtSer *SerReadReq = NULL;
struct IOExtSer *SerWriteReq = NULL;

char letter[8];            /* one letter at a time from console */
char serin;             /* one letter at a time from serial  */
char serbuf[4096];	/* as much as we can get! */
int sercount;			 /* how many to get */
int FullDuplex = TRUE;  /* flag for local echo  */

AllocTerm(slot)
int slot;
{
   char *s = "xxuw.con";

   uw_count += 1;
   uw[slot].free = FALSE;
   uw[slot].win = NewTermWin();
   SetMenuStrip(uw[slot].win, MenuHead);
   *s = (char)slot + 'a';
   *(s+1) = 'w';
   uw[slot].ConWritePort = CreatePort(s,0);
	   if(uw[slot].ConWritePort == 0) Cleanup();
   uw[slot].ConWriteReq = CreateStdIO(uw[slot].ConWritePort);
	   if(uw[slot].ConWriteReq == 0) Cleanup();
   *(s+1) = 'r';
   uw[slot].ConReadPort = CreatePort(s,0);
	   if(uw[slot].ConReadPort == 0) Cleanup();
   uw[slot].ConReadReq =  CreateStdIO(uw[slot].ConReadPort);
	   if(uw[slot].ConReadReq == 0) Cleanup();
   if((OpenConsole(uw[slot].ConWriteReq,
			uw[slot].ConReadReq,
			uw[slot].win)) != 0)
      Cleanup();
   uw[slot].consig = uw[slot].ConReadReq->io_Message.mn_ReplyPort->mp_SigBit;
   uw[slot].winsig = uw[slot].win->UserPort->mp_SigBit;
   waitmask |= INTUITION_MESSAGE(slot) | TYPED_CHARACTER(slot) ;
   QueueRead(uw[slot].ConReadReq, &letter[slot]);
} /* end of AllocTerm */

ZeroTerm(slot)
int slot;
{
	uw[slot].free = TRUE;
	uw[slot].ConReadPort = NULL;
	uw[slot].ConWritePort = NULL;
	uw[slot].ConReadReq = NULL;
	uw[slot].ConWriteReq = NULL;
	uw[slot].win = NULL;
	uw[slot].winsig = 0;
	uw[slot].consig = 0;
} /* end of ZeroTerm */

DeallocTerm(slot)
int slot;
{
   waitmask &= ~(INTUITION_MESSAGE(slot) | TYPED_CHARACTER(slot)) ;
   if (uw[slot].free == FALSE) {
	AbortIO(uw[slot].ConReadReq);  /* cancel the last queued read */
	uw_count -= 1;
	if (uw[slot].ConWriteReq) CloseConsole(uw[slot].ConWriteReq);
	if (uw[slot].ConReadReq) DeleteStdIO(uw[slot].ConReadReq);
	if (uw[slot].ConReadPort) DeletePort(uw[slot].ConReadPort);
	if (uw[slot].ConWriteReq) DeleteStdIO(uw[slot].ConWriteReq);
	if (uw[slot].ConWritePort) DeletePort(uw[slot].ConWritePort);
	if (uw[slot].win) {
	   ClearMenuStrip(uw[slot].win);
	   CloseWindow(uw[slot].win);
	}
	ZeroTerm(slot);
   } /* end if uw.free */
} /* end of DeallocTerm */

InitSerReqs()
{
   serReadPort = CreatePort("uw.ser.read",0);
   if (serReadPort == NULL) Cleanup();
   SerReadReq = (struct IOExtSer *)CreateExtIO(serReadPort,
                                               sizeof(struct IOExtSer));
   if (SerReadReq == NULL) Cleanup();
   serWritePort = CreatePort("uw.ser.write",0);
   if (serWritePort == NULL) Cleanup();
   SerWriteReq = (struct IOExtSer *)CreateExtIO(serWritePort,
                                               sizeof(struct IOExtSer));
   if (SerWriteReq == NULL) Cleanup();
   if ((OpenSerial(SerReadReq, SerWriteReq)) != 0) Cleanup();
   if ((SetParams( SerReadReq, 4096, 0x07, 0x07, 750000,
                           9600, 0x00, 
			   0x51040303, 0x03030303)) != 0)
      Cleanup();
}

main()
{
   USHORT class, code, qualifier;
   int problem, i, serReadBit;
   struct IntuiMessage *message; /* the message the IDCMP sends us */

   IntuitionBase = 0L;
   GfxBase = 0L;
   MenuHead = NULL;

   for(i = 0; i < 8; ++i) ZeroTerm(i);

   InitLibs();
   InitSerReqs();
   serReadBit = SerReadReq->IOSer.io_Message.mn_ReplyPort->mp_SigBit;
   waitmask = INPUT_CHARACTER;

   if (!InitMenus()) Cleanup();

   AllocTerm(1);
   uw_read = 1;
   uw_write = 1;

   sercount = 1;
   ReadSer(serbuf, sercount);

/*   UWExit(); */

   problem = TRUE;
   do {
	Wait(waitmask);
	if(CheckIO(SerReadReq))
	{
		WaitIO(SerReadReq);
		UWwrite(serbuf, sercount);
		sercount = QuerySer(SerReadReq);
		if (sercount < 1) sercount = 1;
		ReadSer(serbuf, sercount);
	} /* end if(CheckIO(SerRead)) */
      for (i = 0; i < 8; ++i) {
	if (!uw[i].free) {
		while((!uw[i].free) &&
			((message = (struct IntuiMessage *)
				GetMsg(uw[i].win->UserPort) ) != NULL)) {
				class     = message->Class;
				code      = message->Code;
				qualifier = message->Qualifier;
				ReplyMsg(message);
				problem &= HandleEvent(i,class,code,qualifier);
/*				if(problem == FALSE) break; */
			} /* end while(mess... */

		if((!uw[i].free) && CheckIO(uw[i].ConReadReq))
		{
			WaitIO(uw[i].ConReadReq);
			if (uw_read != i) SelInput(i);
	/* is this is current input window?  if not, send note off ... */
			UWWriteChar(letter[i]);
/*			SerPutChar(SerWriteReq, letter[i]);	*/
			if (!FullDuplex)
				ConPutChar(uw[uw_write].ConWriteReq, letter[i]);
			QueueRead(uw[i].ConReadReq, &letter[i]);
		} /* end if(CheckIO(ConRead)) */
	} /* end if(!uw[i].free) */
      } /* end for i */
   } while (problem); /* keep going as long as HandleEvent returns nonzero */

   AbortIO(SerReadReq);
   Cleanup();
}

Cleanup()
{
   int i;

   for (i = 0; i < 8; ++i)
	if (!uw[i].free) DeallocTerm(i);
   if (SerReadReq) CloseDevice(SerReadReq);
   if (SerWriteReq) DeleteExtIO(SerWriteReq,sizeof(struct IOExtSer));
   if (serWritePort) DeletePort(serWritePort);
   if (SerReadReq) DeleteExtIO(SerReadReq,sizeof(struct IOExtSer));
   if (serReadPort) DeletePort(serReadPort);
   if (MenuHead) DisposeMenus(MenuHead);
   if (IntuitionBase) CloseLibrary(IntuitionBase);
   if (GfxBase) CloseLibrary(GfxBase);
   exit(0);
} /* end of Cleanup */

HandleEvent(term,class,code,qualifier)
int term;
USHORT class;
USHORT code;
USHORT qualifier;
{
      switch( class ) {
	 case CLOSEWINDOW:
	    return(CloseWin(term));
	    break;
         case MENUPICK:
            return(MenuSwitch(code));
            break;
      } /* end of switch( class ) */
      return(TRUE);
} /* end of HandleEvent */

/*      R e a d S e r
 *
 *      Read characters from the serial.device
 *
 */

ReadSer(data,length)
char *data;
int length;
{
  SerReadReq->IOSer.io_Command = CMD_READ;
  SerReadReq->IOSer.io_Length = length;
  SerReadReq->IOSer.io_Data = (APTR)(data);

   BeginIO(SerReadReq);
}

/*      W r i t e S e r
 *
 *      Write characters to the serial.device
 *
 */

WriteSer(data,length)
char *data;
int length;
{
/*  int i; */

  SerWriteReq->IOSer.io_Command = CMD_WRITE;
  SerWriteReq->IOSer.io_Length = length;
/*  SerWriteReq->IOSer.io_Length = 1; 
  for(i = 0; i < length; ++i) {
	SerWriteReq->IOSer.io_Data = (APTR)(data + i);
	DoIO(SerWriteReq);
  }
*/
  SerWriteReq->IOSer.io_Data = (APTR)(data);

  if(DoIO(SerWriteReq) != 0)
  {
      Notify(uw[uw_write].win, "Serial write error.");
  }
}

extern int uwflag;
sputc(c)
char c;
{
/*	printf(">0%o ",c); */
	if (uwflag) Delay(1);	/* Why is this necessary????? */
	SerPutChar(SerWriteReq, c);
}

/*      W r i t e C o n
 *
 *      Write characters to the console.device
 *
 */

WriteCon(data,length)
char *data;
int length;
{
  uw[uw_write].ConWriteReq->io_Command = CMD_WRITE;
  uw[uw_write].ConWriteReq->io_Length = length;
  uw[uw_write].ConWriteReq->io_Data = (APTR)(data);

if(!uw[uw_write].free)
  if(DoIO(uw[uw_write].ConWriteReq) != 0)
  {
      Notify(uw[uw_write].win, "Console write error.");
  }
}


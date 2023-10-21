#file def.arrows.h
unsigned long UPdata[] = {
0x00000000,
0x00780000,
0x01FE0000,
0x07FF8000,
0x00780000,
0x00780000,
0x00780000,
0x00780000,
0x00780000,
0x00000000,

0xFFFFFC00,
0xFFE7FC00,
0xFFF9FC00,
0xFFFE7C00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFFFFC00 };

unsigned long DOWNdata[] = {
0x00000000,
0x00780000,
0x00780000,
0x00780000,
0x00780000,
0x00780000,
0x07FF8000,
0x01FE0000,
0x00780000,
0x00000000,

0xFFFFFC00,
0xFF87FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF7FC00,
0xFFF07C00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00 };

unsigned long RIGHTdata[] = {
0x00000000,
0x00000000,
0x00070000,
0x0007C000,
0x3FFFF000,
0x3FFFF000,
0x0007C000,
0x00070000,
0x00000000,
0x00000000,

0xFFFFFC00,
0xFFFFFC00,
0xFFF8FC00,
0xFFFF3C00,
0xC00FCC00,
0xFFFFEC00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00 };

unsigned long LEFTdata[] = {
0x00000000,
0x00000000,
0x03800000,
0x0F800000,
0x3FFFF000,
0x3FFFF000,
0x0F800000,
0x03800000,
0x00000000,
0x00000000,

0xFFFFFC00,
0xFFFFFC00,
0xFE7FFC00,
0xFF7FFC00,
0xFF000C00,
0xFFFFEC00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00 };

unsigned long UPLEFTdata[] = {
0x00000000,
0x1FF80000,
0x1FE00000,
0x1FE00000,
0x1FF80000,
0x187E0000,
0x001F8000,
0x0007E000,
0x00018000,
0x00000000,

0xFFFFFC00,
0xE007FC00,
0xFFDFFC00,
0xFFDFFC00,
0xFFE7FC00,
0xFFF9FC00,
0xFFFE7C00,
0xFFFF9C00,
0xFFFFFC00,
0xFFFFFC00 };

unsigned long UPRIGHTdata[] = {
0x00000000,
0x007FE000,
0x001FE000,
0x001FE000,
0x007FE000,
0x01F86000,
0x07E00000,
0x1F800000,
0x06000000,
0x00000000,

0xFFFFFC00,
0xFF801C00,
0xFFFF9C00,
0xFFFF9C00,
0xFFFF9C00,
0xFFFF9C00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00,
0xFFFFFC00 };

unsigned long DOWNLEFTdata[] = {
0x00000000,
0x00018000,
0x0007E000,
0x001F8000,
0x187E0000,
0x1FF80000,
0x1FE00000,
0x1FE00000,
0x1FF80000,
0x00000000,

0xFFFFFC00,
0xFFFE7C00,
0xFFFF9C00,
0xFFFFFC00,
0xE7FFFC00,
0xF9FFFC00,
0xFFFFFC00,
0xFF9FFC00,
0xFFE7FC00,
0xFFFFFC00 };

unsigned long DOWNRIGHTdata[] = {
0x00000000,
0x06000000,
0x1F800000,
0x07E00000,
0x01F86000,
0x007FE000,
0x001FE000,
0x0007E000,
0x001FE000,
0x00000000,

0xFFFFFC00,
0xF9FFFC00,
0xFE7FFC00,
0xFF9FFC00,
0xFFE79C00,
0xFFF8DC00,
0xFFFFDC00,
0xFFFFDC00,
0xFFFFDC00,
0xFFFFFC00 };
#file def.menus.h
#define TEXT(nam,str) struct IntuiText nam = {0,1,JAM2,0,0,NULL,str,NULL}
	/* Commands */
	TEXT(T_HELP,  "?   display help");
	TEXT(T_o,     "o   set options");
	TEXT(T_SHELL, "!   AMIGADOS commands");
	TEXT(T_v,     "v   version number");
	TEXT(T_CR,    "^R   redraw screen");
	TEXT(T_CP,    "^P   repeat last message");
	TEXT(T_Q,     "Q   quit game");
	TEXT(T_S,     "S   save the game");

	/* Inventory */
	TEXT(T_i,     "i   inventory");
	TEXT(T_p,     "p   pay your bill");
	TEXT(T_d,     "d   drop an object");
	TEXT(T_D,     "D   Drop several things");
	TEXT(T_COMMA, ",   Pickup an object");
	TEXT(T_SLASH, "/   identify something");
	TEXT(T_c,     "c   call class of objects");
	TEXT(T_C,     "C   Christen a monster");

	/* Actions */
	TEXT(T_a,    "a   apply/use something");
	TEXT(T_e,    "e   eat something");
	TEXT(T_q,    "q   quaff a potion");
	TEXT(T_r,    "r   read a scroll");
	TEXT(T_t,    "t   throw/shoot weapon");
	TEXT(T_z,    "z   zap a wand");

	/* Preparations */
	TEXT(T_w,    "w   wield a weapon");
	TEXT(T_P,    "P   Put on ring");
	TEXT(T_R,    "R   Remove ring");
	TEXT(T_T,    "T   Take off armor");
	TEXT(T_W,    "W   Wear armor");
	TEXT(T_WPN,   ")   current weapon");
	TEXT(T_ARMOR, "[   current armor");
	TEXT(T_RING,  "=   current rings");

	/* Movement */
	TEXT(T_E,     "E   Engrave msg on floor");
	TEXT(T_s,     "s   search");
	TEXT(T_UP,    "<   Go up stairs");
	TEXT(T_DOWN,  ">   Go down stairs");
	TEXT(T_WAIT,  ".   wait a moment");
	TEXT(T_CT,    "^T   Teleport");

#define IFLAGS ITEMENABLED|ITEMTEXT|HIGHCOMP
#define IDATA(str,off) 0,off,200,9,IFLAGS,0,(APTR)&str,NULL,NULL,NULL,NULL

struct MenuItem cmdsub[] = {
	{ &cmdsub[1], IDATA(T_HELP,   0) },
	{ &cmdsub[2], IDATA(T_o,     10) },
	{ &cmdsub[3], IDATA(T_SHELL, 20) },
	{ &cmdsub[4], IDATA(T_v,     30) },
	{ &cmdsub[5], IDATA(T_CR,    40) },
	{ &cmdsub[6], IDATA(T_CP,    50) },
	{ &cmdsub[7], IDATA(T_Q,     60) },
	{ NULL,       IDATA(T_S,     70) } };

struct MenuItem invsub[] = {
	{ &invsub[1], IDATA(T_i,      0) },
	{ &invsub[2], IDATA(T_p,     10) },
	{ &invsub[3], IDATA(T_d,     20) },
	{ &invsub[4], IDATA(T_D,     30) },
	{ &invsub[5], IDATA(T_COMMA, 40) },
	{ &invsub[6], IDATA(T_SLASH, 50) },
	{ &invsub[7], IDATA(T_c,     60) },
	{ NULL,       IDATA(T_C,     70) } };

struct MenuItem actsub[] = {
	{ &actsub[1], IDATA(T_a,    0) },
	{ &actsub[2], IDATA(T_e,    10) },
	{ &actsub[3], IDATA(T_q,    20) },
	{ &actsub[4], IDATA(T_r,    30) },
	{ &actsub[5], IDATA(T_t,    40) },
	{ NULL,       IDATA(T_z,    50) } };

struct MenuItem armsub[] = {
	{ &armsub[1], IDATA(T_w,      0) },
	{ &armsub[2], IDATA(T_P,     10) },
	{ &armsub[3], IDATA(T_R,     20) },
	{ &armsub[4], IDATA(T_T,     30) },
	{ &armsub[5], IDATA(T_W,     40) },
	{ &armsub[6], IDATA(T_WPN,   50) },
	{ &armsub[7], IDATA(T_ARMOR, 60) },
	{ NULL,       IDATA(T_RING,  70) } };

struct MenuItem movsub[] = {
	{ &movsub[1], IDATA(T_E,    0) },
	{ &movsub[2], IDATA(T_s,    10) },
	{ &movsub[3], IDATA(T_UP,   20) },
	{ &movsub[4], IDATA(T_DOWN, 30) },
	{ &movsub[5], IDATA(T_WAIT, 40) },
	{ NULL,       IDATA(T_CT,   50) } };

struct Menu HackMenu[] = {
   { &HackMenu[1], 10,0, 80,0,MENUENABLED,"Commands",     &cmdsub[0] },
   { &HackMenu[2], 90,0, 80,0,MENUENABLED,"Inventory",    &invsub[0] },
   { &HackMenu[3],180,0, 80,0,MENUENABLED,"Actions",      &actsub[0] },
   { &HackMenu[4],270,0,100,0,MENUENABLED,"Preparations", &armsub[0] },
   { NULL,        390,0, 80,0,MENUENABLED,"Movement",     &movsub[0] } };

char menukey[5][10] = {
	{
	'?',   /*   display help */
	'o',   /*   set options */
	'!',   /*   AMIGADOS commands */
	'v',   /*   version number */
	022,   /*R   redraw screen */
	024,   /*P   repeat last message */
	'Q',   /*   quit game */
	'S',   /*   save the game */
	},{
	/* Inventory */
	'i',   /*   inventory */
	'p',   /*   pay your bill */
	'd',   /*   drop an object */
	'D',   /*   Drop several things */
	',',   /*   Pickup an object */
	'/',   /*   identify something */
	'c',   /*   call a class of objects */
	'C',   /*   Christen a monster */
	},{
	/* Actions */
	'a',   /*   apply/use something */
	'e',   /*   eat something */
	'q',   /*   quaff a potion */
	'r',   /*   read a scroll */
	't',   /*   throw/shoot weapon */
	'z',   /*   zap a wand */
	},{
	/* Preparations */
	'w',   /*   wield a weapon */
	'P',   /*   Put on ring */
	'R',   /*   Remove ring */
	'T',   /*   Take off armor */
	'W',   /*   Wear armor */
	')',   /*   current weapon */
	'[',   /*   current armor */
	'=',   /*   current rings */
	},{
	/* Movement */
	'E',   /*   Engrave msg on floor */
	's',   /*   search */
	'<',   /*   Go up stairs */
	'>',   /*   Go down stairs */
	'.',   /*   wait a moment */
	024,   /*   Teleport */
	} };
#file hack.window.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <exec/types.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <stdio.h>
#include "hack.h"   /* for ROWNO and COLNO */
#include "def.menus.h"

#define XSIZE 8
#define YSIZE 8
#define BASEX 4
#define BASEY 4   /* should be -4 */

#define ICON_REV       0
#define GRAPHICS_REV   29
#define INTUITION_REV  29

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
char *IconBase;
int mousex, mousey;

#define BUFFERED 512
char out_buffer[BUFFERED];
int bufcnt;
char *out_bufp;

#ifdef STUPIDARROWS
#include "def.arrows.h"
struct Image URImage = { 0,0,22,10,2,(short *)&UPRIGHTdata,   0x3, 0, NULL };
struct Image DRImage = { 0,0,22,10,2,(short *)&DOWNRIGHTdata, 0x3, 0, NULL };
struct Image RImage  = { 0,0,22,10,2,(short *)&RIGHTdata,     0x3, 0, NULL };
struct Image DImage  = { 0,0,22,10,2,(short *)&DOWNdata,      0x3, 0, NULL };
struct Image UImage  = { 0,0,22,10,2,(short *)&UPdata,        0x3, 0, NULL };
struct Image LImage  = { 0,0,22,10,2,(short *)&LEFTdata,      0x3, 0, NULL };
struct Image DLImage = { 0,0,22,10,2,(short *)&DOWNLEFTdata,  0x3, 0, NULL };
struct Image ULImage = { 0,0,22,10,2,(short *)&UPLEFTdata,    0x3, 0, NULL };

struct Gadget URGadget = { NULL,      436, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&URImage, NULL, NULL, 0, NULL, 'u', NULL };
struct Gadget DRGadget = { &URGadget, 568, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&DRImage, NULL, NULL, 0, NULL, 'n', NULL };
struct Gadget RGadget  = { &DRGadget, 490, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&RImage, NULL, NULL, 0, NULL, 'l', NULL };
struct Gadget DGadget  = { &RGadget,  544, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&DImage, NULL, NULL, 0, NULL, 'j', NULL };
struct Gadget UGadget  = { &DGadget,  412, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&UImage, NULL, NULL, 0, NULL, 'k', NULL };
struct Gadget LGadget  = { &UGadget,  466, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&LImage, NULL, NULL, 0, NULL, 'h', NULL };
struct Gadget DLGadget = { &LGadget,  520, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&DLImage, NULL, NULL, 0, NULL, 'b', NULL };
struct Gadget ULGadget = { &DLGadget, 388, 0, 22,10,
			GADGHCOMP | GADGIMAGE, GADGIMMEDIATE, BOOLGADGET,
			(APTR)&ULImage, NULL, NULL, 0, NULL, 'y', NULL };
#endif

struct TextAttr HackFont =
	{ "topaz.font", TOPAZ_EIGHTY,FS_NORMAL, FPF_ROMFONT};

struct NewScreen NewHackScreen = {
0, 0, 640, 200, 3, 0, 1, HIRES, CUSTOMSCREEN, &HackFont,
"HACK V1.0.1a - Ported by  John A. Toebes, VIII", NULL, NULL };

struct Window *HackWindow;
struct Screen *HackScreen;

struct NewWindow NewHackWindow = {
  0,1,640,199, -1,-1,   /* left, top, width, height, detailpen, blockpen */
  MOUSEBUTTONS | CLOSEWINDOW | RAWKEY | MENUPICK
#ifdef STUPIDARROWS
 | GADGETDOWN
#endif
  ,WINDOWDEPTH | WINDOWCLOSE | ACTIVATE | SIMPLE_REFRESH,
#ifdef STUPIDARROWS
  &ULGadget,
#else
  NULL,
#endif
NULL, "HACK V1.0.1a - Ported by John A. Toebes, VIII",
  NULL, NULL, 640,200,640,200, CUSTOMSCREEN };

struct IOStdReq consoleIO;

#define HO "\x9BH"
#define CL "\x0C"
#define CE "\x9BK"
#define UP "\x0B"
#define CM "\x9B%d;%dH"
#define ND "\x09"
#define XD "\x9BB"
#define BC "\x08"
#define SO "\x9B4m"
#define SE "\x9B0m"
#define BELL 7
int myx, myy;

startup()
{
}

/* Cursor movements */
extern xchar curx, cury;

curs(x,y)
register int x,y;   /* not xchar: perhaps xchar is unsigned and
            curx-x would be unsigned as well */
{
   if (y != cury || x != curx)
	myprintf(CM, y, x);
   curx = x;
   cury = y;
}

cl_end() {
	myprintf(CE);
}

clear_screen() {
	myprintf(CL);
	curx = cury = 1;
}

home()
{
	myprintf(HO);
	curx = cury = 1;
}

standoutbeg()
{
	myprintf(SO);
}

standoutend()
{
	myprintf(SE);
}

backsp()
{
	myprintf(BC);
	curx--;
}

bell()
{
	myputchar(BELL);
}

delay_output()
{
   /* delay 40 ms, 50 ticks/sec    */
/*   Delay (2); */
}

initterm()
   {
#ifdef DEBUGIT
	printf("intuition.library?\n");
	fflush(stdout);
#endif
   if ( (IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", INTUITION_REV)) == NULL)
	_exit(2);

#ifdef DEBUGIT
	printf("graphics.library?\n");
	fflush(stdout);
#endif
   if ( (GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library",GRAPHICS_REV)) == NULL)
	_exit(3);

#ifdef DEBUGIT
	printf("icon.library?\n");
	fflush(stdout);
#endif
   if ( (IconBase = (char *)
	OpenLibrary("icon.library",ICON_REV)) == NULL)
	_exit(4);

   if ( (HackScreen = (struct Screen *)
	OpenScreen(&NewHackScreen)) == NULL)
	_exit(5);

   NewHackWindow.Screen = HackScreen;

#ifdef DEBUGIT
	printf("OpenWindow?\n");
	fflush(stdout);
#endif
   if ( (HackWindow = (struct Window *)
	OpenWindow(&NewHackWindow)) == NULL)
	{
	CloseScreen(HackScreen);
	_exit(6);
	}

#ifdef DEBUGIT
	printf("menu strip?\n");
	fflush(stdout);
#endif
   SetMenuStrip(HackWindow,&HackMenu);

#ifdef DEBUGIT
	printf("console.device?\n");
	fflush(stdout);
#endif
   consoleIO.io_Data = (APTR) HackWindow;
   consoleIO.io_Length = sizeof(*HackWindow);
   if (OpenDevice("console.device",0, &consoleIO, 0) != 0)
	hackexit(7);

#ifdef DEBUGIT
	printf("doneinit\n");
	fflush(stdout);
#endif
   bufcnt = 0;
   out_bufp = out_buffer;
   }

hackexit(code)
int code;
   {
   CloseDevice(&consoleIO);
   ClearMenuStrip(HackWindow);
   CloseWindow(HackWindow);
   CloseScreen(HackScreen);
   CloseLibrary(IconBase);
   CloseLibrary(GfxBase);
   CloseLibrary(IntuitionBase);
   _exit(code);
   }

myfflush()
   {
	register int dummy1, dummy2;
   if (bufcnt)
	{
	consoleIO.io_Command = CMD_WRITE;
	consoleIO.io_Data = (APTR)out_buffer;
	consoleIO.io_Length = bufcnt;
	DoIO(&consoleIO);
	}
   bufcnt = 0;
   }

myputchar(c)
char c;
   {
	if (bufcnt == BUFFERED)
		myfflush();

	out_buffer[bufcnt++] = c;
   }

myputs(str)
char *str;
   {
	register int dummy1, dummy2;
	int len, tocopy;

	len = strlen(str);

	if (len >= BUFFERED)
		{
		myfflush();
		consoleIO.io_Command = CMD_WRITE;
		consoleIO.io_Data = (APTR)str;
		consoleIO.io_Length = len;
		DoIO(&consoleIO);
		}
	else
		{
		if (bufcnt+len >= BUFFERED) /* is there room */
			{
			tocopy = BUFFERED - bufcnt;
			movmem(str, &out_buffer[bufcnt], tocopy);
			bufcnt += tocopy;
			len -= tocopy;
			str += tocopy;
			myfflush();
			}
		if (len)
			{
			/* just move it in */
			movmem(str, &out_buffer[bufcnt], len);
			bufcnt += len;
			}
		}
	myputchar('\n');
   }

/*VARARGS1*/
myprintf(str,a1,a2,a3,a4,a5,a6,a7,a8,a9)
char *str,*a1,*a2,*a3,*a4,*a5,*a6,*a7,*a8,*a9;
   {
	char buf[BUFFERED], *bptr;
	int len, tocopy;

	bptr = &buf;
	len = (int)sprintf(bptr,str,a1,a2,a3,a4,a5,a6,a7,a8,a9);
	if (bufcnt+len >= BUFFERED) /* is there room */
		{
		tocopy = BUFFERED - bufcnt;
		movmem(bptr, &out_buffer[bufcnt], tocopy);
		bufcnt += tocopy;
		len -= tocopy;
		bptr += tocopy;
		myfflush();
		}
	if (len)
		{
		/* just move it in */
		movmem(bptr, &out_buffer[bufcnt], len);
		bufcnt += len;
		}
   }

inchar()
   {
	register int dummy1, dummy2;
	struct IntuiMessage *Message, *GetMsg();
	int c;
	USHORT thismenu, menusel;
#ifdef STUPIDARROWS
   struct Gadget *gadget;
#endif
   struct MenuItem *item, *ItemAddress();
   c = 0;
   while(!c)
      {
      while( (Message = GetMsg(HackWindow->UserPort)) == NULL)
         Wait( 1 << HackWindow->UserPort->mp_SigBit );

      switch(Message->Class)
	{
	case MENUPICK:
		menusel = thismenu = Message->Code;
		while(thismenu != MENUNULL)
		   {
		   menusel = thismenu;
		   item = ItemAddress(&HackMenu, thismenu);
		   thismenu = item->NextSelect;
		   }
		if (menusel != MENUNULL)
		   c = menukey[MENUNUM(menusel)][ITEMNUM(menusel)];
		break;
	case MOUSEBUTTONS:
		mousex = ( (Message->MouseX) + BASEX ) / XSIZE;
 		mousey = ( (Message->MouseY) - BASEY ) / YSIZE;
		if (mousex > 0 && mousey > 0 &&
		    mousex <= COLNO && mousey <= ROWNO )
			{
			if (Message->Code == SELECTDOWN)
				c = MDOWN;
			else if (Message->Code == SELECTUP)
				c = MUP;
			}
		break;
	case CLOSEWINDOW:
		c = 'Q';
		break;
#ifdef STUPIDARROWS
	case GADGETDOWN:
		gadget = (struct Gadget *)Message->IAddress;
		c = gadget->GadgetID;
		break;
#endif
	case RAWKEY:
		c = cnvrtkey(Message->Code,Message->Qualifier);
		break;
	default:
		c = 'Q';
		break;
	}
      ReplyMsg(Message);
      }
   return(c);
   }

#define NORMAL 0
#define SHIFTED 1
#define CONTROL 2
#define ALTED 3
short lookup[4][96] =
{
/* unshifted table */
	'`',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	'0',	'-',	'=',	'\\',	0,	'0',
	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
	'o',	'p',	'[',	']',	0,	'b',	'j',	'n',
	'a',	's',	'd',	'f',	'g',	'h',	'j',	'k',
	'l',	';',	'\'',	0,	0,	'h',	'.',	'l',
	0,	'z',	'x',	'c',	'v',	'b',	'n',	'm',
	',',	'.',	'/',	0,	'.',	'y',	'k',	'u',
	' ',	8,	'i',	'\n',	'\n',	022,	8,	0,
	0,	0,	'-',	0,	'k',	'j',	'l',	'h',
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	'?',

/* shifted table */
	'~',	'!',	'@',	'#',	'$',	'%',	'^',	'&',
	'*',	'(',	')',	'_',	'+',	'|',	0,	'0',
	'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
	'O',	'P',	'{',	'}',	0,	'B',	'J',	'N',
	'A',	'S',	'D',	'F',	'G',	'H',	'J',	'K',
	'L',	':',	'"',	0,	0,	'H',	'.',	'L',
	0,	'Z',	'X',	'C',	'V',	'B',	'N',	'M',
	'<',	'>',	'?',	0,	'.',	'Y',	'K',	'U',
	' ',	'H',	'I',	'\N',	'\N',	022,	'H',	0,
	0,	0,	'-',	0,	'K',	'J',	'L',	'H',
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	'?',

/* controlled table */
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	'Q',	0,	0,
	0,	0,	0,	022,	024,	0,	0,	0,
	0,	020,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	'?',

/* alted table */
	'`',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	'0',	'-',	'=',	'\\',	0,	'0',
	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
	'o',	'p',	'[',	']',	0,	'b',	'j',	'n',
	'a',	's',	'd',	'f',	'g',	'h',	'j',	'k',
	'l',	';',	'\'',	0,	0,	'h',	'.',	'l',
	0,	'z',	'x',	'c',	'v',	'b',	'n',	'm',
	',',	'.',	'?',	0,	'.',	'y',	'k',	'u',
	' ',	'h',	'i',	'\n',	'\n',	022,	'h',	0,
	0,	0,	'-',	0,	'k',	'j',	'l',	'h',
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	'?'
};

int cnvrtkey(code, qual )
USHORT code, qual;
   {
   int table;

   if (code > 0x5f)
      return(0);

   if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
      table = SHIFTED;
   else if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
       table = ALTED;
   else if (qual & (IEQUALIFIER_CONTROL))
      table = CONTROL;
   else
      table = NORMAL;
   return((int)lookup[table][code]);
   }
#file hack.termcap.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include "config.h"   /* for ROWNO and COLNO */

#define HEIGHT 8
#define WIDTH  8
#define TOP    16
#define LEFT   3

startup()
{
}

/* Cursor movements */
extern xchar curx, cury;

curs(x,y)
register int x,y;   /* not xchar: perhaps xchar is unsigned and
            curx-x would be unsigned as well */
{
   if (y == cury && x == curx) return;
   cmov(x,y);
}

nocmov(x,y)
{
  cmov(x,y);   /* always go to the requested position */
}

cmov(x,y)
register int x,y;
{
   setxy(LEFT+x*WIDTH,TOP+y*HEIGHT);
   cury = y;
   curx = x;
}


cl_end() {
   weraeol();
}

clear_screen() {
   /* printf(CL); */
   home();
}

home()
{
   setxy(TOP,LEFT);
   curx = cury = 1;
}

standoutbeg()
{
   /* printf(SO); */
}

standoutend()
{
   /* printf(SE); */
}

backsp()
{
   cmov(curx-1,cury);
}

bell()
{
    /* putchar('\007'); */
}

delay_output()
{
   /* delay 40 ms, 50 ticks/sec    */
   Delay (2);
}

#file hack.graphics.c
/* Copyright (c) John A. Toebes, VIII 1986 */
#include <exec/types.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <graphics/view.h>
#include <stdio.h>
#include <fcntl.h>
#include "config.h"
#include "hack.h"   /* for ROWNO and COLNO */

#define XSIZE 8
#define YSIZE 8
#define BASEX (-4)
#define BASEY 19
#define PICSIZE (8*4)
#define USEDPLANES 3
extern struct Window *HackWindow;
extern struct Screen *HackScreen;
UWORD colormap[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
USHORT mondata[] = 
  { 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
    0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
    0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
    0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00 };
struct Image monImage = 
   { 0, 0, 8, 8, 3, &mondata[0], 0x0f, 0x0, NULL };
char *monpics;
char basemon;

InitGraphics()
   {
   char maxchar;
   int file;
   int size;
   int i;
   register int dummy1, dummy2;
   if ( (file = open(HACKCSET, O_RDONLY)) == -1 )
	panic("Cannot Open Graphics Characters");
/*   cm = GetColorMap(16);
   HackScreen->ViewPort.ColorMap = cm; */
   mread(file,colormap, 32);
   for (i=0; i<8; i++)
	SetRGB4(&HackScreen->ViewPort, i,
		(colormap[i] >> 8) & 0x0f,
		(colormap[i] >> 4) & 0x0f,
		(colormap[i]) &0x0f);
   mread(file,&basemon,1);
   mread(file,&maxchar,1);
   size = (maxchar-basemon)*PICSIZE;
   if ((monpics = (char *)malloc(size)) == NULL)
	panic("Cannot get char area");
   mread(file,monpics,size);
   }

at(x,y,ch)
register xchar x,y;
char ch;
   {
   char *thisone;
   int i;

   /* first construct the picture */
   thisone = monpics+PICSIZE*(ch-basemon);
   for (i=0; i<PICSIZE; i++)
	mondata[i] = thisone[i] << 8;
   if ((curx == x) & (cury == (y+2)) )
	home();
   myfflush();
   DrawImage(HackWindow->RPort, &monImage, BASEX+(x*XSIZE), BASEY+(y*YSIZE));
   }
#file hack.icon.c
#include <workbench/startup.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>

extern struct WBStartup *WBenchMsg;
extern char pl_character[];
extern char plname[];

geticon()
   {
   struct WBArg *argp;
   char *argname;

   argp = WBenchMsg->sm_ArgList;
   if (WBenchMsg->sm_NumArgs > 1)
	{
	argname = (argp+1)->wa_Name;
	pl_character[0] = pl_character[1] = 0;
	/* argp now points to the name */
	if      (!strcmp(argname, "Wizard"      )) pl_character[0] = 'W';
	else if (!strcmp(argname, "Speliologist")) pl_character[0] = 'S';
	else if (!strcmp(argname, "Tourist"     )) pl_character[0] = 'T';
	else if (!strcmp(argname, "Fighter"     )) pl_character[0] = 'F';
	else if (!strcmp(argname, "Knight"      )) pl_character[0] = 'K';
	else if (!strcmp(argname, "Caveman"     )) pl_character[0] = 'C';
	else strcpy(plname, argname);
	}
   }

makeicon(name,type)
char *name;
char type;
   {
   char *source;
   struct DiskObject *object;

   switch(type) {
	case 'w':
	case 'W':	source = "Wizard";
			break;
	case 's':
	case 'S':	source = "Speliologist";
			break;
	case 't':
	case 'T':	source = "Tourist";
			break;
	case 'f':
	case 'F':	source = "Fighter";
			break;
	case 'k':
	case 'K':	source = "Knight";
			break;
	case 'c':
	case 'C':	source = "Caveman";
			break;

	default:	source = "HACK";
			break;
	}
	if ( (object = GetDiskObject(source)) == NULL)
		myprintf("Cannot get source icon - err #%d\n", IoErr() );
	object->do_CurrentX = NO_ICON_POSITION;
	object->do_CurrentY = NO_ICON_POSITION;
	if ( (object = PutDiskObject( name, object )) == NULL)
		myprintf("Cannot create save icon - err #%d\n", IoErr() );
	FreeDiskObject(object);
   }

delicon(name)
   {
   struct WBObject *object;
   char tempname[100];  /* should hold any file name */

   strcpy(tempname,name);
   strcat(tempname,".info");
   if ( unlink(tempname) == -1)
	myprintf("Cannot delete .info file\n");
   }
#file qsort.c
qsort( v, n, size, comp)
char *v;
int n;
int size;
int (*comp)();
   {
   int gap, i, j, x, cnt;
   char temp, *p1, *p2;

   cnt = 0;
   for (gap=n/2; gap > 0 ; gap /= 2)
      for (i=gap; i<n; i++)
         for (j = i-gap; j >= 0; j -= gap)
            if ( (*comp) ( (p1=v+j*size), (p2=v+(j+gap)*size) ) < 0)
               {
               cnt++;
               /* exchange them */
               for (x=0; x<size; x++)
                  {
                  temp = *p1;
                  *p1++ = *p2;
                  *p2++ = temp;
                  }
               }
   return(cnt);
   }
#file unixxface.c
#include <stdio.h>
#include <libraries/dosextens.h>

int getpid()
   {
   long now[3];

   DateStamp(&now);

   return(now[0] ^ now[1] ^ now[2]);
   }

int *signal(num,func)
int num;
int *func;
   {
   return(NULL);
   }


getenv(var)
char *var;
   {
   return(NULL);
   }


execl()
   {
   /* this should flag an error */
   }


chdir(dir)
char *dir;
   {
   struct FileLock *lock;

   if ( (lock = Lock(dir, ACCESS_READ)) == NULL)
	return(1);  /* cannot find the directory */

   lock = CurrentDir( lock );

   if (lock)
	UnLock(lock);

   /* change to the desired directory */
   return(0);  /* phoney success */
   }


char *getlogin()
   {
   /* return the login name - perhaps we can use getenv */
   return (NULL);
   }

perror(string)
char *string;
   {
   myprintf("Call to perror for '%s'\n", string);
   }

char *index(p,c)
char *p;
char c;
   {
   char *strchr();

   return(strchr(p,c));
   }

char *rindex(p,c)
char *p;
char c;
   {
   char *strrchr();

   return(strrchr(p,c));
   }
#file _main.c
#include <stdio.h>
#include <ctype.h>
#include <ios1.h>

#include "workbench/startup.h"
#include "libraries/dos.h"

#define MAXARG 32              /* maximum command line arguments */

#ifndef TINY
extern int _stack,_fmode,_iomode;
#endif
extern int LoadAddress;

extern struct UFB _ufbs[];
int argc;			/* arg count */
char *argv[MAXARG];		/* arg pointers */

#define MAXWINDOW 40
extern struct WBStartup *WBenchMsg;
static char window[MAXWINDOW] = "con:10/10/320/80/";

/**
*
* name         _main - process command line, open files, and call "main"
*
* synopsis     _main(line);
*              char *line;     ptr to command line that caused execution
*
* description	This function performs the standard pre-processing for
*		the main module of a C program.  It accepts a command
*		line of the form
*
*			pgmname arg1 arg2 ...
*
*		and builds a list of pointers to each argument.  The first
*		pointer is to the program name.  For some environments, the
*		standard I/O files are also opened, using file names that
*		were set up by the OS interface module XCMAIN.
*
**/
_main(line)
char *line;
{
char c;
int x;

/*
*
* Build argument pointer list
*
*/
for(argc = 0; argc < MAXARG; )
	{
	while(isspace(*line)) line++;
	if(*line == '\0') break;
	argv[argc++] = line;
	while((*line != '\0') && (isspace(*line) == 0)) line++;
	c = *line;
	*line++ = '\0';
	if(c == '\0') break;
	}

_ufbs[0].ufbflg |= UFB_OP | UFB_RA | UFB_NC;
_ufbs[1].ufbflg |= UFB_OP | UFB_WA | UFB_NC;
_ufbs[2].ufbflg |= UFB_OP | UFB_WA ;

/*
*
* Call user's main program
*
*/
#ifdef DEBUG
printf("load address = %lx\n",LoadAddress);
Debug(0);
#endif

main(argc,argv);              /* call main function */
_exit(0);
}

#file link_hack
FROM HACK_game:lib/c.o+     *
HACK_source:_main.o+        *
HACK_source:savelev.o+      *
HACK_source:hack.trap.o+    *
HACK_source:hack.save.o+    *
HACK_source:hack.o+         *
HACK_source:hack.rumors.o+  *
HACK_source:hack.end.o+     *
HACK_source:hack.apply.o+   *
HACK_source:hack.mhitu.o+   *
HACK_source:hack.o_init.o+  *
HACK_source:hack.worm.o+    *
HACK_source:hack.do.o+      *
HACK_source:hack.wield.o+   *
HACK_source:hack.pri.o+     *
HACK_source:hack.invent.o+  *
HACK_source:hack.version.o+ *
HACK_source:hack.u_init.o+  *
HACK_source:hack.vault.o+   *
HACK_source:hack.eat.o+     *
HACK_source:hack.dog.o+     *
HACK_source:hack.timeout.o+ *
HACK_source:rnd.o+          *
HACK_source:hack.cmdlist.o+ *
HACK_source:hack.options.o+ *
HACK_source:hack.topl.o+    *
HACK_source:hack.mkobj.o+   *
HACK_source:hack.monst.o+   *
HACK_source:hack.stat.o+    *
HACK_source:hack.steal.o+   *
HACK_source:hack.makemon.o+ *
HACK_source:mklv.shknam.o+  *
HACK_source:hack.track.o+   *
HACK_source:hack.zap.o+     *
HACK_source:hack.do_wear.o+ *
HACK_source:mklv.shk.o+     *
HACK_source:hack.objnam.o+  *
HACK_source:hack.worn.o+    *
HACK_source:hack.lev.o+     *
HACK_source:hack.shk.o+     *
HACK_source:hack.whatis.o+  *
HACK_source:hack.bones.o+   *
HACK_source:hack.read.o+    *
HACK_source:hack.Decl.o+    *
HACK_source:hack.search.o+  *
HACK_source:hack.do_name.o+ *
HACK_source:mklv.makemaz.o+ *
HACK_source:hack.main.o+    *
HACK_source:alloc.o+        *
HACK_source:hack.fight.o+   *
HACK_source:hack.tty.o+     *
HACK_source:UnixXface.o+    *
HACK_source:hack.engrave.o+ *
HACK_source:mklev.o+        *
HACK_source:hack.mon.o+     *
HACK_source:qsort.o+        *
HACK_source:hack.window.o+  *
HACK_source:hack.graphics.o+*
HACK_source:hack.icon.o+    *
HACK_source:hack.rip.o

TO      HACK_game:Hack
LIBRARY HACK_game:lib/lc.lib+HACK_game:lib/amiga.lib
MAP     nil:
#file ccall
stack 10000
lc1 -ii: -cw -oram: hack.window
lc2 -v -ohack_source: ram:hack.window
lc1 -ii: -cw -oram: hack.icon
lc2 -v -ohack_source: ram:hack.icon
lc1 -ii: -cw -oram: unixxface
lc2 -v -ohack_source: ram:unixxface
lc1 -ii: -cw -oram: -dTINY _main
lc2 -v -ohack_source: ram:-dTINY _main
lc1 -ii: -cw -oram: hack.save
lc2 -v -ohack_source: ram:hack.save
lc1 -ii: -cw -oram: hack.trap
lc2 -v -ohack_source: ram:hack.trap
lc1 -ii: -cw -oram: hack
lc2 -v -ohack_source: ram:hack
lc1 -ii: -cw -oram: hack.rumors
lc2 -v -ohack_source: ram:hack.rumors
lc1 -ii: -cw -oram: hack.end
lc2 -v -ohack_source: ram:hack.end
lc1 -ii: -cw -oram: hack.apply
lc2 -v -ohack_source: ram:hack.apply
lc1 -ii: -cw -oram: hack.o_init
lc2 -v -ohack_source: ram:hack.o_init
lc1 -ii: -cw -oram: hack.mhitu
lc2 -v -ohack_source: ram:hack.mhitu
lc1 -ii: -cw -oram: hack.worm
lc2 -v -ohack_source: ram:hack.worm
lc1 -ii: -cw -oram: hack.do
lc2 -v -ohack_source: ram:hack.do
lc1 -ii: -cw -oram: hack.pri
lc2 -v -ohack_source: ram:hack.pri
lc1 -ii: -cw -oram: hack.invent
lc2 -v -ohack_source: ram:hack.invent
lc1 -ii: -cw -oram: hack.wield
lc2 -v -ohack_source: ram:hack.wield
lc1 -ii: -cw -oram: hack.version
lc2 -v -ohack_source: ram:hack.version
lc1 -ii: -cw -oram: hack.u_init
lc2 -v -ohack_source: ram:hack.u_init
lc1 -ii: -cw -oram: hack.vault
lc2 -v -ohack_source: ram:hack.vault
lc1 -ii: -cw -oram: hack.dog
lc2 -v -ohack_source: ram:hack.dog
lc1 -ii: -cw -oram: hack.eat
lc2 -v -ohack_source: ram:hack.eat
lc1 -ii: -cw -oram: hack.timeout
lc2 -v -ohack_source: ram:hack.timeout
lc1 -ii: -cw -oram: rnd
lc2 -v -ohack_source: ram:rnd
lc1 -ii: -cw -oram: hack.options
lc2 -v -ohack_source: ram:hack.options
lc1 -ii: -cw -oram: hack.cmdlist
lc2 -v -ohack_source: ram:hack.cmdlist
lc1 -ii: -cw -oram: qsort
lc2 -v -ohack_source: ram:qsort
lc1 -ii: -cw -oram: hack.zap
lc2 -v -ohack_source: ram:hack.zap
lc1 -ii: -cw -oram: hack.do_wear
lc2 -v -ohack_source: ram:hack.do_wear
lc1 -ii: -cw -oram: mklv.shk
lc2 -v -ohack_source: ram:mklv.shk
lc1 -ii: -cw -oram: hack.objnam
lc2 -v -ohack_source: ram:hack.objnam
lc1 -ii: -cw -oram: hack.worn
lc2 -v -ohack_source: ram:hack.worn
lc1 -ii: -cw -oram: hack.lev
lc2 -v -ohack_source: ram:hack.lev
lc1 -ii: -cw -oram: hack.shk
lc2 -v -ohack_source: ram:hack.shk
lc1 -ii: -cw -oram: hack.whatis
lc2 -v -ohack_source: ram:hack.whatis
lc1 -ii: -cw -oram: hack.bones
lc2 -v -ohack_source: ram:hack.bones
lc1 -ii: -cw -oram: hack.read
lc2 -v -ohack_source: ram:hack.read
lc1 -ii: -cw -oram: hack.Decl
lc2 -v -ohack_source: ram:hack.Decl
lc1 -ii: -cw -oram: hack.search
lc2 -v -ohack_source: ram:hack.search
lc1 -ii: -cw -oram: hack.do_name
lc2 -v -ohack_source: ram:hack.do_name
lc1 -ii: -cw -oram: mklev
lc2 -v -ohack_source: ram:mklev
lc1 -ii: -cw -oram: mklv.makemaz
lc2 -v -ohack_source: ram:mklv.makemaz
lc1 -ii: -cw -oram: hack.main
lc2 -v -ohack_source: ram:hack.main
lc1 -ii: -cw -oram: alloc
lc2 -v -ohack_source: ram:alloc
lc1 -ii: -cw -oram: hack.fight
lc2 -v -ohack_source: ram:hack.fight
lc1 -ii: -cw -oram: hack.tty
lc2 -v -ohack_source: ram:hack.tty
lc1 -ii: -cw -oram: hack.engrave
lc2 -v -ohack_source: ram:hack.engrave
lc1 -ii: -cw -oram: hack.mon
lc2 -v -ohack_source: ram:hack.mon
lc1 -ii: -cw -oram: hack.rip
lc2 -v -ohack_source: ram:hack.rip
lc1 -ii: -cw -oram: hack.topl
lc2 -v -ohack_source: ram:hack.topl
lc1 -ii: -cw -oram: hack.mkobj
lc2 -v -ohack_source: ram:hack.mkobj
lc1 -ii: -cw -oram: hack.monst
lc2 -v -ohack_source: ram:hack.monst
lc1 -ii: -cw -oram: savelev
lc2 -v -ohack_source: ram:savelev
lc1 -ii: -cw -oram: hack.stat
lc2 -v -ohack_source: ram:hack.stat
lc1 -ii: -cw -oram: hack.steal
lc2 -v -ohack_source: ram:hack.steal
lc1 -ii: -cw -oram: mklv.shknam
lc2 -v -ohack_source: ram:mklv.shknam
lc1 -ii: -cw -oram: hack.makemon
lc2 -v -ohack_source: ram:hack.makemon
lc1 -ii: -cw -oram: hack.track
lc2 -v -ohack_source: ram:hack.track

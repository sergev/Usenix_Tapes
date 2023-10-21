/*
amiga/tutorial #88, from cheath, 13531 chars, Sat Oct 26 21:49:20 1985

--------------------------
* amigademo.c
*   Contains some example code for graphics and I/O,
* organized as simple benchmarks.  This is NOT the
* final vesion of these tests; it will be posted at a
* later date.
*
*             Copyright 1985 by MicroSmiths.
*   Permission is granted to use this code as an example for
* personal or commercial use.  If you use these as a benchmark
*   for publication, please credit MicroSmiths as the origin.
*
*****************************************************/

#include "exec/types.h"
#include "exec/devices.h"
#include "intuition/intuition.h"


#include "exec/memory.h"      /* Need for AllocMem */

#define INTUITION_REV   29
#define GRAPHICS_REV    29

#define WDTH 640   /* For a 640 * 200 , 16 color display   */
#define HGHT 200
#define DPTH 2

#define VIEW_MODE HIRES /* Set this for a 640 screen  */

#define NL NULL

#define VID MEMF_CHIP | MEMF_CLEAR     /* For allocating blitter-ram   */

/************************************************************************/
/* These structures are included to avoid compile-time warnings.        */
/* This is safe UNLESS YOU DECLARE A STRUCTURE THAT REQUIRES THEM.      */
/************************************************************************/

struct CopList  { int fake; };   /* graphics/copper.h  */
struct UCopList { int fake; };
struct cprlist  { int fake; };
struct Region   { int fake; };   /* graphics/regions.h  */
struct VSprite  { int fake; };   /* graphics/gels.h     */
struct collTable {int fake; };
struct KeyMap   { int fake; };
   
/* These librarie handles are needed. No need to include a file ferem.. */
struct GfxBase       { int fake; } *GfxBase;
struct IntuitionBase *IntuitionBase;

WORD  *t_rast;      /* Temporary raster from AllocMem   */
WORD   *area_buf;   /* Area used for AreaInfo   */

/****    Variables Initialized by init_generic()  **********/
struct Screen *Screen;
struct Window *Window;
struct RastPort *pRp;   /* Graphics rendering area */
struct MsgPort  *pUp;   /* Input message port   */

struct ViewPort *pVp;

struct timer {          /* Timer struct used to record times   */
   int seconds,micros;
   };

/******************************************************
* Alright already, here's the stuff                   */
main()
{
struct IntuiMessage *imsg;

init_generic();

printf("Doing point test in a full-screen window\n");
point_test(pRp);     /* First try with the 'window' rastport   */

printf("Now doing point test directly to screen rastport\n");
point_test(&Screen->RastPort); /* Next try the 'screen' rastport   */


clear_test();     /* Does block clear of 32Kbytes  */

printf("Block test in a full-screen window\n");
block_test(pRp);

printf("Block test directly to screen rastport\n");
block_test(&Screen->RastPort);

printf("Line test in window\n");
line_test(pRp);

window_test();

SetWindowTitles(Window,"Click Close to Quit...",NL);

SetDrMd(pRp,COMPLEMENT);
FOREVER {
   Draw(pRp,2+rand()%(WDTH-4),9+rand()%(HGHT-11));
   if ( imsg = (struct IntuiMessage *)GetMsg(pUp) ) {
      if ( imsg->Class == CLOSEWINDOW ) {

         close_generic();
         }
      SetRGB4(pVp,rand()&3,rand(),rand(),rand());
      ReplyMsg(imsg);
      }
   }
}


/* Used to open the test Window   */
struct NewWindow TstWindow = {
   0,0,WDTH,HGHT, /* LeftEdge,TopEdge,Width,Height */
   8,9,           /* DetailPen,BlockPen     */
   NL,
   SMART_REFRESH | ACTIVATE, /*Flags*/
   NL,NL,"Test",   /* FirstGadget, CheckMark, Title  */
   NL,         /* MUST SET SCREEN AFTER OPENSCREEN!!! */
   NL,         /* BitMap    */
   100,60,32767,32767, /* MinW, MinH, MaxW, MaxH */
   CUSTOMSCREEN };     /* Type   */



/***************************************************************
* window_test()
*     This test opens up a window 100 times. The window is
* a full-screen WDTH*HGHT * 4 BitPlane display.
*     Next, it does the same test but instead just does window-
* to-front, window-to-back 100 times.
***************************************************************/
window_test()
{
register int i;
register struct Window *tst_window;
struct timer start,finish ,t2start,t2finish;

TstWindow.Screen = Screen;

CurrentTime(&start.seconds,&start.micros);
for ( i=0 ; i<100 ; i++) {
   if ( ! (tst_window=(struct Window *)OpenWindow(&TstWindow)) ) {

      printf("Error Openeing Window, aborted!\n");
      close_generic();
      }
   CloseWindow(tst_window);
   }
CurrentTime(&finish.seconds,&finish.micros);
printf("Window Benchmark #1 took "); delta(&finish,&start);
printf("\n");

/* Now do second test... Just WindowToFront, WindowToBack   */
tst_window = (struct Window *)OpenWindow(&TstWindow);

CurrentTime(&t2start.seconds,&t2start.micros);
for ( i=0; i<100 ; i++) {
   WindowToBack(tst_window);
   WindowToFront(tst_window);
   }
CloseWindow(tst_window);
CurrentTime(&t2finish.seconds,&t2finish.micros);


printf("Window Benchmark #2 took "); delta(&t2finish,&t2start);
printf("\n");
}




/**************************************************************
* point_test(rastport)
*     This routine tests the Read/Write/Move pixel functions
***************************************************************/
point_test(rastport)
struct RastPort *rastport;
{
register struct RastPort *fRp;
register int x,y;

struct timer write,read,move,control, start;

   fRp = rastport;


   CurrentTime(&start.seconds,&start.micros);
   for (y=0;y<HGHT;y++) {
      SetAPen(fRp,y&15);
      for (x=0;x<WDTH; x++)
         WritePixel(fRp,x,y);
      }
   CurrentTime(&write.seconds,&write.micros);

   for (y=0;y<HGHT;y++) {
      for (x=0;x<WDTH; x++)
         ReadPixel(fRp,x,y);
      }
   CurrentTime(&read.seconds,&read.micros);

   for (y=0;y<HGHT;y++) {
      for (x=0;x<WDTH; x++)
         Move(pRp,x,y);
      }
   CurrentTime(&move.seconds,&move.micros);


   for (y=0;y<HGHT;y++)
      for (x=0;x<WDTH; x++)
         ;
   CurrentTime(&control.seconds,&control.micros);
   printf("Write pixel 640*200 took "); delta(&write,&start);
   printf("   Readpixel   640*200 took "); delta(&read,&write);
   printf("\nMove        640*200 took "); delta(&move,&read);
   printf("    Control     640*200 took "); delta(&control,&move);
   printf("\n");
}


/***********************************************************
*  block_test
*     Does block copy, set block to value, block complement.
* These are all done by "applications" functions supplied in the
* Amiga graphics library.  It may be possible to speed these up a
* bit if you are not rendering in a rastport, using direct blitter
* manipulations.

*****************************************************************/

block_test(rastport)
struct RastPort *rastport;
{
struct timer start1,finish1,finish2,finish3;

register int i;

#define COPY 0xc0    /* These are blitter minterms */
#define XOR  0x50

   CurrentTime(&start1.seconds,&start1.micros);

   for ( i=0; i<500; i++ )
      SetRast(rastport,i&3);

   CurrentTime(&finish1.seconds,&finish1.micros);

   for ( i=0; i<500; i++)

      ClipBlit(rastport,0,0,rastport,0,0,WDTH,HGHT,COPY);

   CurrentTime(&finish2.seconds,&finish2.micros);

   for ( i=0; i<500; i++)
      ClipBlit(rastport,0,0,rastport,0,0,WDTH,HGHT,COPY);

   CurrentTime(&finish3.seconds,&finish3.micros);

   printf("Set 32K "); delta(&finish1,&start1);
   printf(" Copy "); delta(&finish2,&finish1);
   printf(" Complement "); delta(&finish3,&finish2);
   printf("\n");
}

/*****************************************************************
* clear_test()
*     This test clears a block of memory two ways:
* First, using an applications blitter-clear call;
* Then using a somewhat optimized long-word clear using the

* 68000.  
*****************************************************************/
clear_test()
{
struct timer start1,finish1,finish2;

register WORD *mem_ptr;
register LONG *mp2;

register int i,j;

   if ( ! (mem_ptr = (WORD *) AllocMem(32000,VID)) ) {
      printf("Can't get mem for clear test\n");
      close_generic();
      }

   CurrentTime(&start1.seconds,&start1.micros);

   /* Use the blitter alone to clear mem  */
   for ( i = 0; i < 499 ; i++)

      BltClear(mem_ptr,32000,0); /* flags are 'Don't Wait'  */
   WaitBlit();    /* Wait for the above to finish...  */

   CurrentTime(&finish1.seconds,&finish1.micros);

   /* Use the CPU alone to clear mem   */
   for ( i=0; i<500 ; i++) {
      mp2 = (LONG *)mem_ptr;
      for ( j = 0; j < 2000 ; j++) {
         *mp2++ = NL;
         *mp2++ = NL;
         *mp2++ = NL;
         *mp2++ = NL;
         }
      }

   CurrentTime(&finish2.seconds,&finish2.micros);

   printf("Block clear blitter 1:"); delta(&finish1,&start1);
   printf(" CPU "); delta(&finish2,&finish1);

   printf("\n");
   FreeMem(mem_ptr,32000);
}

line_test(rastport)
struct RastPort *rastport;
{

   line_sub(rastport,0);
   line_sub(rastport,1);
   line_sub(rastport,20);
   line_sub(rastport,100);
}

/************************************************************
* line_sub(offset)
*     This routine was borrowed from bwebster, originating from
* a guy in Switzerland named Fons Rademakers of CERNS, who
* designed the test for comparing Mac and Apollos           */
line_sub(rastport,offset)

int offset;
struct RastPort *rastport;
{
register int x1,y1,x2,y2,i,j;
register struct RastPort *fastrp;

struct timer start,finish;

fastrp = rastport;

CurrentTime(&start.seconds,&start.micros);

x1 = 150 - offset;
y1 = 0;
x2 = 150+offset;
y2 = 199;

for ( i=0; i<20; i++) {
   SetAPen(fastrp,i&3);
   for ( j=0; j < 200; j++) {

      Move(fastrp,x1++,y1);
      Draw(fastrp,x2++,y2);
      }
   SetAPen(fastrp,(~i)&3);
   for ( j=0; j<200; j++) {
      Move(fastrp,x1--,y1);
      Draw(fastrp,x2--,y2);
      }
   }
CurrentTime(&finish.seconds,&finish.micros);
printf("Offset %d result:",offset); delta(&finish,&start);
printf("\n");
}


/***************************************************
* delta()
*     This little cluge calculates the delta-time
* and prints it, along with a '\n'                */
delta(finish,start)

struct timer *finish,*start;
{
int dsec,dmic;

   dmic = finish->micros - start->micros;
   dsec = finish->seconds - start->seconds;

   if (dmic < 0) {
      dmic += 1000000;
      dsec--;
      }
   printf("%d.%d",dsec,dmic/100000);
}





/************************************************************************/
/* Many of the Amiga 'C' language structures are most efficiently set   */

/* up by initializing the values with the declarations         */
/************************************************************************/

/* Font Descriptor   */
struct TextAttr MyFont = {"topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT };

/* Used to open a Screen   */
struct NewScreen NewScreen = {
   0,0,WDTH,HGHT,DPTH,  /* Left,Top,Width,Height,Depth    */
   7,8,VIEW_MODE,            /* DetailPen, BlockPen, ViewModes */
   CUSTOMSCREEN,&MyFont,/* Type, Font  */
   (UBYTE *)"Expletive Deletion",  /* Title       */
   NL,NL };       /* Gadgets, BitMap  */

/* Used to open a Window   */
struct NewWindow NewWindow = {
   0,0,WDTH,HGHT, /* LeftEdge,TopEdge,Width,Height */
   8,9,           /* DetailPen,BlockPen     */
   CLOSEWINDOW | MOUSEMOVE,
   WINDOWCLOSE | WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | SMART_REFRESH |

      ACTIVATE | REPORTMOUSE, /*Flags*/
   NL,NL,"Main Window",   /* FirstGadget, CheckMark, Title  */
   NL,         /* MUST SET SCREEN AFTER OPENSCREEN!!! */
   NL,         /* BitMap    */
   100,60,32767,32767, /* MinW, MinH, MaxW, MaxH */
   CUSTOMSCREEN };     /* Type   */


/* These items are used as temp buffers for fill etc  */
#define AREA_ENTS 400   /* Number of areabuffer entries, area_buf */
#define AREASIZ AREA_ENTS*5   /* Space for AreaInfo   */

struct AreaInfo   area_info;
struct TmpRas     tmp_ras;



/****************************************************************
* init_generic()
*

*     Opens Screen and full-sized Window for ****** program.
*     Sets up TmpRas for draw and flood-fill
*     Allocates and initializes various buffer areas...
*
*     Initializes handles Screen,Window,pRp
*
*  See also: close_generic()                                     */
init_generic()
{
   if ( ! (IntuitionBase = (struct IntuitionBase *)
     OpenLibrary("intuition.library",INTUITION_REV)) ||
      ! (GfxBase = (struct GfxBase *)
     OpenLibrary("graphics.library",GRAPHICS_REV))  ||
      ! (Screen =(struct Screen *)OpenScreen(&NewScreen)) ) {
     printf("Wow this shouldn't happen!!!\n");
          exit(101);
          }

   NewWindow.Screen = Screen;   /* NEED TO SET SCREEN!!! */


   if ( ! (Window =(struct Window *)OpenWindow(&NewWindow))) {
      printf("Sombody stole your memory!");
      exit(102);
      }

   pRp = Window->RPort;    /* Get E-Z acess to handles  */
   pUp = Window->UserPort;
   pVp = (struct ViewPort *)ViewPortAddress(Window);

   /* Allocate major buffer entries */
   t_rast   = (WORD *) AllocMem(RASSIZE(WDTH,HGHT), VID);
   area_buf = (WORD *) AllocMem(2000,VID);

   if ( ! t_rast || ! area_buf ) {
      printf("Out of memory\n");
      exit(103);
      }

   /* Initialize rastport temp buffers */
   InitArea(&area_info,area_buf,AREA_ENTS);

   pRp->AreaInfo = &area_info;

   tmp_ras.RasPtr = (BYTE *)t_rast;
   tmp_ras.Size = RASSIZE(WDTH,HGHT);
   pRp->TmpRas = &tmp_ras;
}


/*************************************************
* close_generic
*
*  Returns all resources gotten by init_generic
*************************************************/
close_generic()
{
   FreeMem(t_rast,RASSIZE(WDTH,HGHT));
   FreeMem(area_buf,2000);

   CloseWindow(Window);
   CloseScreen(Screen);

   exit();
}

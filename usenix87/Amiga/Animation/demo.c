


Here is a sample program that demonstrates the animation routines
(for Bobs and VSprites) on the Amiga.  It also uses double buffering
(drawing into one area while displaying another) so as to smooth
the display motion.

Amiga/Lattice C 3.03 linking information:

	FROM lib:Astartup.obj, dbuff.gels.o, geltools.o
	TO dbuff.gels
	LIBRARY lib:amiga.lib, lib:lc.lib

/* dbuff.gels.c */


/* SAMPLE PROGRAM THAT USES GELTOOLS TO PRODUCE A DOUBLE BUFFERED DISPLAY
 * SCREEN CONTAINING TWO BOBS AND TWO VSPRITES
 *
 * Author:  David Lucas
 */


/* Leave this structure definition at the top. Look at gels.h. */
struct vInfo {
	short vx,vy;		/* This VSprites velocity. */
	short id;
};
#define VUserStuff struct vInfo


/* Things to notice:
 
        Default value in sprite/playfield priority register has all
        hardware sprites having a higher priority than either of the
        two playfields.   Areas containing color 0 of both the bob and
        vsprite are shown as transparent (see hole in center of each).
 
        You can specify bob drawing order by using the before and after
        pointers, thereby always maintaining an apparent precedence of
        one bob over another.  Re Vsprites.... because they are assigned
        sequentially from top of screen to bottom, in sprite numerical
        order (0, 1, 2, 3 etc), and because the lowest numbered hardware
        sprite has the highest video precedence, the sprite that is
        closest to the top of the screen always appears in front of the
        sprite beneath it.
 
        Without double-buffering, there would be flicker on the part
	of the bobs.  Double buffering consists of writing into an area 
	that is not being displayed.  Some of the flicker could have been
	alleviated by waiting for the video beam to reach top-of-frame 
	before doing the drawing, but when the bobs are near the top, 
	it makes it all the more difficult to draw without apparent 
	flicker in that case.  Also note that multitasking will 
	occasionally upset even this plan in that it can delay the 
	drawing operation until the beam is in the area that is being drawn.
 
*/

/*******************************************************************************
 * A sprite and a bob on a screen.
 */

#include "intuall.h"

#define SBMWIDTH 320	/* My screen size constants. */
#define SBMHEIGHT 200
#define SBMDEPTH 4

#define RBMWIDTH 330	/* My rastport size constants. */
#define RBMHEIGHT 210
#define RBMDEPTH SBMDEPTH

#define VSPRITEWIDTH 1	/* My VSprite constants. */
#define VSPRITEHEIGHT 12
#define VSPRITEDEPTH 2
#define NSPRITES 2

#define BOBWIDTH 62	/* My Bob constants. */
#define BOBHEIGHT 31
#define BOBDEPTH 4
#define NBOBS 2

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

struct IntuiMessage *MyIntuiMessage = NULL;

struct TextAttr TestFont = {	/* Needed for opening screen. */
	(STRPTR)"topaz.font", TOPAZ_EIGHTY, 0, 0
};

/* DBL BUF */
struct BitMap *MyBitMapPtrs[2] = {NULL, NULL};
WORD ToggleFrame = 0;

struct GelsInfo GInfo;		/* For all Gels. */

struct VSprite *VSprites[NSPRITES];
WORDBITS VSpriteImage[] = {
/* Plane 0, Plane 1 */
	0xFFFF, 0xFFFF,	/* Line 1, first. */
	0xFFFF, 0xC003,
	0xFFFF, 0xC003,
	0xF00F, 0xCFF3,
	0xF00F, 0xCFF3,
	0xF00F, 0xCC33,
	0xF00F, 0xCC33,
	0xF00F, 0xCFF3,
	0xF00F, 0xCFF3,
	0xFFFF, 0xC003,
	0xFFFF, 0xC003,
	0xFFFF, 0xFFFF,	/* Line 12, last. */
};
USHORT *VSpriteImage_chip = 0;

/* These are the colors that will be used for my VSprites. Note I really do mean
 * colors, not color register numbers. High to low, starting at bit 12 and going
 * down to LSB, there are four bits each of red, green and blue. Please read the
 * sprite section of the hardware manual. The gels system will put them into the
 * proper color registers when they are displayed. Reminder: Sprites can only
 * use color registers in sets of 3...
 *   17,18,19 = sprite 0 and 1,
 *   21,22,23 = sprite 2 and 3,
 *   25,26,27 = sprite 4 and 5,
 *   29,30,31 = sprite 6 and 7.
 * Please read the section on how VSprites are assigned in the RKM.
 */
WORD MyVSpriteColors[] = {
	0x0f00,	/* Full red. */
	0x00f0,	/* Full green. */
	0x000f	/* Full blue. */
};

struct Bob *Bobs[NBOBS];
short BobImage[] = {
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,	/* Plane 0, line 1. */
	0xC000, 0x0000,	0x0000, 0x000C,
	0xCFFF, 0xFFFF,	0xFFFF, 0xFFCC,
	0xCC00, 0x0000,	0x0000, 0x00CC,
	0xCCFF, 0xFFFF,	0xFFFF, 0xFCCC,
	0xCCC0, 0x0000,	0x0000, 0x0CCC,
	0xCCCF, 0xFFFF,	0xFFFF, 0xCCCC,
	0xCCCC, 0x0000,	0x0000, 0xCCCC,
	0xCCCC, 0xFFFF, 0xFFFC, 0xCCCC,
	0xCCCC, 0xC000,	0x000C, 0xCCCC,
	0xCCCC, 0xCFFF,	0xFFCC, 0xCCCC,
	0xCCCC, 0xCC00,	0x00CC, 0xCCCC,
	0xCCCC, 0xCCFF,	0xFCCC, 0xCCCC,
	0xCCCC, 0xCCC0,	0x0CCC, 0xCCCC,
	0xCCCC, 0xCCCF,	0xCCCC, 0xCCCC,
	0xCCCC, 0xCCCC,	0xCCCC, 0xCCCC,
	0xCCCC, 0xCCCF,	0xCCCC, 0xCCCC,
	0xCCCC, 0xCCC0,	0x0CCC, 0xCCCC,
	0xCCCC, 0xCCFF,	0xFCCC, 0xCCCC,
	0xCCCC, 0xCC00,	0x00CC, 0xCCCC,
	0xCCCC, 0xCFFF,	0xFFCC, 0xCCCC,
	0xCCCC, 0xC000,	0x000C, 0xCCCC,
	0xCCCC, 0xFFFF, 0xFFFC, 0xCCCC,
	0xCCCC, 0x0000,	0x0000, 0xCCCC,
	0xCCCF, 0xFFFF,	0xFFFF, 0xCCCC,
	0xCCC0, 0x0000,	0x0000, 0x0CCC,
	0xCCFF, 0xFFFF,	0xFFFF, 0xFCCC,
	0xCC00, 0x0000,	0x0000, 0x00CC,
	0xCFFF, 0xFFFF,	0xFFFF, 0xFFCC,
	0xC000, 0x0000,	0x0000, 0x000C,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,	/* Plane 0, line 31. */

        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,	/* Plane 1, line 1. */
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
        0xF000, 0x0000, 0x0000, 0x003C,
        0xF000, 0x0000, 0x0000, 0x003C,
        0xF0FF, 0xFFFF, 0xFFFF, 0xFC3C,
        0xF0FF, 0xFFFF, 0xFFFF, 0xFC3C,
        0xF0F0, 0x0000, 0x0000, 0x3C3C,
        0xF0F0, 0x0000, 0x0000, 0x3C3C,
        0xF0F0, 0xFFFF, 0xFFFC, 0x3C3C,
        0xF0F0, 0xFFFF, 0xFFFC, 0x3C3C,
        0xF0F0, 0xF000, 0x003C, 0x3C3C,
        0xF0F0, 0xF000, 0x003C, 0x3C3C,
        0xF0F0, 0xF0FF, 0xFC3C, 0x3C3C,
        0xF0F0, 0xF0FF, 0xFC3C, 0x3C3C,
        0xF0F0, 0xF0F0, 0x3C3C, 0x3C3C,
        0xF0F0, 0xF0F0, 0x3C3C, 0x3C3C,
        0xF0F0, 0xF0F0, 0x3C3C, 0x3C3C,
        0xF0F0, 0xF0FF, 0xFC3C, 0x3C3C,
        0xF0F0, 0xF0FF, 0xFC3C, 0x3C3C,
        0xF0F0, 0xF000, 0x003C, 0x3C3C,
        0xF0F0, 0xF000, 0x003C, 0x3C3C,
        0xF0F0, 0xFFFF, 0xFFFC, 0x3C3C,
        0xF0F0, 0xFFFF, 0xFFFC, 0x3C3C,
        0xF0F0, 0x0000, 0x0000, 0x3C3C,
        0xF0F0, 0x0000, 0x0000, 0x3C3C,
        0xF0FF, 0xFFFF, 0xFFFF, 0xFC3C,
        0xF0FF, 0xFFFF, 0xFFFF, 0xFC3C,
        0xF000, 0x0000, 0x0000, 0x003C,
        0xF000, 0x0000, 0x0000, 0x003C,
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,	/* Plane 1, line 31. */

	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC, /* Plane 2, line 1. */
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFF00, 0x03FC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0xFFFF, 0xFFFC, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFF00, 0x0000, 0x0000, 0x03FC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC,	/* Plane 2, line 31. */

	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC, /* Plane 3, line 1. */
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0x0000,	0x0003, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFC	/* Plane 3, line 31. */
};
USHORT *BobImage_chip = 0;

/* These are for my custom screen. */
struct Screen *screen = NULL;
struct NewScreen ns = {
   0, 0,		/* Start position. */
   SBMWIDTH, SBMHEIGHT, SBMDEPTH,	/* Width, height, depth. */
   0, 0,		/* Default detail pen, block pen. */
   NULL,		/* Viewing mode. */
   CUSTOMSCREEN | CUSTOMBITMAP,		/* Screen type. DBL BUF */
   &TestFont,		/* Font to use. */
   NULL,		/* No default title. */
   NULL,		/* No pointer to additional gadgets. */
   NULL			/* No pointer to CustomBitMap. */
};

/* These are for my window. */
struct Window *window = NULL;
struct NewWindow nw = {
   0, 0,		/* Start position. */
   SBMWIDTH, SBMHEIGHT,	/* Width, height. */
   0, 0,		/* Detail pen, block pen. */
   CLOSEWINDOW,		/* IDCMP flags. */
   WINDOWCLOSE | BORDERLESS,		/* Flags. */
   NULL,		/* No pointer to FirstGadget. */
   NULL,		/* No pointer to first CheckMark. */
   NULL,		/* No default Title. */
   NULL,		/* No pointer to Screen. */
   NULL,		/* No pointer to BitMap. */
   0, 0,		/* MinWidth, MinHeight (not used). */
   SBMWIDTH, SBMHEIGHT,	/* MaxWidth, MaxHeight (not used). */
   CUSTOMSCREEN		/* Screen type. */
};

/*******************************************************************************
 * This will be called if a sprite collision with the border is detected.
 */

borderPatrol(s, b)
struct VSprite *s;
int b;
{
   register struct vInfo *info;

   info = &s->VUserExt;
   if (b & (TOPHIT | BOTTOMHIT))	/* Top/Bottom hit, change direction. */
      info->vy = -(info->vy);
   if (b & (LEFTHIT | RIGHTHIT))	/* Left/Right hit, change direction. */
      info->vx = -(info->vx);

}

/*******************************************************************************
 * Fun Starts.
 */

main()
{
   SHORT i, j;

   /* Open libraries that will be used directly. */
   if ((IntuitionBase = (struct IntuitionBase *)
         OpenLibrary("intuition.library", LIBRARY_VERSION)) == 0) {
#ifdef DEBUG
      kprintf("Main: Can't open Intuition.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

   if ((GfxBase = (struct GfxBase *)
         OpenLibrary("graphics.library", LIBRARY_VERSION)) == 0) {
#ifdef DEBUG
      kprintf("Main: Can't open Graphics.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

/*******************************************************************************
 * DBL BUF
 */
   for(j=0; j<2; j++) {
      if ((MyBitMapPtrs[j] = (struct BitMap *)
            AllocMem(sizeof(struct BitMap), MEMF_CHIP)) == 0) {
#ifdef DEBUG
         kprintf("Main: Can't allocate BitMap.\n");
#endif
         MyCleanup();
         Exit(-1);
      }
      InitBitMap(MyBitMapPtrs[j], RBMDEPTH, RBMWIDTH, RBMHEIGHT);
      for(i=0; i<RBMDEPTH; i++) {
         if ((MyBitMapPtrs[j]->Planes[i] = (PLANEPTR)AllocRaster(RBMWIDTH,
               RBMHEIGHT)) == 0) {
#ifdef DEBUG
            kprintf("Main: Can't allocate BitMaps' Planes.\n");
#endif
            MyCleanup();
            Exit(-1);
         }
         BltClear(MyBitMapPtrs[j]->Planes[i], (RBMWIDTH / 8) * RBMHEIGHT, 1);
      }
   }
   ns.CustomBitMap = MyBitMapPtrs[0]; /* !! */
   screen->RastPort.Flags = DBUFFER;

   /* Open My Very Own Screen. */
   if ((screen = (struct Screen *)OpenScreen(&ns)) == 0) {
#ifdef DEBUG
      kprintf("Main: Can't open Screen.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

   /* Now get that flashing title bar off the display. DBL BUF */
/*
   screen->ViewPort.RasInfo->RxOffset = 5;
   screen->ViewPort.RasInfo->RyOffset = 5;
*/

   /* Set screens' colors (Could've used LoadRGB4()). */
   SetRGB4(&screen->ViewPort, 00, 00, 00, 00);
   SetRGB4(&screen->ViewPort, 01, 15, 00, 00);
   SetRGB4(&screen->ViewPort, 02, 00, 15, 00);
   SetRGB4(&screen->ViewPort, 03, 00, 00, 15);
   SetRGB4(&screen->ViewPort, 04, 15, 15, 00);
   SetRGB4(&screen->ViewPort, 05, 15, 00, 15);
   SetRGB4(&screen->ViewPort, 06, 08, 15, 15);
   SetRGB4(&screen->ViewPort, 07, 15, 11, 00);
   SetRGB4(&screen->ViewPort, 08, 05, 13, 00);
   SetRGB4(&screen->ViewPort, 09, 14, 03, 00);
   SetRGB4(&screen->ViewPort, 10, 15, 02, 14);
   SetRGB4(&screen->ViewPort, 11, 15, 13, 11);
   SetRGB4(&screen->ViewPort, 12, 12, 09, 08);
   SetRGB4(&screen->ViewPort, 13, 11, 11, 11);
   SetRGB4(&screen->ViewPort, 14, 07, 13, 15);
   SetRGB4(&screen->ViewPort, 15, 15, 15, 15);

   nw.Screen = screen;

   if ((window = (struct Window *)OpenWindow(&nw)) == 0) {
#ifdef DEBUG
      kprintf("Main: Can't open Window.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

  /*****************************************************************************
   * Now that the screen envirionment is set up, It's time to set up the
   * gels system.
   */

   /* ReadyGels is in GelTools(). */
   if (ReadyGels(&GInfo, &screen->RastPort) != 0) {
#ifdef DEBUG
      kprintf("Main: ReadyGels failed.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

   SetCollision(0, borderPatrol, &GInfo);

   /* Copy Images to chip memory. */
   if (!InitImages()) {
#ifdef DEBUG
      kprintf("Main: InitImages() failed.\n");
#endif
      MyCleanup();
      Exit(-1);
   }

  /*****************************************************************************
   * System is set up, now set up each Gel.
   */

   /* First use the routines in geltools to get the sprite. */
   for(i = 0; i < NSPRITES; i++) {
      if ((VSprites[i] = (struct VSprite *)MakeVSprite(VSPRITEHEIGHT,
            VSpriteImage_chip, &MyVSpriteColors[0], i*6, (i*8)+10,
            VSPRITEWIDTH,  VSPRITEDEPTH, VSPRITE)) == 0) {
#ifdef DEBUG
         kprintf("Main: MakeVSprite failed.\n");
#endif
         MyCleanup();
         Exit(-1);
      }

      VSprites[i]->VUserExt.vx = 1;
      VSprites[i]->VUserExt.vy = 1;
      VSprites[i]->VUserExt.id = i;

      AddVSprite(VSprites[i], &screen->RastPort);
   }

   /* First use the routines in geltools to get the bob. */
   for(i = 0; i < NBOBS; i++) {
      if ((Bobs[i] = (struct Bob *)MakeBob(BOBWIDTH, BOBHEIGHT, BOBDEPTH,
            BobImage_chip, 0x0F, 0x00, (i*6), (i*8)+10,
            SAVEBACK | OVERLAY)) == 0) {
#ifdef DEBUG
         kprintf("Main: MakeBob failed.\n");
#endif
         MyCleanup();
         Exit(-1);
      }

      Bobs[i]->BobVSprite->VUserExt.vx = 1;
      Bobs[i]->BobVSprite->VUserExt.vy = 1;
      Bobs[i]->BobVSprite->VUserExt.id = i;

      /* DBL BUF */
      if ((Bobs[i]->DBuffer = (struct DBufPacket *)AllocMem (sizeof(struct
            DBufPacket), MEMF_CHIP)) == 0) {
#ifdef DEBUG
         kprintf("Main: Can't allocate double buffers' packet for a bob.\n");
#endif
         MyCleanup();
         Exit(-1);
      }
      if ((Bobs[i]->DBuffer->BufBuffer = (WORD *)AllocMem (sizeof(SHORT) *
            ((BOBWIDTH+15)/16) * BOBHEIGHT * BOBDEPTH, MEMF_CHIP)) == 0) {
#ifdef DEBUG
         kprintf("Main: Can't allocate double buffer for a bob.\n");
#endif
         MyCleanup();
         Exit(-1);
      }
      AddBob(Bobs[i], &screen->RastPort);

      /* The following relies on the fact that AddBob sets the before
       * and after pointers to 0, so the first before and last after.
       * are left alone.
       * Earlier bob has higher priority, thus this bob'll be drawn
       * AFTER that one, thus this bob will appear on top of all earlier
       * ones. One could set the bobs to be drawn in any order by rearranging
       * these pointers.
       */
      if (i > 0) {
         Bobs[i]->After = Bobs[i-1];
         Bobs[i]->After->Before = Bobs[i];
      }
   }  /* End of for. */

  /*****************************************************************************
   * Hey, wow, everything opened, and allocated, and initialized! Whew.
   */
   for (;;) {
      DrawGels();

      while (MyIntuiMessage = (struct IntuiMessage *)
            GetMsg(window->UserPort))
         switch (MyIntuiMessage->Class) {
            case CLOSEWINDOW:
               ReplyMsg(MyIntuiMessage);
               MyCleanup();
               Exit(TRUE);
               break;
            default:
               ReplyMsg(MyIntuiMessage);
               break;
         }
   }
} 

/*******************************************************************************
 * DrawGels part of loop.
 */

DrawGels()
{
   register struct VSprite *pSprite;

   /* Move everything in the sprite list. This includes Bobs. */
   pSprite = GInfo.gelHead->NextVSprite;
   while (pSprite != GInfo.gelTail){
      pSprite->X += pSprite->VUserExt.vx;
      pSprite->Y += pSprite->VUserExt.vy;
      pSprite = pSprite->NextVSprite;
   }

   SortGList(&screen->RastPort);        /* Put the list in order. */
   DoCollision(&screen->RastPort);	/* Collision routines may called now. */
   DrawGList(&screen->RastPort,&screen->ViewPort);      /* Draw 'em. */
   screen->ViewPort.RasInfo->BitMap = MyBitMapPtrs[ToggleFrame];   /* DBL BUF */
   WaitTOF();                           /* When the beam hits the top... */
   MakeScreen(screen);                  /* Tell intuition to do it's stuff. */
   RethinkDisplay();                    /* Does a MrgCop & LoadView. */
   ToggleFrame ^=1;						   /* DBL BUF */
   screen->RastPort.BitMap = MyBitMapPtrs[ToggleFrame];		   /* DBL BUF */
}

/*******************************************************************************
 * This will be called in case of error, or when main is done.
 */

MyCleanup()
{
   short i, j;

   for (i=0; i < NBOBS; i++) {
      if (Bobs[i] != NULL) {
        DeleteGel(Bobs[i]->BobVSprite);
      }
   }

   for (i=0; i < NSPRITES; i++) {
      if (VSprites[i] != NULL) {
        DeleteGel(VSprites[i]);
      }
   }
   PurgeGels(&GInfo);
   FreeImages();
   if (window != NULL)
      CloseWindow(window);
   if (screen != NULL)
      CloseScreen(screen);

   /* DBL BUF */
   for(j=0; j<2; j++) {
      if (MyBitMapPtrs[j] != NULL) {
         for(i=0; i<RBMDEPTH; i++) {
         if (MyBitMapPtrs[j]->Planes[i] != 0)
            FreeRaster(MyBitMapPtrs[j]->Planes[i], RBMWIDTH, RBMHEIGHT);
        }
         FreeMem(MyBitMapPtrs[j], sizeof(struct BitMap));
     }
   }

   if (GfxBase != NULL)
      CloseLibrary(GfxBase);
   if (IntuitionBase != NULL)
      CloseLibrary(IntuitionBase);
}

InitImages()
{
   extern USHORT *VSpriteImage_chip;
   extern USHORT *BobImage_chip;
   int i;

   if ((VSpriteImage_chip = (USHORT *)
         AllocMem(sizeof(VSpriteImage), MEMF_CHIP)) == 0) {
#ifdef DEBUG
      kprintf("InitImages: No Memory for VSpriteImage.\n");
#endif
      return(FALSE);
   }
   if ((BobImage_chip = (USHORT *)
         AllocMem(sizeof(BobImage), MEMF_CHIP)) == 0) {
#ifdef DEBUG
      kprintf("InitImages: No Memory for BobImage.\n");
#endif
      return(FALSE);
   }
   for(i=0; i<24; i++)
      VSpriteImage_chip[i] = VSpriteImage[i];
   for(i=0; i<496; i++)
      BobImage_chip[i] = BobImage[i];
   return(TRUE);
}

FreeImages()
{
   extern USHORT *VSpriteImage_chip;
   extern USHORT *BobImage_chip;

   if (VSpriteImage_chip != 0)
         FreeMem(VSpriteImage_chip, sizeof(VSpriteImage));
   if (BobImage_chip != 0)
         FreeMem(BobImage_chip, sizeof(BobImage));
}



/* geltools.c */

/* =========================================================================

GELTOOLS.C - 

	A FILE CONTAINING USEFUL SETUP TOOLS FOR THE ANIMATION SYSTEM


	author:  Rob Peck, incorporating valuable comments and changes from
		 Barry Whitebook and David Lucas.

=========================================================================== */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>	/* ALWAYS INCLUDE GFX.H before other includes */
#include <graphics/gels.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>


/*******************************************************************************
 * This file is a collection of tools which are used with the vsprite and
 * bob software.  It contains the following:
 *
 * ReadyGels( *gelsinfo, *rastport );
 * PurgeGels( *gelsinfo );
 *
 * struct VSprite *MakeVSprite(lineheight,*image,*colorset,x,y,
 * 	wordwidth,imagedepth,flags);       
 * DeleteVSprite( &VSprite );
 *
 * struct Bob *MakeBob(bitwidth,lineheight,imagedepth,*image,
 * 		planePick,planeOnOff,x,y)
 * DeleteBob( &Bob );
 *
 * ReadyGels sets up the defaults of the gel system by initializing the
 * GelsInfo structure you provide.  First it allocates room for and
 * links in lastcolor and nextline. It then uses information in your
 * RastPort structure to establish boundary collision defaults at
 * the outer edges of the raster.  It then links together the GelsInfo
 * and the RastPort which you provide. Next it allocates space for two
 * dummy virtual sprite structures, calls InitGels and SetCollision.
 ! You must already have run LoadView before ReadyGels is called.
 *
 * PurgeGels deallocates all memory which ReadyGels and NewGelList have
 * allocated.  The system will crash if you have not used these
 * routines to allocate the space (you cant deallocate something
 * which you havent allocated in the first place).
 *
 * MakeVSprite allocates enough space for and inits a normal vsprite.
 * DeleteVSprite deallocates the memory it used.
 *
 * MakeBob initializes a standard bob and allocates as much memory as is needed
 * for a normal bob and its vsprite structure, links them together.
 * To find the associated vsprite, look at the back-pointer (see the
 * routine doc itself).
 * DeleteBob deallocates the memory it used.
 *
 * Written by Rob Peck, with thanks to Barry Whitebrook and David Lucas.
 *
 ******************************************************************************/

void border_dummy()
{ 
   return; 
}

/* Caller passes a pointer to his GelsInfo structure which he wants to init, 
 * along with a pointer to his IVPArgs.  Default init places the topmost
 * bottommost etc at the outermost boundaries of callers rastport parameters.
 * Caller can change all this stuff after this routine returns.
 */

extern struct RastPort *myRast;

struct VSprite *SpriteHead = NULL;
struct VSprite *SpriteTail = NULL;

/*******************************************************************************
 * This routine cannot be run until the first LoadView(&view) has been
 * executed.  InitGels works with an already active View, so LoadView
 * must have been run first.
 */

ReadyGels(g, r)
struct RastPort *r;
struct GelsInfo *g;
{
   /* Allocate head and tail of list. */
   if ((SpriteHead = (struct VSprite *)AllocMem(sizeof
         (struct VSprite), MEMF_PUBLIC | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("ReadyGels: No memory for sprite head.\n");
#endif
      return(-1);
   }

   if ((SpriteTail = (struct VSprite *)AllocMem(sizeof
         (struct VSprite), MEMF_PUBLIC | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("ReadyGels: No memory for sprite tail.\n");
#endif
      return(-1);
   }

  /* By setting all bits here, it means that there are NO
   * reserved sprites.  The system can freely use all of the 
   * hardware sprites for its own purposes.  The caller will not be
   * trying to independently use any hardware sprites!
   */
   g->sprRsrvd = -1;

  /* The nextline array is used to hold system information about
   * "at which line number on the screen is this hardware sprite
   * again going to become available to be given a new vsprite to
   * display".
   */
   if ((g->nextLine = (WORD *)AllocMem(sizeof(WORD) * 8,
         MEMF_PUBLIC | MEMF_CLEAR)) == NULL) {
#ifdef DEBUG
      kprintf("ReadyGels: No memory for nextline.\n");
#endif
      return(-1);
   }

  /* In the lastcolor pointer array, the system will store
   * a pointer to the color definitions most recently used
   * by the system. .... as a reminder, virtual sprites can
   * be assigned to any of the real hardware sprites which
   * may be available at the time.  The vsprite colors will
   * be written into the hardware sprite register set for
   * the hardware sprite to which that vsprite is assigned.
   * This pointer array contains one pointer to the last
   * set of three colors (from the vsprite structure *sprColors)
   * for each hardware sprite.  
   *
   * As the system is scanning to determine which hardware 
   * sprite should next be used to represent a vsprite, it
   * checks the contents of this array.  If a hardware sprite
   * is available and already has been assigned this set of
   * colors, no color assignment is needed, and therefore
   * no color change instructions will be generated for the
   * copper list.
   *
   * If all vsprites use a different set of sprColors, (pointers
   * to sprColors are different for all vsprites), then there
   * is a limit of 4 vsprites on a horizontal line.  If, on
   * the other hand, you define, lets say 8 vsprites, with
   * 1 and 2 having the same sprColors, 3 and 4 the same as
   * each other, 5 and 6 the same as each other, and 7 and 8
   * also having the same vsprite colors, then you will be
   * able to have all 8 vsprites on the same horizontal line.
   *
   * In this case, you will be able to put all 8 vsprites on
   * the same horizontal line.  The reason this helps is that
   * the system hardware shares the color registers between pairs
   * of hardware sprites.  The system thus has enough resources
   * to assign all vsprites to hardware sprites in that there
   * are 4 color-sets for 8 vsprites, exactly matching the 
   * hardware maximum capabilities.
   *
   * Note that lastcolor will not be used for bobs. Just sprites.
   */
   if ((g->lastColor = (WORD **)AllocMem(sizeof(LONG) * 8,
         MEMF_PUBLIC | MEMF_CLEAR)) == NULL) {
#ifdef DEBUG
      kprintf("ReadyGels: No memory for lastcolor.\n");
#endif
      return(-1);
   }

  /* This is a table of pointers to the routines which should
   * be performed when DoCollision senses a collision.  This
   * declaration may not be necessary for a basic vsprite with
   * no collision detection implemented, but then it makes for
   * a complete example.
   */
   if ((g->collHandler = (struct collTable *)AllocMem(sizeof(struct
         collTable), MEMF_PUBLIC | MEMF_CLEAR)) == NULL) {
#ifdef DEBUG
      kprintf("ReadyGels: No memory for collHandler.\n");
#endif
      return(-1);
   }

  /* When any part of the object touches or passes across
   * this boundary, it will cause the boundary collision
   * routine to be called.  This is at smash[0] in the
   * collision handler table and is called only if
   * DoCollision is called.
   */
   g->leftmost = 0;
   g->rightmost = r->BitMap->BytesPerRow * 8 - 1;
   g->topmost = 0;
   g->bottommost = r->BitMap->Rows - 1;

   r->GelsInfo = g;  /* Link together the two structures */

   InitGels(SpriteHead, SpriteTail, g );

  /* Pointers initialized to the dummy sprites which will be
   * used by the system to keep track of the animation system.
   */
   SetCollision(0, border_dummy, g);
   WaitTOF();
   return(0);
}

/*******************************************************************************
 * Use this to get rid of the gels stuff when it is not needed any more.
 * You must have allocated the gels info stuff (use the ReadyGels routine).
 */

PurgeGels(g)
struct GelsInfo *g;
{
   if (g->collHandler != NULL)
      FreeMem(g->collHandler, sizeof(struct collTable));
   if (g->lastColor != NULL)
      FreeMem(g->lastColor, sizeof(LONG) * 8);
   if (g->nextLine != NULL)
      FreeMem(g->nextLine, sizeof(WORD) * 8);
   if (g->gelHead != NULL)
      FreeMem(g->gelHead, sizeof(struct VSprite));
   if (g->gelTail != NULL)
      FreeMem(g->gelTail, sizeof(struct VSprite));
}

/******************************************************************************
 * Because MakeVSprite is called by MakeBob, MakeVSprite only creates the
 * VSprite,it doesn't add it to the system list. The calling routine must
 * do an AddVSprite after it is created.
 */

struct VSprite *MakeVSprite(lineheight, image, colorset, x, y,
				wordwidth, imagedepth, flags)
SHORT lineheight;	/* How tall is this vsprite? */
WORD *image;		/* Where is the vsprite image data, should be
			   twice as many words as the value of lineheight */
WORD *colorset;		/* Where is the set of three words which describes
			   the colors that this vsprite can take on? */
SHORT x, y;		/* What is its initial onscreen position? */
SHORT wordwidth, imagedepth, flags;
{
   struct VSprite *v;	/* Make a pointer to the vsprite structure which
			   this routine dynamically allocates */

   if ((v = (struct VSprite *)AllocMem(sizeof(struct VSprite),
         MEMF_PUBLIC | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      printf("MakeVSprite: Couldn't allocate VSprite.\n");
#endif
      return(0);
   }

   v->Flags = flags;	/* Is this a vsprite, not a bob? */

   v->Y = y;		/* Establish initial position relative to */
   v->X = x;		/* the Display coordinates. */

   v->Height = lineheight;	/* The Caller says how high it is. */
   v->Width = wordwidth;	/* A vsprite is always 1 word (16 bits) wide. */

  /* There are two kinds of depth... the depth of the image itself, and the
   * depth of the playfield into which it will be drawn. The image depth
   * says how much data space will be needed to store an image if it's
   * dynamically allocated. The playfield depth establishes how much space
   * will be needed to save and restore the background when a bob is drawn.
   * A vsprite is always 2 planes deep, but if it's being used to make a
   * bob, it may be deeper...
   */

   v->Depth = imagedepth;

  /* Assume that the caller at least has a default boundary collision
   * routine.... bit 1 of this mask is reserved for boundary collision
   * detect during DoCollision(). The only collisions reported will be
   * with the borders. The caller can change all this later.
   */

   v->MeMask = 1;
   v->HitMask = 1;

   v->ImageData = image;	/* Caller says where to find the image. */

  /* Show system where to find a mask which is a squished down version
   * of the vsprite (allows for fast horizontal border collision detect).
   */

   if ((v->BorderLine = (WORD *)AllocMem((sizeof(WORD)*wordwidth),
         MEMF_PUBLIC | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("MakeVSprite: Couldn't allocate BorderLine.\n");
#endif
      return(0);
   }

  /* Show system where to find the mask which contains a 1 bit for any
   * position in the object in any plane where there is a 1 bit (all planes
   * OR'ed together).
   */

   if ((v->CollMask = (WORD *)AllocMem(sizeof(WORD)*lineheight*wordwidth,
         MEMF_CHIP | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("MakeVSprite: Couldn't allocate CollMask.\n");
#endif
      return(0);
   }

  /* This isn't used for a Bob, just a VSprite. It's where the
   * Caller says where to find the VSprites colors.
   */
   v->SprColors = colorset;

   /* These aren't used for a VSprite, and MakeBob'll do set up for Bob. */
   v->PlanePick = 0x00;
   v->PlaneOnOff = 0x00;

   InitMasks(v);	/* Create the collMask and borderLine */	
   return(v);
}

struct Bob *MakeBob(bitwidth,lineheight,imagedepth,image,
      planePick,planeOnOff, x,y, flags)
SHORT bitwidth,lineheight,imagedepth,planePick,planeOnOff,x,y,flags;
WORD *image;
{
   struct Bob *b;
   struct VSprite *v;
   SHORT wordwidth;

   wordwidth = (bitwidth+15)/16;

  /* Create a vsprite for this bob, it will need to be deallocated
   * later (freed) when this bob gets deleted.
   * Note: No color set for bobs.
   */
   if ((v = MakeVSprite(lineheight, image, NULL, x, y, wordwidth,
         imagedepth, flags)) == 0) {
#ifdef DEBUG
      kprintf("MakeBob: MakeVSprite failed.\n");
#endif
      return(0);
   }

   /* Caller selects which bit planes into which the image is drawn. */
   v->PlanePick = planePick;

   /* What happens to the bit planes into which the image is not drawn. */
   v->PlaneOnOff = planeOnOff;

   if ((b = (struct Bob *)AllocMem(sizeof(struct Bob),
         MEMF_PUBLIC | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("MakeBob: Couldn't allocate bob.\n");
#endif
      return(0);
   }

   v->VSBob = b; /* Link together the bob and its vsprite structures */

   b->Flags = 0; /* Not part of an animation (BOBISCOMP) and don't keep the
			image present after bob is removed (SAVEBOB) */ 

  /* Tell where to save background. Must have enough space for as many
   * bitplanes deep as the display into which everything is being drawn.
   */

   if ((b->SaveBuffer = (WORD *)AllocMem(sizeof(SHORT) * wordwidth
         * lineheight * imagedepth, MEMF_CHIP | MEMF_CLEAR)) == 0) {
#ifdef DEBUG
      kprintf("MakeBob: Couldn't allocate save buffer.\n");
#endif
      return(0);
   }

   b->ImageShadow = v->CollMask;

  /* Interbob priorities are set such that the earliest defined bobs have
   * the lowest priority, last bob defined is on top.
   */

   b->Before = NULL;	/* Let the caller worry about priority later. */
   b->After = NULL;

   b->BobVSprite = v;

  /* InitMasks does not preset the imageShadow ... caller may elect to use
   * the collMask or to create his own version of a shadow, although it
   * is usually the same.
   */

   b->BobComp = NULL;	/* this is not part of an animation */
   b->DBuffer = NULL;	/* this is not double buffered */

  /* Return a pointer to this newly created bob for additional caller
   * interaction or for AddBob(b);
   */
   return(b);
}

/* Deallocate memory which has been allocated by the routines Makexxx. */
/* Assumes images and imageshadow deallocated elsewhere. */
DeleteGel(v)
struct VSprite *v;
{
   if (v != NULL) {
      if (v->VSBob != NULL) {
         if (v->VSBob->SaveBuffer != NULL) {
            FreeMem(v->VSBob->SaveBuffer, sizeof(SHORT) * v->Width
                  * v->Height * v->Depth);
         }
         if (v->VSBob->DBuffer != NULL) {
            if (v->VSBob->DBuffer->BufBuffer != 0) {
               FreeMem(v->VSBob->DBuffer->BufBuffer,
                     sizeof(SHORT) * v->Width * v->Height * v->Depth);
            }
            FreeMem(v->VSBob->DBuffer, sizeof(struct DBufPacket));
         }
         FreeMem( v->VSBob, sizeof(struct Bob));
      }
      if (v->CollMask != NULL) {
         FreeMem(v->CollMask, sizeof(WORD) * v->Height * v->Width);
      }
      if (v->BorderLine != NULL) {
         FreeMem(v->BorderLine, sizeof(WORD) * v->Width);
      }
      FreeMem(v, sizeof(struct VSprite));
   }
}


/* intuall.h */

/*** intuall.h **************************************************************
 *
 *  intuall.h, general includer for intuition
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 *                  		Modification History
 *	date	    author :   	Comments
 *	------      ------      -------------------------------------
 *	1-30-85     -=RJ=-      created this file!
 *
 ****************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h> 
#include <exec/lists.h>
/* #include <exec/interrupts.h> */
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/devices.h>

#include <devices/console.h>
#include <devices/timer.h>
#include <devices/keymap.h>
#include <devices/inputevent.h>

#define Msg IOStdReq		/* temporary kluge for dosextens.h */

#include <libraries/dos.h>
#include <libraries/dosextens.h>


#include <graphics/gfx.h>	/* ALWAYS INCLUDE GFX.H before other includes */
#include <graphics/regions.h>	/* new as of 7/9/85 */
#include <hardware/blit.h>

#define blitNode bltnode	/* temporary kludge for gels.h */

#include <graphics/collide.h>
#include <graphics/copper.h> 
#include <graphics/display.h>
#include <hardware/dmabits.h>
#include <graphics/gels.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
/* #include <hardware/intbits.h> */
#include <hardware/custom.h> 
#include <graphics/gfxmacros.h>
#include <graphics/layers.h>
#include <intuition/intuition.h>	/* changed so I can get gadget addr */
#include <devices/gameport.h>

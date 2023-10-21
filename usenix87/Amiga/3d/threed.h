/***************************************************************/
threed.h
/***************************************************************/
#define UP 0
#define DOWN 1
#define OFF 0
#define ON 1

#define LEFT 32 

#define NTSC_TOP 4
#define PAL_TOP 0

#define WIDTH 256

#define NTSC_HEIGHT 192
#define PAL_HEIGHT 256

#define TMPWIDTH 512
#define TMPHEIGHT 512

#define NTSC_MAXHINOLACE 200
#define PAL_MAXHINOLACE 256

#define NTSC_MAXHILACE 400
#define PAL_MAXHILACE 512

#define N_BIT_PLANES 3
#define MAXNUMCOLORS 32
#define MAXNUMARGS 16

#define SINA 0x0400
#define COSA 0x3FE0
#define SINB 0x0100
#define COSB 0x3FFE

#define NULLPROC 0
#define ADDVECT   1
#define SUBVECT 2
#define ROLL 3
#define PITCH 4
#define YAW 5
#define TRANSPOSE 6

struct UV 
{
    WORD uv11;
    WORD uv12;
    WORD uv13;
    WORD uv21;
    WORD uv22;
    WORD uv23;
    WORD uv31;
    WORD uv32;
    WORD uv33;
};

struct Coordinate
{
    WORD x;
    WORD y;
    WORD z;
};

struct Polygon
{
    WORD vertexcount;
    struct Coordinate *(*vertexstart)[];
    struct Coordinate *normalvector;
    WORD polycolor;
};

struct Object
{
    struct Object *nextobject;
    struct Object *subobject;
    struct UV *umatrix;
    struct Coordinate *position;
    WORD pointcount;
    struct Coordinate *(*pointstart)[];
    WORD normalcount;
    struct Coordinate *(*normalstart)[];
    WORD polycount;
    struct Polygon *(*polystart)[];
    APTR procedure;
};

struct Objectinfo {
    struct Objectinfo *nextobjectinfo;
    struct Objectinfo *subobjectinfo;
    struct UV *objectmatrix;
    struct Coordinate *objectposition;
    WORD objectnumpoints;
    struct Coordinate **objectpoints;
    WORD objectnumnormals;
    struct Coordinate **objectnormals;
    WORD objectnumpolys;
    struct Polygon **objectpolys;
    struct UV *displaymatrix;
    struct Coordinate *displayposition;
    WORD objectbufpointsize;
    struct Coordinate *objectbufpoints;
    WORD objectbufnormalsize;
    struct Coordinate *objectbufnormals;
    WORD pptrbufsize;
    struct Coordinate *pptrbuf;
    WORD nptrbufsize;
    struct Coordinate *nptrbuf;
    WORD colorbufsize;
    struct WORD *colorbuf;
    APTR objectprocedure;
};

/***************************************************************/
joystick.h
/***************************************************************/
#define BUTTON_RIGHT        1
#define BUTTON_LEFT         2
#define BUTTON_BACK	     4
#define BUTTON_FORWARD	     8
#define BUTTON_DOWN  16
#define BUTTON_UP    32
#define NO_BUTTON    64
#define ACTION       128

/***************************************************************/
threed.c
/***************************************************************/
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>

#include <graphics/layers.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include "threed.h"
#include "joystick.h"

/* #define DEBUG */
/* #define ODEBUG */
/* #define DRAWDEBUG */
/* #define TICKDEBUG */
/* #define TICKPRINT */
/* #define JOYDEBUG */
/* #define OLDALLOCATE */
/* #define OLDCAMERA */
#define FIXEDILLUMINATION
#define CYCLECOLORS

#define NUM_OF_PATTERNS 2

#define IS_PAL (((struct GfxBase *)GfxBase)->DisplayFlags & PAL)

UWORD patterns[NUM_OF_PATTERNS][8] =
{
   0,0,0,0,0,0,0,0,       /* solid penb */
   -1,-1,-1,-1,-1,-1,-1,-1,   /* solid pena */
};

#ifndef FIXEDILLUMINATION

UWORD colorpalette[MAXNUMCOLORS] =
{
    0x0000,
    0x0222,
    0x0444,
    0x0666,
    0x0888,
    0x0aaa,
    0x0ccc,
    0x0fff,
    0x0000,
    0x0222,
    0x0444,
    0x0666,
    0x0888,
    0x0aaa,
    0x0ccc,
    0x0fff,
    0x0000,
    0x0222,
    0x0444,
    0x0666,
    0x0888,
    0x0aaa,
    0x0ccc,
    0x0fff,
    0x0000,
    0x0222,
    0x0444,
    0x0666,
    0x0888,
    0x0aaa,
    0x0ccc,
    0x0fff,
};

#else

UWORD colorpalette[MAXNUMCOLORS] =
{
    0x0456,
    0x0006,
    0x0009,
    0x000c,
    0x000f,
    0x0004,
    0x0f00,
    0x0fff,
    0x0888,
    0x0999,
    0x0aaa,
    0x0bbb,
    0x0ccc,
    0x0ddd,
    0x0eee,
    0x0fff,
    0x0000,
    0x000f,
    0x000c,
    0x0008,
    0x000c,
    0x0004,
    0x0f00,
    0x0fff,
    0x0888,
    0x0999,
    0x0aaa,
    0x0bbb,
    0x0ccc,
    0x0ddd,
    0x0eee,
    0x0fff
};

#endif

UBYTE title[] = "3D WINDOW";

extern struct Custom custom;
 
struct TmpRas tmpras;

struct BitMap bitmap0;
struct BitMap bitmap1;

struct RastPort r[2];
struct RastPort *rp[2];

struct RasInfo ri[2];
struct RasInfo *rip[2];

struct RasInfo *irip;

WORD pcount = 0;
WORD vcount = 0;

UWORD frametoggle = 0;

struct Objectinfo *universeobjectinfo = NULL;
struct Objectinfo *cameraobjectinfo = NULL;

BPTR objectsegment = NULL;

struct Object *Amiga = NULL;

struct UV *cameramatrix = NULL;
struct Coordinate *cameraposition = NULL;

extern struct Object Camera1;

extern struct UV camera1matrix;
extern struct Coordinate camera1position;
extern int camera1procedure();

extern long GfxBase;
extern long DosBase;

extern struct Object *UniverseObject;

/******************************************************************************/

do3d(view,screen,window,state)
struct View *view;
struct Screen *screen;
struct Window *window;
ULONG state;
{
    int error = FALSE;

    /* copy rastport from *(window->RPort) to r[0], r[1] */

    r[0] = *(window->RPort);
    r[1] = *(window->RPort);

    /* force non-layer interperetation of r[0,1] */

    /* r[0].Layer = 0; */
    /* r[1].Layer = 0; */

    /* set NOCROSSFILL flag in r[0,1] */

    r[0].Flags |= NOCROSSFILL;
    r[1].Flags |= NOCROSSFILL;

    /* point r[0,1] at tmpras */

    r[0].TmpRas = &tmpras;
    r[1].TmpRas = &tmpras;

    /* point r[0] at bitmap0, r[1] at bitmap1 */

    r[0].BitMap = &bitmap0;
    r[1].BitMap = &bitmap1;

    /* copy rasinfo from *((&screen->ViewPort)->Rasinfo) to ri[0], ri[1] */ 

    ri[0] = *((&screen->ViewPort)->RasInfo);
    ri[1] = *((&screen->ViewPort)->RasInfo);

    /* point ri[0] at bitmap0, ri[1] at bitmap1 */

    ri[0].BitMap = &bitmap0;
    ri[1].BitMap = &bitmap1;

    /* clear the raster that we're NOT displaying */

    SetRast(rp[frametoggle ^ 0x0001],0);

#ifdef OLDCAMERA

    /* rotate the camera matrix */
    {

   if (!(state & BUTTON_DOWN))
   {
       if (state & BUTTON_FORWARD)
       {
      struct Coordinate cfb;

      cfb.x = (cameraobjectinfo->objectmatrix->uv31 < 0)?(cameraobjectinfo->objectmatrix->uv31)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv31)>>8 ;
      cfb.y = (cameraobjectinfo->objectmatrix->uv32 < 0)?(cameraobjectinfo->objectmatrix->uv32)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv32)>>8 ;
      cfb.z = (cameraobjectinfo->objectmatrix->uv33 < 0)?(cameraobjectinfo->objectmatrix->uv33)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv33)>>8 ;

      addvect(&cfb,cameraposition,cameraposition);
       }

       if (state & BUTTON_BACK )
       {
      struct Coordinate cfb;

      cfb.x = (cameraobjectinfo->objectmatrix->uv31 < 0)?(cameraobjectinfo->objectmatrix->uv31)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv31)>>8 ;
      cfb.y = (cameraobjectinfo->objectmatrix->uv32 < 0)?(cameraobjectinfo->objectmatrix->uv32)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv32)>>8 ;
      cfb.z = (cameraobjectinfo->objectmatrix->uv33 < 0)?(cameraobjectinfo->objectmatrix->uv33)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv33)>>8 ;

      subvect(&cfb,cameraposition,cameraposition);
       }

       if (state & BUTTON_RIGHT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      addvect(&crl,cameraposition,cameraposition);
       }

       if (state &  BUTTON_LEFT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      subvect(&crl,cameraposition,cameraposition);
       }


   }
   else
   {
       if (state & BUTTON_FORWARD)
       {
      struct Coordinate cud;

      cud.x = (cameraobjectinfo->objectmatrix->uv21 < 0)?(cameraobjectinfo->objectmatrix->uv21)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv21)>>8 ;
      cud.y = (cameraobjectinfo->objectmatrix->uv22 < 0)?(cameraobjectinfo->objectmatrix->uv22)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv22)>>8 ;
      cud.z = (cameraobjectinfo->objectmatrix->uv23 < 0)?(cameraobjectinfo->objectmatrix->uv23)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv23)>>8 ;

      subvect(&cud,cameraposition,cameraposition);
       }

       if (state & BUTTON_BACK )
       {
      struct Coordinate cud;

      cud.x = (cameraobjectinfo->objectmatrix->uv21 < 0)?(cameraobjectinfo->objectmatrix->uv21)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv21)>>8 ;
      cud.y = (cameraobjectinfo->objectmatrix->uv22 < 0)?(cameraobjectinfo->objectmatrix->uv22)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv22)>>8 ;
      cud.z = (cameraobjectinfo->objectmatrix->uv23 < 0)?(cameraobjectinfo->objectmatrix->uv23)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv23)>>8 ;

      addvect(&cud,cameraposition,cameraposition);
       }

       if (state & BUTTON_RIGHT)   yaw(cameraobjectinfo->objectmatrix,-SINB,COSB);
       if (state & BUTTON_LEFT )   yaw(cameraobjectinfo->objectmatrix, SINB,COSB);
   }

    }

#else

    /* rotate the camera matrix */
    {

   if (!(state & BUTTON_DOWN))
   {
       if (state & BUTTON_FORWARD)
       {
      struct Coordinate cfb;

      cfb.x = (cameraobjectinfo->objectmatrix->uv31 < 0)?(cameraobjectinfo->objectmatrix->uv31)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv31)>>8 ;
      cfb.y = (cameraobjectinfo->objectmatrix->uv32 < 0)?(cameraobjectinfo->objectmatrix->uv32)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv32)>>8 ;
      cfb.z = (cameraobjectinfo->objectmatrix->uv33 < 0)?(cameraobjectinfo->objectmatrix->uv33)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv33)>>8 ;

      addvect(&cfb,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
      /* keep amiga behind the screen */

      if (cameraobjectinfo->objectposition->z > -1536) cameraobjectinfo->objectposition->z = -1536;

      /* keep amiga behind the screen */
       }

       if (state & BUTTON_BACK )
       {
      struct Coordinate cfb;

      cfb.x = (cameraobjectinfo->objectmatrix->uv31 < 0)?(cameraobjectinfo->objectmatrix->uv31)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv31)>>8 ;
      cfb.y = (cameraobjectinfo->objectmatrix->uv32 < 0)?(cameraobjectinfo->objectmatrix->uv32)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv32)>>8 ;
      cfb.z = (cameraobjectinfo->objectmatrix->uv33 < 0)?(cameraobjectinfo->objectmatrix->uv33)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv33)>>8 ;

      subvect(&cfb,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       if (state & BUTTON_RIGHT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      addvect(&crl,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       if (state &  BUTTON_LEFT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      subvect(&crl,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }


   }
   else
   {

       if (state & BUTTON_FORWARD)
       {
      struct Coordinate cud;

      cud.x = (cameraobjectinfo->objectmatrix->uv21 < 0)?(cameraobjectinfo->objectmatrix->uv21)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv21)>>8 ;
      cud.y = (cameraobjectinfo->objectmatrix->uv22 < 0)?(cameraobjectinfo->objectmatrix->uv22)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv22)>>8 ;
      cud.z = (cameraobjectinfo->objectmatrix->uv23 < 0)?(cameraobjectinfo->objectmatrix->uv23)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv23)>>8 ;

      addvect(&cud,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       if (state & BUTTON_BACK )
       {
      struct Coordinate cud;

      cud.x = (cameraobjectinfo->objectmatrix->uv21 < 0)?(cameraobjectinfo->objectmatrix->uv21)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv21)>>8 ;
      cud.y = (cameraobjectinfo->objectmatrix->uv22 < 0)?(cameraobjectinfo->objectmatrix->uv22)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv22)>>8 ;
      cud.z = (cameraobjectinfo->objectmatrix->uv23 < 0)?(cameraobjectinfo->objectmatrix->uv23)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv23)>>8 ;

      subvect(&cud,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       if (state & BUTTON_RIGHT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      addvect(&crl,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       if (state &  BUTTON_LEFT)
       {
      struct Coordinate crl;

      crl.x = (cameraobjectinfo->objectmatrix->uv11 < 0)?(cameraobjectinfo->objectmatrix->uv11)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv11)>>8 ;
      crl.y = (cameraobjectinfo->objectmatrix->uv12 < 0)?(cameraobjectinfo->objectmatrix->uv12)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv12)>>8 ;
      crl.z = (cameraobjectinfo->objectmatrix->uv13 < 0)?(cameraobjectinfo->objectmatrix->uv13)>>8|0xFF00:(cameraobjectinfo->objectmatrix->uv13)>>8 ;

      subvect(&crl,cameraobjectinfo->objectposition,cameraobjectinfo->objectposition);
       }

       /*

       if (state & BUTTON_RIGHT)   yaw(cameraobjectinfo->objectmatrix,-SINB,COSB);
       if (state & BUTTON_LEFT )   yaw(cameraobjectinfo->objectmatrix, SINB,COSB);
       */
   }

    }

#endif

    /* draw all the objects in the list headed by firsobjectinfo */

#ifdef OLDCAMERA

#ifdef DISPLAYDEBUG
    printf("do3d: call display(%lx)...\n",universeobjectinfo);
#endif

    display(view,screen,window,universeobjectinfo);

#ifdef DISPLAYDEBUG
    printf("do3d: returned from display(%lx)...\n",universeobjectinfo);
#endif

#else

#ifdef DISPLAYDEBUG
    printf("do3d: call displayuniverse(%lx)...\n",universeobjectinfo);
#endif

    displayuniverse(view,screen,window,cameraobjectinfo,universeobjectinfo);

#ifdef DISPLAYDEBUG
    printf("do3d: returned from displayuniverse(%lx)...\n",universeobjectinfo);
#endif

#endif

    /* for double buffering */

    frametoggle = frametoggle ^ 0x0001;

    return(error);

}

toggle(onoff)
WORD *onoff;
{
    switch (*onoff)
    {
   case OFF:   *onoff = ON;
         break;

   case ON:   *onoff = OFF;
         break;

   default:   break;
    }

}

test3d(view,screen,window)
struct View *view;
struct Screen *screen;
struct Window *window;
{
    int tickcount = 0;
    int error = FALSE;
    WORD actiontoggle = OFF;
    struct IntuiMessage *message;
    WORD mousex, mousey;
    WORD i, j, k;
    WORD mousetoggle = UP ;
    WORD messagenum = 0;
    ULONG framecount = 0;
    ULONG state = 0;
    struct IOStdReq *ior;


#ifdef DEBUG
    printf("test3d: entering test3d...\n");
#endif

    /* open the gameport device for joystick input */

    ior = open_joystick();

    /* set colors for this screen */

    for(i = 0; i < (1<<N_BIT_PLANES); i++)
    {
   SetRGB4(&screen->ViewPort,i,(colorpalette[i]>>8)&0xF,
                (colorpalette[i]>>4)&0xF,(colorpalette[i])&0xF);
    }

    /* copy rastport from *(window->RPort) to r[0], r[1] */

    r[0] = *(window->RPort);
    r[1] = *(window->RPort);

    /* force non-layer interperetation of r[0,1] */

    /* r[0].Layer = 0; */
    /* r[1].Layer = 0; */

    /* set NOCROSSFILL flag in r[0,1] */

    r[0].Flags |= NOCROSSFILL;
    r[1].Flags |= NOCROSSFILL;

    /* point r[0,1] at tmpras */

    r[0].TmpRas = &tmpras;
    r[1].TmpRas = &tmpras;

    /* point rp[0] at r[0], rp[1] at r[1] */

    rp[0] = &r[0];
    rp[1] = &r[1];

    /* point r[0] at bitmap0, r[1] at bitmap1 */

    r[0].BitMap = &bitmap0;
    r[1].BitMap = &bitmap1;

    /* clone rasinfo pointer from (&screen->ViewPort->RasInfo) to irip */

    irip = (&screen->ViewPort)->RasInfo;

    /* copy rasinfo from *((&screen->ViewPort)->RasInfo) to ri[0], ri[1] */ 

    ri[0] = *((&screen->ViewPort)->RasInfo);
    ri[1] = *((&screen->ViewPort)->RasInfo);

    /* point ri[1] at bitmap1 */
    /* don't point r1[0] at bitmap0 yet, so that we can clear initial rastport bitmap */

    ri[1].BitMap = &bitmap1;

    /* point rip[0] at ri[0], rip[1] at ri[1] */

    rip[0] = &ri[0];
    rip[1] = &ri[1];

    /* clear the raster that we ARE displaying */

    SetRast(rp[frametoggle],0);

    /* reset the frametoggle */

    frametoggle = 0;

#ifdef OLDCAMERA

    /* allocate a cameramatrix for this display */

    if ((cameramatrix = (struct UV *)AllocMem(sizeof(struct UV),MEMF_PUBLIC)) == NULL) 
    {
   return(error); 
    }

    /* initialize the cameramatrix */

    matrixinit(cameramatrix);

    /* allocate a cameraposition for this display */

    if ((cameraposition = (struct Coordinate *)AllocMem(sizeof(struct Coordinate),MEMF_PUBLIC)) == NULL) 
    {
   return(error); 
    }

    /* initialize the cameraposition */

    positioninit(cameraposition);
    cameraposition->z = 0xfa00;

#else

    /* allocate a cameraobject for this display */

    allocateobjectinfolist(view,screen,window,&cameraobjectinfo,&Camera1);

    /* initialize the cameramatrix */

    matrixinit(cameraobjectinfo->objectmatrix);

    /* initialize the cameraposition */

    positioninit(cameraobjectinfo->objectposition);

    cameraobjectinfo->objectposition->z = 0xfa00;

#endif

    /* allocate objectinfo structures */

#ifdef OLDALLOCATE
    {
   struct Object *ob;

   /* for all the objects in this objectsegment */

   for (ob = Amiga; ob; ob = ob->nextobject)
   {
       struct Objectinfo *thisobjectinfo;

#ifdef ODEBUG
       printf("test3d: allocate objectinfo for object(%lx) ",ob);
#endif

       /* allocate an objectinfo structure for the current object */

       if ((thisobjectinfo = (struct Objectinfo *)AllocMem(sizeof(struct Objectinfo),MEMF_PUBLIC|MEMF_CLEAR)) == NULL) 
       {
      return(error); 
       }

#ifdef ODEBUG
       printf("= %lx\n",thisobjectinfo);
#endif
       /* initialize the buffers for the current 3d object */

       if(!objectinit(view,screen,window,thisobjectinfo,ob))
       {
          return(error);
       }

       /* make this objectinfo last on the objectinfo list */
       {
      struct Objectinfo **oipp;

      oipp =  &universeobjectinfo;
      while (*oipp)
      {
          oipp = &((*oipp)->nextobjectinfo);
      }
      *oipp = thisobjectinfo;
       thisobjectinfo->nextobjectinfo = NULL;
       }

   }

    }
#else
    
    allocateobjectinfolist(view,screen,window,&universeobjectinfo,Amiga);

#endif

#ifdef ODEBUG
    printf("test3d: &universeobjectinfo = %lx\n",&universeobjectinfo);
    {
   struct Objectinfo *oip;

   printf("    OBJECTINFO LIST     \n");
   printf("_________________________\n");

   for (oip = universeobjectinfo; oip; oip = oip->nextobjectinfo)
          printf("    objectinfo(%lx)\n",oip);

   printf("_________________________\n");
    }
#endif

    /* now wait for instructions from the user */

    message = (struct IntuiMessage *)GetMsg(window->UserPort);

    while ( (!message) || (message->Class) != CLOSEWINDOW )
    {
   if(message) ReplyMsg(message);

   /* read joystick, if any */

   if (ior) state = test_joystick(ior,state);

#ifdef JOYDEBUG
   printf("do3d: state = %lx\n",state);
#endif

   /* call do3d */

#ifdef ODEBUG
   printf("test3d: call do3d...\n");
#endif

#ifdef TICKDEBUG

   /* through timer */

   tickcount = timeproc(do3d,view,screen,window,state);

#ifdef TICKPRINT
   printf("%lx\n",tickcount);
#endif

#else
   error = do3d(view,screen,window,state);

#ifdef ODEBUG
   printf("test3d: returned from call to do3d... error = %lx\n",error);
#endif

#endif

   /* did 3d, increment framecount */

   framecount += 17;

#ifdef CYCLECOLORS

   /* cycle colors for AMIGA polygons */

   SetRGB4(&screen->ViewPort,6,((framecount & 0x00000fff)>>8 )&0xF,
                ((framecount & 0x00000fff)>>4 )&0xF,
                ((framecount & 0x00000fff)    )&0xF);
   SetRGB4(&screen->ViewPort,7,((framecount & 0x00000f00)>>10)+0xB,
                ((framecount & 0x000000f0)>>6 )+0xB,
                ((framecount & 0x0000000f)>>2 )+0xB);

#endif

   /* double buffer */

   Forbid();
   WaitTOF();
   WaitBlit();
   Disable();
   (&screen->ViewPort)->RasInfo = rip[frametoggle];
   ScrollVPort(&screen->ViewPort);
   Enable();
   Permit();

   message = (struct IntuiMessage *)GetMsg(window->UserPort);
    }
    ReplyMsg(message);

    /* deallocate objectinfo structures */

#ifdef ODEBUG
    printf("test3d: deallocate the active objectinfo structures...\n");
#endif

#ifdef OLDALLOCATE
    while( universeobjectinfo )
    {
   struct Objectinfo *oip;

   /* delink the first objectinfo from the objectinfo list */

   oip = universeobjectinfo;

   universeobjectinfo = universeobjectinfo->nextobjectinfo;

   /* deallocate the buffers dependent on the current objectinfo */

#ifdef ODEBUG
    printf("    deallocate objectinfo(%lx)\n",oip);
#endif

   objectdeallocate(view,screen,window,oip);

   /* deallocate this objectinfo structure itself */

   FreeMem(oip,sizeof(struct Objectinfo));
    }
#else

    deallocateobjectinfolist(view,screen,window,&universeobjectinfo);

#endif

#ifdef OLDCAMERA

    /* free camera allocations */

    if(cameramatrix)
      FreeMem(cameramatrix,sizeof(struct UV));

    if(cameraposition)
      FreeMem(cameraposition,sizeof(struct Coordinate));

#else

    deallocateobjectinfolist(view,screen,window,&cameraobjectinfo);

#endif

    /* if joystick, close device */

    if (ior) close_joystick(ior);

    /* restore rasinfo pointer from irip to (view->ViewPort->RasInfo) */

    view->ViewPort->RasInfo = irip;

    return(error);

}

notest3d(view,screen,window)
struct View *view;
struct Screen *screen;
struct Window *window;
{
    int tickcount = 0;
    int error = 0;
    WORD actiontoggle = OFF;
    struct IntuiMessage *message;
    WORD mousex, mousey;
    WORD i, j, k;
    WORD mousetoggle = UP ;
    WORD messagenum = 0;
    ULONG framecount = 0;

    /* now wait for instructions from the user */

    message = (struct IntuiMessage *)GetMsg(window->UserPort);

    while ( (!message) || (message->Class) != CLOSEWINDOW )
    {
   if(message) ReplyMsg(message);
   message = (struct IntuiMessage *)GetMsg(window->UserPort);
    }
    ReplyMsg(message);

    return(0);

}


donothing(view,screen,window)
struct View *view;
struct Screen *screen;
struct Window *window;
{
    int tickcount = 0;
    int error = 0;
    WORD actiontoggle = OFF;
    struct IntuiMessage *message;
    WORD mousex, mousey;
    WORD i, j, k;
    WORD mousetoggle = UP ;
    WORD messagenum = 0;
    ULONG framecount = 0;

    /* now wait for instructions from the user */

    message = (struct IntuiMessage *)GetMsg(window->UserPort);

    while ( (!message) || (message->Class) != CLOSEWINDOW )
    {
   if(message) ReplyMsg(message);

   message = (struct IntuiMessage *)GetMsg(window->UserPort);
    }
    ReplyMsg(message);

    return(0);

}

/**********************************************************************************/

main()
{
    int (*usrproc)();
    WORD left = LEFT;
    WORD top = IS_PAL? PAL_TOP : NTSC_TOP ;
    WORD width = WIDTH;
    WORD height = IS_PAL? PAL_HEIGHT : NTSC_HEIGHT ;
    WORD n_bit_planes = N_BIT_PLANES;
    WORD i, j;
    int error = FALSE;

    usrproc = test3d;

    /* open the dos library */

    if ( (DosBase = OpenLibrary("dos.library",0) ) == NULL)
    {
#ifdef DEBUG
   printf("main: can't open dos lib...\n");
#endif
   exit(-1);
    }

    /* load the object data segment */

#ifdef GREENHILLS

    if ( (objectsegment = LoadSeg("3dobject") ) == NULL)
    {
#ifdef DEBUG
   printf("main: can't load objectsegment...\n");
#endif
   exit(0);
    }
    else
    {
   Amiga = (struct Object *)BADDR( (objectsegment+1) );
    }

#else

   Amiga = UniverseObject;
 
#endif

    /* open the graphics library */

    if ( (GfxBase = OpenLibrary("graphics.library",0) ) == NULL)
    {
#ifdef DEBUG
   printf("main: can't open graphics lib...\n");
#endif
   exit(-1);
    }

    /* open a window in a custom screen under dos */

    /* first, allocate a temporary raster for request routine to use */

#ifdef DEBUG
   printf("main: allocate a temporary raster...\n");
#endif

    if ((tmpras.RasPtr = (BYTE *)AllocRaster(TMPWIDTH,TMPHEIGHT)) == NULL)
    {
#ifdef DEBUG
   printf("main: can't allocate tmpras memory...\n");
#endif
   error = TRUE;
    }   
    else
    {
   tmpras.Size = RASSIZE(TMPWIDTH,TMPHEIGHT);
    }

    /* now set up a bitmap for 3d routine to use */

#ifdef DEBUG
   printf("main: set up a super-bit map...\n");
#endif
    
    bitmap0.BytesPerRow = ( ((width+15)>>3) & 0xFFFE);
    bitmap0.Rows = ((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE));
    bitmap0.Flags = 0;
    bitmap0.Depth = n_bit_planes;
   
    for (i=0; i< n_bit_planes; i++)
    {
   if(!error)
   {
       if ( (bitmap0.Planes[i] = (PLANEPTR)AllocMem(RASSIZE(width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))),MEMF_CHIP|MEMF_CLEAR) ) == NULL)
       {
#ifdef DEBUG
      printf("main: can't allocate bitmap memory...\n");
#endif
      error = TRUE;
      for (--i; i >= 0; --i) FreeRaster(bitmap0.Planes[i],width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE)));
        
       }
       else
       {
#ifdef DEBUG
      printf("main: allocated  bitmap0.Planes[%lx] = %lx\n",i,bitmap0.Planes[i]);
#endif
       }
   }
    }

    /* now set up an alternate bitmap for 3d routine to use for double buffering */

#ifdef DEBUG
   printf("main: set up an alternate bitmap...\n");
#endif
    
    bitmap1.BytesPerRow = ( ((width+15)>>3) & 0xFFFE);
    bitmap1.Rows = ((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE));
    bitmap1.Flags = 0;
    bitmap1.Depth = n_bit_planes;
   
    for (i=0; i< n_bit_planes; i++)
    {
   if(!error)
   {
       if ( (bitmap1.Planes[i] = (PLANEPTR)AllocMem(RASSIZE(width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))),MEMF_CHIP|MEMF_CLEAR) ) == NULL)
       {
#ifdef DEBUG
      printf("main: can't allocate bitmap memory...\n");
#endif
      error = TRUE;
      for (--i; i >= 0; --i) FreeRaster(bitmap1.Planes[i],width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE)));
       }
       else
       {
#ifdef DEBUG
      printf("main: allocated  bitmap1.Planes[%lx] = %lx\n",i,bitmap1.Planes[i]);
#endif
       }
   }
    }

#ifdef DEBUG
    printf("main: call startgfxdos()...\n");
#endif

    if(!error)
    {
   error = startgfxdos(left,top,height,width,n_bit_planes,usrproc,title,SIMPLE_REFRESH|BACKDROP|ACTIVATE,&tmpras,NULL);
    }    

#ifdef ODEBUG
    printf("returned from startgfxdos with error = %lx\n",error);
#endif

    if(!error)
    {
#ifdef ODEBUG
   printf("main: clean up...\n");
#endif

#ifdef ODEBUG
   printf("main: free tempras memory...\n");
#endif
   FreeRaster(tmpras.RasPtr,TMPWIDTH,TMPHEIGHT);

#ifdef ODEBUG
   printf("main: free bitmap0 memory...\n");
#endif
   for (i=0; i< n_bit_planes; i++)
   {
#ifdef ODEBUG
       printf("main: freeing bitmap0.Plane[%lx] = %lx\n",i,bitmap0.Planes[i]);
#endif
       FreeRaster(bitmap0.Planes[i],width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE)));
   }

#ifdef ODEBUG
   printf("main: free bitmap1 memory...\n");
#endif
   for (i=0; i< n_bit_planes; i++)
   {
#ifdef ODEBUG
       printf("main: freeing bitmap1.Plane[%lx] = %lx\n",i,bitmap1.Planes[i]);
#endif
       FreeRaster(bitmap1.Planes[i],width,((height>(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE))?(IS_PAL?PAL_MAXHILACE:NTSC_MAXHILACE):(IS_PAL?PAL_MAXHINOLACE:NTSC_MAXHINOLACE)));
   }

    }

    /* unload the objectsegment before exiting */

#ifdef GREENHILLS

#ifdef ODEBUG
    printf("main: unload the objectsegment before exiting...\n");
#endif

    UnLoadSeg(objectsegment);

#endif

    /* exit main */

#ifdef ODEBUG
    printf("main: exit main...\n");
#endif


}
/***************************************************************/

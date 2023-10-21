/** ShowILBM.c **************************************************************
 *
 * Read an ILBM raster image file and display it.  11/19/85.
 *
 * By Jerry Morrison, Steve Shaw, and Steve Hayes, Electronic Arts.
 * This software is in the public domain.
 *
 * USE THIS AS AN EXAMPLE PROGRAM FOR AN IFF READER.
 *
 * The IFF reader portion is essentially a recursive-descent parser.
 * This program will look into a CAT or LIST to find a FORM ILBM, but it
 * won't look inside another FORM type for a nested FORM ILBM.
 *
 * The display portion is specific to the Commodore Amiga computer.
 *
 * NOTE: This program displays an image, pauses, then exits. It doesn't
 * bother to switch you back to the CLI or workbench "screen", so type
 * Amiga-N when it's done.
 *
 ****************************************************************************/

#include "graphics/system.h"
#include "libraries/dos.h"
#include "iff/ilbm.h"

/* This example's max number of planes in a bitmap. Could use MaxAmDepth. */
#define EXDepth 5
#define maxColorReg (1<<EXDepth)
#define MIN(a,b) ((a)<(b)?(a):(b))

#define SafeFreeMem(p,q) {if(p)FreeMem(p,q);}

/* general usage pointers */
struct GfxBase *GfxBase;

/* Globals for displaying an image */
struct RastPort rP;
struct BitMap bitmap;
struct RasInfo rasinfo;
struct View v = {0};
struct ViewPort vp = {0};

/* Define the size of a temporary buffer used in unscrambling the ILBM rows.*/
#define bufSz 512

/* Message strings for IFFP codes. */
char MsgOkay[]        = { "(IFF_OKAY) No FORM ILBM in the file." };
char MsgEndMark[]     = { "(END_MARK) How did you get this message?" };
char MsgDone[]        = { "(IFF_DONE) All done."};
char MsgDos[]         = { "(DOS_ERROR) The DOS returned an error." };
char MsgNot[]         = { "(NOT_IFF) Not an IFF file." };
char MsgNoFile[]      = { "(NO_FILE) No such file found." };
char MsgClientError[] = { "(CLIENT_ERROR) ShowILBM bug or insufficient RAM."};
char MsgForm[]        = { "(BAD_FORM) A malformed FORM ILBM." };
char MsgShort[]       = { "(SHORT_CHUNK) A malformed FORM ILBM." };
char MsgBad[]         = { "(BAD_IFF) A mangled IFF file." };

/* THESE MUST APPEAR IN RIGHT ORDER!! */
char *IFFPMessages[-LAST_ERROR+1] = {
    /*IFF_OKAY*/  MsgOkay,
    /*END_MARK*/  MsgEndMark,
    /*IFF_DONE*/  MsgDone,
    /*DOS_ERROR*/ MsgDos,
    /*NOT_IFF*/   MsgNot,
    /*NO_FILE*/   MsgNoFile,
    /*CLIENT_ERROR*/ MsgClientError,
    /*BAD_FORM*/  MsgForm,
    /*SHORT_CHUNK*/  MsgShort,
    /*BAD_IFF*/   MsgBad
    };

/*------------ ILBM reader -----------------------------------------------*/
/* ILBMFrame is our "client frame" for reading FORMs ILBM in an IFF file.
 * We allocate one of these on the stack for every LIST or FORM encountered
 * in the file and use it to hold BMHD & CMAP properties. We also allocate
 * an initial one for the whole file.
 * We allocate a new GroupContext (and initialize it by OpenRIFF or
 * OpenRGroup) for every group (FORM, CAT, LIST, or PROP) encountered. It's
 * just a context for reading (nested) chunks.
 *
 * If we were to scan the entire example file outlined below:
 *    reading          proc(s)                new               new
 *
 * --whole file--   ReadPicture+ReadIFF   GroupContext        ILBMFrame
 * CAT              ReadICat                GroupContext
 *   LIST           GetLiILBM+ReadIList       GroupContext        ILBMFrame
 *     PROP ILBM    GetPrILBM                   GroupContext
 *       CMAP       GetCMAP
 *       BMHD       GetBMHD
 *     FORM ILBM    GetFoILBM                   GroupContext        ILBMFrame
 *       BODY       GetBODY
 *     FORM ILBM    GetFoILBM                   GroupContext        ILBMFrame
 *       BODY       GetBODY
 *   FORM ILBM      GetFoILBM                 GroupContext        ILBMFrame
 */
typedef struct {
   ClientFrame clientFrame;
   UBYTE foundBMHD;
   UBYTE nColorRegs;
   BitMapHeader bmHdr;
   Color4 colorMap[maxColorReg];
   /* If you want to read any other property chunks, e.g. GRAB or CAMG, add
    * fields to this record to store them. */
   } ILBMFrame;


/* NOTE: For a simple version of this program, set Fancy to 0.
 * That'll compile a program that skips all LISTs and PROPs in the input
 * file. It will look in CATs for FORMs ILBM. That's suitable for most uses.
 *
 * For a fancy version that handles LISTs and PROPs, set Fancy to 1. */
#define Fancy  0


/** DisplayPic() ************************************************************
 *
 * Interface to Amiga graphics ROM routines.
 *
 ****************************************************************************/
DisplayPic(ptilbmFrame)  ILBMFrame *ptilbmFrame;  {
    int i;
    struct View *oldView = GfxBase->ActiView;   /* so we can restore it */

    InitView(&v);
    InitVPort(&vp);
    v.ViewPort = &vp;
    InitRastPort(&rP);
    rP.BitMap = &bitmap;
    rasinfo.BitMap = &bitmap;

    /* Always show the upper left-hand corner of this picture. */
    rasinfo.RxOffset = 0;
    rasinfo.RyOffset = 0;

    vp.DWidth = ptilbmFrame->bmHdr.pageWidth;   /* Physical display WIDTH */
    vp.DHeight = ptilbmFrame->bmHdr.pageHeight; /* Display height */

#if 0
    /* Specify where on screen to put the ViewPort. */
    vp.DxOffset = ptilbmFrame->bmHdr.x;
    vp.DyOffset = ptilbmFrame->bmHdr.y;
#else
    /* Always display it in upper left corner of screen.*/
#endif

    if (ptilbmFrame->bmHdr.pageWidth <= 320)
        vp.Modes = 0;
    else vp.Modes = HIRES;
    if (ptilbmFrame->bmHdr.pageHeight > 200) {
        v.Modes |= LACE;
        vp.Modes |= LACE;
        }
    vp.RasInfo = &rasinfo;
    MakeVPort(&v,&vp);
    MrgCop(&v);
    LoadView(&v);       /* show the picture */
    WaitBlit();
    WaitTOF();
    LoadRGB4(&vp, ptilbmFrame->colorMap, ptilbmFrame->nColorRegs);

    for (i = 0; i < 5*60; ++i)  WaitTOF();      /* Delay 5 seconds. */

    LoadView(oldView);  /* switch back to old view */
    }

/** GetFoILBM() *************************************************************
 *
 * Called via ReadPicture to handle every FORM encountered in an IFF file.
 * Reads FORMs ILBM and skips all others.
 * Inside a FORM ILBM, it stops once it reads a BODY. It complains if it
 * finds no BODY or if it has no BMHD to decode the BODY.
 *
 * Once we find a BODY chunk, we'll allocate the BitMap and read the image.
 *
 ****************************************************************************/
IFFP GetFoILBM(parent)  GroupContext *parent;  {
   /*compilerBug register*/ IFFP iffp;
   GroupContext formContext;
   ILBMFrame ilbmFrame;         /* only used for non-clientFrame fields.*/
   register int i;
   int plsize;  /* Plane size in bytes. */
   int nPlanes; /* number of planes in our display image */
   BYTE buffer[bufSz];

   if (parent->subtype != ID_ILBM)
      return(IFF_OKAY); /* just continue scaning the file */

   ilbmFrame = *(ILBMFrame *)parent->clientFrame;
   iffp = OpenRGroup(parent, &formContext);
   CheckIFFP();

   do switch (iffp = GetFChunkHdr(&formContext)) {
      case ID_BMHD: {
        ilbmFrame.foundBMHD = TRUE;
        iffp = GetBMHD(&formContext, &ilbmFrame.bmHdr);
        break; }
      case ID_CMAP: {
        ilbmFrame.nColorRegs = maxColorReg;  /* we have room for this many */
        iffp = GetCMAP(
           &formContext, (WORD *)&ilbmFrame.colorMap, &ilbmFrame.nColorRegs);
        break; }
      case ID_BODY: {
         if (!ilbmFrame.foundBMHD)  return(BAD_FORM);   /* No BMHD chunk! */

         nPlanes = MIN(ilbmFrame.bmHdr.nPlanes, EXDepth);
         InitBitMap(
            &bitmap,
            nPlanes,
            ilbmFrame.bmHdr.w,
            ilbmFrame.bmHdr.h);
         plsize = RowBytes(ilbmFrame.bmHdr.w) * ilbmFrame.bmHdr.h;
         if (bitmap.Planes[0] =
                (PLANEPTR)AllocMem(nPlanes * plsize, MEMF_CHIP))
            {
            for (i = 1; i < nPlanes; i++)
                bitmap.Planes[i] = (PLANEPTR) bitmap.Planes[0] + plsize*i;
            iffp = GetBODY(
                &formContext,
                &bitmap,
                NULL,
                &ilbmFrame.bmHdr,
                buffer,
                bufSz);
            if (iffp == IFF_OKAY) iffp = IFF_DONE;      /* Eureka */
            }
         else
            iffp = CLIENT_ERROR;        /* not enough RAM for the bitmap */
         break; }
      case END_MARK: { iffp = BAD_FORM; break; } /* No BODY chunk! */
      } while (iffp >= IFF_OKAY);  /* loop if valid ID of ignored chunk or a
                          * subroutine returned IFF_OKAY (no errors).*/

   if (iffp != IFF_DONE)  return(iffp);

   /* If we get this far, there were no errors. */
   CloseRGroup(&formContext);
   DisplayPic(&ilbmFrame);
   return(iffp);
   }

/** Notes on extending GetFoILBM ********************************************
 *
 * To read more kinds of chunks, just add clauses to the switch statement.
 * To read more kinds of property chunks (GRAB, CAMG, etc.) add clauses to
 * the switch statement in GetPrILBM, too.
 *
 * To read a FORM type that contains a variable number of data chunks--e.g.
 * a FORM FTXT with any number of CHRS chunks--replace the ID_BODY case with
 * an ID_CHRS case that doesn't set iffp = IFF_DONE, and make the END_MARK
 * case do whatever cleanup you need.
 *
 ****************************************************************************/

/** GetPrILBM() *************************************************************
 *
 * Called via ReadPicture to handle every PROP encountered in an IFF file.
 * Reads PROPs ILBM and skips all others.
 *
 ****************************************************************************/
#if Fancy
IFFP GetPrILBM(parent)  GroupContext *parent;  {
   /*compilerBug register*/ IFFP iffp;
   GroupContext propContext;
   ILBMFrame *ilbmFrame = (ILBMFrame *)parent->clientFrame;

   if (parent->subtype != ID_ILBM)
      return(IFF_OKAY); /* just continue scaning the file */

   iffp = OpenRGroup(parent, &propContext);
   CheckIFFP();

   do switch (iffp = GetPChunkHdr(&propContext)) {
      case ID_BMHD: {
        ilbmFrame->foundBMHD = TRUE;
        iffp = GetBMHD(&propContext, &ilbmFrame->bmHdr);
        break; }
      case ID_CMAP: {
        ilbmFrame->nColorRegs = maxColorReg; /* we have room for this many */
        iffp = GetCMAP(
          &propContext, (WORD *)&ilbmFrame->colorMap, &ilbmFrame->nColorRegs);
        break; }
      } while (iffp >= IFF_OKAY);  /* loop if valid ID of ignored chunk or a
                          * subroutine returned IFF_OKAY (no errors).*/

   CloseRGroup(&propContext);
   return(iffp == END_MARK ? IFF_OKAY : iffp);
   }
#endif

/** GetLiILBM() *************************************************************
 *
 * Called via ReadPicture to handle every LIST encountered in an IFF file.
 *
 ****************************************************************************/
#if Fancy
IFFP GetLiILBM(parent)  GroupContext *parent;  {
    ILBMFrame newFrame; /* allocate a new Frame */

    newFrame = *(ILBMFrame *)parent->clientFrame;  /* copy parent frame */

    return( ReadIList(parent, (ClientFrame *)&newFrame) );
    }
#endif

/** ReadPicture() ***********************************************************
 *
 * Read a picture from an IFF file, given a file handle open for reading.
 *
 ****************************************************************************/
IFFP ReadPicture(file)  LONG file;  {
   ILBMFrame iFrame;    /* Top level "client frame".*/
   IFFP iffp = IFF_OKAY;

#if Fancy
   iFrame.clientFrame.getList = GetLiILBM;
   iFrame.clientFrame.getProp = GetPrILBM;
#else
   iFrame.clientFrame.getList = SkipGroup;
   iFrame.clientFrame.getProp = SkipGroup;
#endif
   iFrame.clientFrame.getForm = GetFoILBM;
   iFrame.clientFrame.getCat  = ReadICat ;

   /* Initialize the top-level client frame's property settings to the
    * program-wide defaults. This example just records that we haven't read
    * any BMHD property or CMAP color registers yet. For the color map, that
    * means the default is to leave the machine's color registers alone.
    * If you want to read a property like GRAB, init it here to (0, 0). */
   iFrame.foundBMHD  = FALSE;
   iFrame.nColorRegs = 0;

   iffp = ReadIFF(file, (ClientFrame *)&iFrame);

   Close(file);
   return(iffp);
   }

/** main0() *****************************************************************/
void main0(filename)  char *filename;  {
    LONG file;
    IFFP iffp = NO_FILE;

    /* load and display the picture */
    if( !(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0)) )
        exit(0);
    file = Open(filename, MODE_OLDFILE);
    if (file)
        iffp = ReadPicture(file);

    printf(" %s\n", IFFPMessages[-iffp]);

    /* cleanup */
    if (bitmap.Planes[0])  {
        FreeMem(bitmap.Planes[0],
                bitmap.BytesPerRow * bitmap.Rows * bitmap.Depth);
        FreeVPortCopLists(&vp);
        FreeCprList(v.LOFCprList);
        }
    CloseLibrary(GfxBase);
    }

/** main() ******************************************************************/
void main(argc, argv)  int argc;  char **argv;  {
    printf("Showing file '%s' ...", argv[1]);
    if (argc < 2)
        printf("\nUsage: 'ShowILBM filename'");
    else
        main0(argv[1]);
    printf("\n");
    exit(0);
    }


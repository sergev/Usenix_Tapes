/** Raw2ILBM.c **************************************************************
 *
 * Read an raw raster image file and write an IFF FORM ILBM file.  11/15/85.
 *
 * By Jerry Morrison and Steve Shaw, Electronic Arts.
 * This software is in the public domain.
 *
 * USE THIS AS AN EXAMPLE PROGRAM FOR AN IFF WRITER.
 *
 ****************************************************************************/
#include "graphics/system.h"
#include "iff/ilbm.h"

/* Size of the buffer for PutBODY. */
#define bufSize 512


/** PutAnILBM() *************************************************************
 *
 * Write an entire BitMap as a FORM ILBM in an IFF file.
 * This procedure assumes the image is in the Amiga's 320 x 200 display mode.
 *
 * Normal return result is IFF_OKAY.
 *
 * The utility program IFFCheck would print the following outline of the
 * resulting file:
 *
 *   FORM ILBM
 *     BMHD
 *     CMAP
 *     BODY       (compressed)
 *
 ****************************************************************************/
#define CkErr(expression)  {if (ifferr == IFF_OKAY) ifferr = (expression);}

IFFP PutAnILBM(file, bitmap, mask, colorMap, depth, xy, buffer, bufsize)
      LONG file;  struct BitMap *bitmap;  BYTE *mask;  WORD *colorMap;
      UBYTE depth;  Point2D *xy;  BYTE *buffer;  LONG bufsize;
   {
   BitMapHeader bmHdr;
   GroupContext fileContext, formContext;
   IFFP ifferr;

   ifferr = InitBMHdr(&bmHdr, bitmap, mskNone, cmpByteRun1, 0, 320, 200);
        /* You could write an uncompressed image by passing cmpNone instead
         * of cmpByteRun1 to InitBMHdr. */
   bmHdr.nPlanes = depth;       /* This must be  <= bitmap->Depth */
   if (mask != NULL) bmHdr.masking = mskHasMask;
   bmHdr.x = xy->x;   bmHdr.y = xy->y;

   CkErr( OpenWIFF(file, &fileContext, szNotYetKnown) );
   CkErr(StartWGroup(&fileContext, FORM, szNotYetKnown, ID_ILBM, &formContext));

   CkErr( PutBMHD(&formContext, &bmHdr) );
   CkErr( PutCMAP(&formContext, colorMap, depth) );
   CkErr( PutBODY(&formContext, bitmap, mask, &bmHdr, buffer, bufsize) );

   CkErr( EndWGroup(&formContext) );
   CkErr( CloseWGroup(&fileContext) );
   return( ifferr );
   }

/** PutPicture() ************************************************************
 *
 * Put a picture into an IFF file.
 * This procedure calls PutAnILBM, passing in an <x, y> location of <0, 0>,
 * a NULL mask, and a locally-allocated buffer. It also assumes you want to
 * write out all the bitplanes in the BitMap.
 *
 ****************************************************************************/
Point2D nullPoint = {0, 0};

IFFP PutPicture(file, bitmap, colorMap)
      LONG file;  struct BitMap *bitmap;  WORD *colorMap;
   {
   BYTE buffer[bufSize];
   return( PutAnILBM(file, bitmap, NULL,
                     colorMap, bitmap->Depth, &nullPoint,
                     buffer, bufSize) );
   }

/* [TBD] More to come. We need code to read a "raw" raster file into a BitMap
 * and code to handle file opening & closing, error msgs, etc. */


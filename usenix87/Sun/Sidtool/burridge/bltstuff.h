
/*  bltstuff.h
 *
 *  RasterOp definitions used by Sid Tool.
 *  Written by Rich Burridge - SUN Microsystems Australia (Melbourne).
 *
 *  Version 2.1.  -  April 1987.
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#define  RRPL           PIX_SRC                  /* RASTEROP codes. */
#define  ROR            PIX_SRC | PIX_DST
#define  RXOR           PIX_SRC ^ PIX_DST
#define  RCLR           PIX_CLR
#define  RSET           PIX_SET
#define  RINV           PIX_NOT(PIX_DST)

/* Machine independent rasterop calls. */

/* Manipulate a portion of the screen with itself. */
#define  BLT_SCRN(sx,sy,w,h,op) \
         (pw_writebackground(pw,sx,sy,w,h,op))

/* Move an offscreen raster area to the screen. */
#define  BLT_MEM_TO_SCRN(sx,sy,w,h,op,mem,mx,my) \
         (pw_write(pw,sx,sy,w,h,op,mem,mx,my))

/* Move an offscreen raster area to another offscreen raster area. */
#define  BLT_MEM(mem1,mx1,my1,w,h,op,mem2,mx2,my2) \
         ((void) pr_rop(mem1,mx1,my1,w,h,op,mem2,mx2,my2))

/* Write text at x,y using rasterop function sfunc and font pf. */
#define  WRITELN(x,y,text) \
         (pw_text(pw,x,y,sfunc,pf,text))

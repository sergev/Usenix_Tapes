
/*  bltstuff.h
 *
 *  Graphics definitions used by the SID tool.
 *  Written by Rich Burridge - SUN Australia, June 1986.
 *
 *  Version 1.5
 *
 *  No responsibility is taken for any errors inherent either to the code
 *  or the comments of this program, but if reported to me then an attempt
 *  will be made to fix them.
 */

#include <pixrect/pixrect_hs.h>
#include <suntool/gfx_hs.h>

extern struct gfxsubwindow *gfx ;     /* Pointer to SID tool window. */

#define  RRPL           PIX_SRC                  /* RASTEROP codes. */
#define  ROR            PIX_SRC | PIX_DST
#define  RXOR           PIX_SRC ^ PIX_DST
#define  RCLR           PIX_CLR
#define  RSET           PIX_SET
#define  RINV           PIX_NOT(PIX_DST)
#define  RXNOR          PIX_SRC ^ PIX_NOT(PIX_DST)
#define  RORNOT         PIX_NOT(PIX_SRC) | PIX_DST
#define  RANDNOT        PIX_NOT(PIX_SRC) & PIX_DST

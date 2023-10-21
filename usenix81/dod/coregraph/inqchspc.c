#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchspc                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchspc(dx,dy,dz)
   float *dx,*dy,*dz;
   {
    *dx = current.chrspace.dx;
    *dy = current.chrspace.dy;
    *dz = 0.0;
    return(0);
   }



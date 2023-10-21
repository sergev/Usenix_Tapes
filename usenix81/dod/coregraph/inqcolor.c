#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqcolor                                                   */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqcolor(color)
   float color[]; /*** pointer to array of three reals passed ***/
   {
    color[0] = current.color[0];
    color[1] = current.color[1];
    color[2] = current.color[2];
    return(0);
   }



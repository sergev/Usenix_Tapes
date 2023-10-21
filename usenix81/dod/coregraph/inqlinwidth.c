#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqlinwidth                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqlinwidth(linwidth)
   float *linwidth;
   {
    *linwidth = current.linwidth;
    return(0);
   }



#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchsize                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchsize(chwidth,cheight)
   float *chwidth,*cheight;
   {
    *chwidth = current.charsize.width;
    *cheight = current.charsize.height;
    return(0);
   }



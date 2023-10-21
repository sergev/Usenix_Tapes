#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqfont                                                    */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqfont(font)
   int *font;
   {
    *font = current.font;
    return(0);
   }



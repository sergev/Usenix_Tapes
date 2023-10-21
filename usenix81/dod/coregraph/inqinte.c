#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqintensity                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqintensity(intensty)
   float *intensty;
   {
    *intensty = current.intensty;
    return(0);
   }



#include "inclusions.c"


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqchquality                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqchquality(chqualty)
   int *chqualty;
   {
    *chqualty = current.chqualty;
    return(0);
   }



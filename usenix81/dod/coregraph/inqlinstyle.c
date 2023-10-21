#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqlinstyle                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqlinstyle(linestyl)
   int *linestyl;
   {
    *linestyl = current.linestyl;
    return(0);
   }



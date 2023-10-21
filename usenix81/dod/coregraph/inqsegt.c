#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqsegtype                                                 */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqsegtype(segtype)
   int *segtype;
   {
   *segtype = csegtype;
   return(0);
   }


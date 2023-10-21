#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inqpickid                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

inqpickid(pickid)
   int *pickid;
   {
    *pickid = current.pick2id;
    return(0);
   }

#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setchquality                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setchquality(chqualty)
   int chqualty;
   {
    char *funcname;
    int errnum;

    funcname = "setchquality";
    if (chqualty < minimum.chqualty || chqualty > maximum.chqualty)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(errnum);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(errnum);
       }
    current.chqualty = chqualty;
    qualflag = TRUE;
    return(0);
   }





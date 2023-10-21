#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setpickid                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setpickid(pickid)
   int pickid;
   {
    char *funcname;
    int errnum;

    funcname = "setpickid";
    if (pickid < minimum.pick2id)
       {
       errnum = 27;
       errhand(funcname,errnum);
       return(2);
       }
    if (osexists == FALSE)
       {
       errnum = 26;
       errhand(funcname,errnum);
       return(1);
       }
    picflag = TRUE;
    current.pick2id = pickid;
    return(0);
   }












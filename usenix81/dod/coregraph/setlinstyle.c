#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setlinstyle                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setlinstyle(linestyl)
   int linestyl;
   {
    char *funcname;
    int errnum;

    funcname = "setlinstyle";
    if (linestyl < minimum.linestyl || linestyl > maximum.linestyl)
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
    lsflag = TRUE;
    current.linestyl = linestyl;
    return(0);
   }


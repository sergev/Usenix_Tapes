#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setlinwidth                                                */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setlinwidth(linwidth)
   float linwidth;
   {
    char *funcname;
    int errnum;

    funcname = "setlinwidth";
    if (linwidth < minimum.linwidth || linwidth > maximum.linwidth)
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
    lwflag = TRUE;
    current.linwidth = linwidth;
    return(0);
   }


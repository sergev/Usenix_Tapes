#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setintensity                                               */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setintensity(intensty)
   float intensty;
   {
    char *funcname;
    int errnum;

    funcname = "setintensity";
    if (intensty < minimum.intensty || intensty > maximum.intensty)
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
    intflag = TRUE;
    current.intensty = intensty;
    return(0);
   }


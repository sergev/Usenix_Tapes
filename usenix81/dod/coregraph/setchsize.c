#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setchsize                                                  */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setchsize(chwidth,cheight)
   float chwidth,cheight;
   {
    char *funcname;
    int errnum;

    funcname = "setchsize";
    if (chwidth < minimum.charsize.width || cheight < minimum.charsize.height)
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
    szeflag = TRUE;
    current.charsize.width = chwidth;
    current.charsize.height = cheight;
    return(0);
   }



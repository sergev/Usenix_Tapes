#include "inclusions.c"

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setfont                                                    */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setfont(font)
   int font;
   {
    char *funcname;
    int errnum;

    funcname = "setfont";
    if (font < minimum.font || font > maximum.font)
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
    fntflag = TRUE;
    current.font = font;
    return(0);
   }



#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: movabs2                                                    */
/*                                                                          */
/*     PURPOSE: THE CP IS SET TO THE VALUE (x,y) WHERE (x,y) IS A POSITION  */
/*              IN THE WORLD COORDINATE SYSTEM.                             */
/*                                                                          */
/****************************************************************************/

movabs2(x,y)
   float x,y;
   {
   char *funcname;
   int errnum;

   funcname = "movabs2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }

   cp[0] = x;
   cp[1] = y;

   cpchang = TRUE;
   return(0);
   }



#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: movrel2                                                    */
/*                                                                          */
/*     PURPOSE: THE CP IS SET TO THE VALUE (x,y) WHERE (x,y) IS A POSITION  */
/*              IN THE WORLD COORDINATE SYSTEM RELATIVE TO PREVIOUS (x,y).  */
/*                                                                          */
/****************************************************************************/

movrel2(dx,dy)
   float dx,dy;
   {
   char *funcname;
   int errnum;

   funcname = "movrel2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }

   cp[0] = cp[0] + dx;
   cp[1] = cp[1] + dy;

   cpchang = TRUE;
   return(0);
   }





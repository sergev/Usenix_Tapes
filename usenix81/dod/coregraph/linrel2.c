#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: linrel2                                                    */
/*                                                                          */
/*     PURPOSE: DESCRIBE A LINE OF AN OBJECT IN WORLD COORDINATES.THIS LINE */
/*              RUNS FROM CP TO THE POSITION SPECIFIED. CP IS UPDATED.      */
/*                                                                          */
/****************************************************************************/

linrel2(dx,dy)
   float dx,dy;
   {
   char *funcname;
   int errnum;

   funcname = "linrel2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }

   return(linabs2(cp[0] + dx,cp[1] + dy));
   }



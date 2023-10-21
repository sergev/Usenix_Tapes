#include "inclusions.c"

double sin(),cos(),atan2();

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: setsegtype                                                 */
/*                                                                          */
/*     PURPOSE: LISTED ABOVE                                                */
/*                                                                          */
/****************************************************************************/

setsegtype(segtype)
   int segtype;
   {
   char *funcname;
   int errnum;

   funcname = "setsegtype";
   if ((segtype < sgtypmin) || (segtype > sgtypmax))
      {
      errnum = 28;
      errhand(funcname,errnum);
      return(1);
      }
   csegtype = segtype;
   return(0);
   }


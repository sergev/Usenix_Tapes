#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: plyrel2                                                    */
/*                                                                          */
/*     PURPOSE: DESCRIBE A CONNECTED SEQUENCE OF LINES IN AN OBJECT IN      */
/*              WORLD COORDINATES.                                          */
/*                                                                          */
/****************************************************************************/

plyrel2(xcoord,ycoord,n)
   float xcoord[],ycoord[];
   int n;
   {
   char *funcname;
   int errnum;
   register int index;

   funcname = "plyrel2";

   if (! osexists)
      {
      errnum = 21;
      errhand(funcname,errnum);
      return(1);
      }
   if (n <= 0)
      {
      errnum = 22;
      errhand(funcname,errnum);
      return(2);
      }

   for (index = 0; index < n; index++)
       {
       linrel2(xcoord[index],ycoord[index]);
       }

   return(0);
   }



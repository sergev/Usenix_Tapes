#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: plyabs2                                                   */
/*                                                                          */
/*     PURPOSE: DESCRIBE A CONNECTED SEQUENCE OF LINES IN AN OBJECT IN      */
/*              WORLD COORDINATES.                                          */
/*                                                                          */
/****************************************************************************/

plyabs2(xcoord,ycoord,n)
   float xcoord[],ycoord[];
   int n;
   {
   char *funcname;
   int errnum;
   register int index;

   funcname = "plyabs2";

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

   movabs2(xcoord[0],ycoord[0]);

   for (index = 1; index < n; index++)
      {
      linabs2(xcoord[index],ycoord[index]);
      }

   return(0);
   }


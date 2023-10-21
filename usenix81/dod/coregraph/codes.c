#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: codes                                                      */
/*                                                                          */
/*     PURPOSE: ASSIGN A CODE TO THE POINT SPECIFIED.                       */
/*                                                                          */
/****************************************************************************/

codes(x,y,code)
   float x,y;
   int *code;
   {

   *code = 0;
   if(x < window.xmin)
      {
      (*code)++;
      }
   else if(x > window.xmax)
      {
      *code =+ 2;
      }
   if(y < window.ymin)
      {
      *code =+ 4;
      }
   else if(y > window.ymax)
      {
      *code =+ 8;
      }

   return(0);
   }







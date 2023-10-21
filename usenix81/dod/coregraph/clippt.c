#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: clippt                                                     */
/*                                                                          */
/*     PURPOSE: CLIP POINT AGAINST CURRENT VIEWING WINDOW BY DETERMINING    */
/*              IF POINT OF PLACEMENT IS ON OR OFF SCREEN.                  */
/*                                                                          */
/****************************************************************************/

clippt(x,y,returnx,returny,flag)
   float x,y,*returnx,*returny;
   int *flag;
   {

   if(vwrotate)
      {
      rotate(x,y,&x,&y);
      }

   if((x < window.xmin) || (x > window.xmax) || (y < window.ymin) || (y > window.ymax))
      {
      *flag = TRUE;
      return(0);
      }
   else *flag = FALSE;

   *returnx = x;
   *returny = y;

   return(0);
   }







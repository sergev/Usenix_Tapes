#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: rotate                                                     */
/*                                                                          */
/*     PURPOSE: ROTATE A POINT THROUGH A CLOCKWISE ANGLE ABOUT THE ORIGIN.  */
/*                                                                          */
/****************************************************************************/

rotate(passedx,passedy,returnx,returny)
   float passedx,passedy;
   float *returnx,*returny;
   {

   *returnx = passedx * viewup[1] + passedy * viewup[0];
   *returny = -passedx * viewup[0] + passedy * viewup[1];

   return(0);
   }






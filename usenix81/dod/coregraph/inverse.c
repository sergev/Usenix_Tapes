#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: inverse                                                    */
/*                                                                          */
/*     PURPOSE: REVERSE IMAGE TRANSFORM NORMALIZED DEVICE COORDINATES       */
/*                                                                          */
/****************************************************************************/

inverse(x,y,returnx,returny)
   float x,y,*returnx,*returny;
   {
   float temp1,temp2,temp3,temp4,temp5,temp6;
   float detmtrx;

   /***********************************************************/
   /***                                                     ***/
   /*** SET UP TEMPORARY VALUES TO BE USED IN CALCULATIONS. ***/
   /***                                                     ***/
   /***********************************************************/

   temp1 = imxform[0][0];
   temp2 = imxform[0][1];
   temp3 = imxform[1][0];
   temp4 = imxform[1][1];
   temp5 = imxform[2][0];
   temp6 = imxform[2][1];

   /***************************************/
   /***                                 ***/
   /*** CALCULATE DETERMINANT OF MATRIX ***/
   /***                                 ***/
   /***************************************/

   detmtrx = (temp1 * temp4) - (temp3 * temp2);

   /*********************************************************/
   /***                                                   ***/
   /*** CALCULATE INVERSE VALUES, MAKING USE OF CLASSICAL ***/
   /*** ADJOINT METHOD OF FINDING INVERSE MATRIX.         ***/
   /***                                                   ***/
   /*********************************************************/

   *returnx = x * (temp4/detmtrx) + y * (-temp3/detmtrx) + (temp3 * temp6 - temp5 * temp4)/detmtrx;
   *returny = x * (-temp2/detmtrx) + y * (temp1/detmtrx) + (-(temp1 * temp6 - temp5 * temp2)/detmtrx);

   return(0);
   }






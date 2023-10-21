#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: wldtondc                                                   */
/*                                                                          */
/*     PURPOSE: CONVERTS WORLD COORDINATES TO NORMALIZED DEVICE COORDINATES.*/
/*                                                                          */
/****************************************************************************/

wldtondc(wldx,wldy,ndcx,ndcy)
   float wldx,wldy,*ndcx,*ndcy;
   {

   /*******************************/
   /***                         ***/
   /*** COMPUTE NDC COORDINATES ***/
   /*** REFER TO VWPORT2 AND    ***/
   /*** WINDOW2 FOR MAKE-UP OF  ***/
   /*** VIEWING TRANSFORMATION  ***/
   /***                         ***/
   /*******************************/

   *ndcx = wldx * vwxform[0][0] + vwxform[0][1];
   *ndcy = wldy * vwxform[1][0] + vwxform[1][1];

   return(0);
   }






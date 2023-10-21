#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: ndctowld                                                   */
/*                                                                          */
/*     PURPOSE: CONVERTS NORMALIZED DEVICE COORDINATES TO WORLD COORDINATES.*/
/*                                                                          */
/****************************************************************************/

ndctowld(ndcx,ndcy,wldx,wldy)
   float ndcx,ndcy,*wldx,*wldy;
   {

   /*********************************/
   /***                           ***/
   /*** COMPUTE WORLD COORDINATES ***/
   /*** REFER TO VWPORT2 AND      ***/
   /*** WINDOW2 FOR MAKE-UP OF    ***/
   /*** VIEWING TRANSFORMATION    ***/
   /***                           ***/
   /*********************************/

   *wldx = ( ndcx - vwxform[0][1])/vwxform[0][0];
   *wldy = ( ndcy - vwxform[1][1])/vwxform[1][0];

   return(0);
   }













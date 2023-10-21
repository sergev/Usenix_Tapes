#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: clipbox                                                    */
/*                                                                          */
/*     PURPOSE: CLIP CHARACTER BOX AGAINST CURRENT VIEWING WINDOW.          */
/*                                                                          */
/****************************************************************************/

clipbox(x,y,boxflag)
   float x,y;
   int *boxflag;
   {
   int ptflag,index;
   int reset;          /*** BOOLEAN IN VERSION 7 ***/

   /*********************************************************************/
   /***                                                               ***/
   /*** ROTATE POINT INDICATING THE BOX,CLEAR ROTATE FLAG SO ALL FOUR ***/
   /*** POINTS OF BOX WON'T BE ROTATED, AND SET A FLAG INDICATING     ***/
   /*** ROTATE FLAG NEEDS RESETING.                                   ***/
   /***                                                               ***/
   /*********************************************************************/

   if(vwrotate)
      {
      rotate(x,y,&x,&y);
      vwrotate = FALSE;
      reset = TRUE;
      }

   /*********************************************************************/
   /***                                                               ***/
   /*** FIGURE OUT FOUR POINTS OF BOX AND 'CLIPPT' EACH ONE OF THESE. ***/
   /***                                                               ***/
   /*********************************************************************/

   for(index = 0;index < 4;index++)
      {
      switch(index)
	 {
	 case 0: y =- current.charsize.height;
		 break;
	 case 1: x =+ current.charsize.width;
		 break;
	 case 2: y =+ (2 * current.charsize.height);
		 break;
	 case 3: x =- current.charsize.width;
		 break;
	 }
      clippt(x,y,&x,&y,&ptflag);
      if(ptflag)
	 {
	 *boxflag = TRUE;
	 return(0);
	 break;
	 }
      }

   *boxflag = FALSE;

   /*********************************************************************/
   /***                                                               ***/
   /*** IF APPROPRIATE, RESET ROTATE FLAG FOR OTHER CLIPPING ROUTINES ***/
   /***                                                               ***/
   /*********************************************************************/

   if(reset)
      {
      vwrotate = TRUE;
      }

   return(0);
   }







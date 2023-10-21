#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: clipline                                                   */
/*                                                                          */
/*     PURPOSE: CLIP LINE AGAINST CURRENT VIEWING WINDOW AND RETURN THE     */
/*              END POINTS OF THE REMAINING LINE SEGMENT.                   */
/*                                                                          */
/****************************************************************************/

clipline(x,y,x1,y1,x2,y2,flag)
   float x,y,*x1,*y1,*x2,*y2;
   int *flag;   /*** BOOLEAN UNDER VERSION 7 ***/
   {
   float pseudocp[2];
   int code1,code2,save;

   /***********************************************/
   /***                                         ***/
   /*** ROTATE ENDPOINTS OF LINE IF FLAG IS SET ***/
   /***                                         ***/
   /***********************************************/

   if(vwrotate)
      {
      rotate(cp[0],cp[1],&pseudocp[0],&pseudocp[1]);
      rotate(x,y,&x,&y);
      }
   else
      {
      pseudocp[0] = cp[0];
      pseudocp[1] = cp[1];
      }

   /******************************************************/
   /***                                                ***/
   /*** COMPUTE CODES TO TWO ENDPOINTS OF LINE SEGMENT ***/
   /***                                                ***/
   /******************************************************/

   codes(x,y,&code1);
   codes(pseudocp[0],pseudocp[1],&code2);

   if(code1 == 0 && code2 == 0)
      {
      *x1 = pseudocp[0];
      *y1 = pseudocp[1];
      *x2 = x;
      *y2 = y;
      }

   while(!(code1 == 0 && code2 == 0))   /*** SOME PORTION OF LINE INVISIBLE? ***/
      {
      if((code1 & code2) != 0)   /*** LINE ENTIRELY OFF SCREEN? ***/
	 {
	 *flag = TRUE;
	 return(0);
	 }
      /******************************************************************/
      /***                                                            ***/
      /*** AT THIS JUNCTURE OF THE PROGRAM, IF THE POINT SPECIFIED IS ***/
      /*** VISIBLE, THEN THE CP IS INVISIBLE. ASSIGN POINTS SO THAT   ***/
      /*** X2,Y2 CONTAINS THE ENDPOINT TO BE ADJUSTED BY ALGORITHM.   ***/
      /***                                                            ***/
      /******************************************************************/

      if(code1 == 0)
	 {
	 *x1 = pseudocp[0];
	 *y1 = pseudocp[1];
	 *x2 = x;
	 *y2 = y;

	 save = code1;
	 code1 = code2;
	 code2 = save;
	 }
      else
	 {
	 *x1 = x;
	 *y1 = y;

	 *x2 = pseudocp[0];
	 *y2 = pseudocp[1];
	 }
      if(code1 & 1)   /*** POINT OFF LEFT SIDE OF SCREEN? ***/
	 {
	 /*****************************/
	 /***                       ***/
	 /*** PUSH TOWARD LEFT EDGE ***/
	 /***                       ***/
	 /*****************************/

	 *y1 =+ (*y2 - *y1) * (window.xmin - *x1)/(*x2 - *x1);
	 *x1 = window.xmin;
	 }
      else if(code1 & 2)   /*** POINT OFF RIGHT SIDE OF SCREEN? ***/
	 {
	 /******************************/
	 /***                        ***/
	 /*** PUSH TOWARD RIGHT EDGE ***/
	 /***                        ***/
	 /******************************/

	 *y1 =+ (*y2 - *y1) * (window.xmax - *x1)/(*x2 - *x1);
	 *x1 = window.xmax;
	 }
      else if(code1 & 4)   /*** POINT OFF BOTTOM OF SCREEN? ***/
	 {
	 /*******************************/
	 /***                         ***/
	 /*** PUSH TOWARD BOTTOM EDGE ***/
	 /***                         ***/
	 /*******************************/

	 *x1 =+ (*x2 - *x1) * (window.ymin)/(*y2 - *y1);
	 *y1 = window.ymin;
	 }
      else if(code1 & 8)   /*** POINT OFF TOP OF SCREEN? ***/
	 {
	 /*****************************/
	 /***                       ***/
	 /*** PUSH TOWARD TOP EDGE  ***/
	 /***                       ***/
	 /*****************************/

	 *x1 =+ (*x2 - *x1) * (window.ymax)/(*y2 - *y1);
	 *y1 = window.ymax;
	 }

      codes(*x1,*y1,&code1);

      }

   *flag = FALSE;
   return(0);
   }






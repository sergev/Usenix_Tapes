#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: slin2rel                                                   */
/*                                                                          */
/*     PURPOSE: DRAW STROKES USING A VARIATION OF THE LINE FUNCTION WHICH   */
/*              DOES THE SAME THINGS EXCEPT NO PDF ENTRY IS MADE, NO ERROR  */
/*              MESSAGES ARE SENT AND ONLY ONE VIEW SURFACE IS INVOLVED.    */
/*                                                                          */
/****************************************************************************/

slin2rel(dx,dy)
   float dx,dy;
   {
   float x,y,xnew,ynew;
   int flag;
   float x1,y1,x2,y2;

   x = cp[0] + dx;
   y = cp[1] + dy;

   if(wndwclip)
      {
      clipline(x,y,&x1,&y1,&x2,&y2,&flag);
      if (flag) /*** line totally clipped outside of window ***/
	 {
	 cp[0] = x;
	 cp[1] = y;
	 cpchang = TRUE;
	 return(0);
	 }
      }
   else
      {
      x1 = cp[0];
      y1 = cp[1];
      x2 = x;
      y2 = y;
      }

   wldtondc(x1,y1,&x1,&y1); /*** world to normalized device coordinates ***/
   wldtondc(x2,y2,&x2,&y2);

   if(! idenflag)
      {
      if(openseg->type == XLATE2)
	 {
	 /***********************************/
	 /***                             ***/
	 /*** PERFORM A TRANSLATION ONLY. ***/
	 /***                             ***/
	 /***********************************/

	 x1 = x1 + imxform[2][0];
	 y1 = y1 + imxform[2][1];

	 x2 = x2 + imxform[2][0];
	 y2 = y2 + imxform[2][1];
	 }
      else if(openseg->type == XFORM2)
	 {
	 /********************************************/
	 /***                                      ***/
	 /*** PERFORM A FULL IMAGE TRANSFORMATION: ***/
	 /*** ROTATE,SCALE, AND TRANSLATE.         ***/
	 /***                                      ***/
	 /********************************************/

	 xnew = x1 * imxform[0][0] + y1 * imxform[1][0] + imxform[2][0];
	 ynew = x1 * imxform[0][1] + y1 * imxform[1][1] + imxform[2][1];
	 x1 = xnew;
	 y1 = ynew;

	 xnew = x2 * imxform[0][0] + y2 * imxform[1][0] + imxform[2][0];
	 ynew = x2 * imxform[0][1] + y2 * imxform[1][1] + imxform[2][1];
	 x2 = xnew;
	 y2 = ynew;
	 }
      }

   if (cpchang)  /*** cp changed since last sent to DD ***/
      {
       ddstruct.opcode = MOVE;
       ddstruct.float1 = x1;
       ddstruct.float2 = y1;
       (*(surfptr->dev1drive))(&ddstruct);
      }

   ddstruct.opcode = LINE;
   ddstruct.float1 = x2;
   ddstruct.float2 = y2;
   (*(surfptr->dev1drive))(&ddstruct);

   cp[0] = x;
   cp[1] = y;

   return(0);
   }






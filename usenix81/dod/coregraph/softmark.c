#include "inclusions.c"

short ptype;

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: softmark                                                   */
/*                                                                          */
/*     PURPOSE: USING THE CHARACTER PARAMETERS, SIMULATE MARKER DRAWING BY  */
/*              SENDING THE APPROPRIATE LINE COMMANDS TO THE SURFACE.       */
/*                                                                          */
/****************************************************************************/

softmark(marknum)
   int marknum;
   {

   char *onechar;
   int hold2;
   float hold1,hold3,hold4,hold5,hold6;

   /*****************************************************************/
   /***                                                           ***/
   /*** CENTER MARKER - MOVE CP DOWN(HEIGHT/2) AND LEFT(WIDTH/2). ***/
   /***                                                           ***/
   /*****************************************************************/

   cp[0] = cp[0] - current.chrspace.dx/2.0;
   cp[1] = cp[1] - current.chrspace.dy/2.0;

   /*************************************************************/
   /***                                                       ***/
   /*** CONVERT MARKER NUMBER TO LENGTH ONE CHARACTER STRING. ***/
   /***                                                       ***/
   /*************************************************************/

   onechar = marknum;

   /******************************************************/
   /***                                                ***/
   /*** TEMPORARILY RESET TEXT ATTRIBUTES TO DEFAULTS. ***/
   /***                                                ***/
   /******************************************************/

   hold1 = current.linwidth;
   hold2 = current.linestyl;
   hold3 = current.charsize.width;
   hold4 = current.charsize.height;
   hold5 = current.chrspace.dx;
   hold6 = current.chrspace.dy;

   current.linwidth = defaultt.linwidth;
   current.linestyl = defaultt.linestyl;
   current.charsize.width = defaultt.charsize.width;
   current.charsize.height = defaultt.charsize.height;
   current.chrspace.dx = defaultt.chrspace.dx;
   current.chrspace.dy = defaultt.chrspace.dy;

   /**********************************************/
   /***                                        ***/
   /*** DRAW MARKER USING SOFTWARE GENERATION. ***/
   /***                                        ***/
   /**********************************************/

   softstring(onechar);

   /*************************************************/
   /***                                           ***/
   /*** RESET TEXT ATTRIBUTES TO PREVIOUS VALUES. ***/
   /***                                           ***/
   /*************************************************/

   current.linwidth = hold1;
   current.linestyl = hold2;
   current.charsize.width = hold3;
   current.charsize.height = hold4;
   current.chrspace.dx = hold5;
   current.chrspace.dy = hold6;

   return(0);
   }






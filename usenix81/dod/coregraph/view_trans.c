#include "inclusions.c"


/*** The following functions define the viewing transformation ***/

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: ndcsp2                                                     */
/*                                                                          */
/*     PURPOSE: DEFINES THE SIZE OF THE 2D NORMALIZED DEVICE COORDINATE     */
/*              SPACE WHICH CAN BE ADDRESSED ON THE VIEW SURFACE OF ALL     */
/*              DISPLAY DEVICES USED BY THE APPLICATION PROGRAM, AND WITHIN */
/*              WHICH VIEWPORTS WILL BE SPECIFIED.                          */
/*                                                                          */
/****************************************************************************/

ndcsp2(width,height)
   float width,height;
   {
   char *funcname;
   int errnum;

   funcname = "ndcsp2";
   if (ndcinvkd)
      {
      errnum = 42;
      errhand(funcname,errnum);
      return(1);
      }
   if(ndcspuse)
      {
      errnum = 43;
      errhand(funcname,errnum);
      return(2);
      }
   if ((width <= 0.0) || (width > 1.0) || (height <= 0.0) || (height > 1.0))
      {
      errnum = 44;
      errhand(funcname,errnum);
      return(3);
      }
   if ((width != 1.0) && (height != 1.0))
      {
      errnum = 45;
      errhand(funcname,errnum);
      return(4);
      }

   /*****************************************/
   /***                                   ***/
   /*** INFORM DDS OF BOUNDS OF NDC SPACE ***/
   /***                                   ***/
   /*****************************************/

   ddstruct.opcode = NDCSP2;
   ddstruct.float1 = width;
   ddstruct.float2 = height;

   for(surfptr = &surface[0];surfptr < &surface[MAXVSURF];surfptr++)
      {
      if(surfptr->vinit)
	 {
	 (*(surfptr->dev1drive))(&ddstruct);
	 }
      }


   ndcspace[0] = width;
   ndcspace[1] = height;

   /********************************************/
   /***                                      ***/
   /*** RESET DEFAULT VIEWPORT SPECIFICATION ***/
   /***                                      ***/
   /********************************************/

   vwport2(0.0,width,0.0,height);


   vfinvokd = TRUE;
   ndcinvkd = TRUE;

   return(0);
   }



/****************************************************************************/
/*                                                                          */
/*     FUNCTION: vwport2                                                    */
/*                                                                          */
/*     PURPOSE: PARAMETERS GIVE THE EXTENT IN 2D NORMALIZED DEVICE          */
/*              COORDINATE SPACE OF THE CURRENT VIEWPORT.                   */
/*                                                                          */
/****************************************************************************/

vwport2(xmin,xmax,ymin,ymax)
   float xmin,xmax,ymin,ymax;
   {
   char *funcname;
   int errnum;

   funcname = "vwport2";

   if ((xmin > ndcspace[0]) || (xmax > ndcspace[0]) || (ymin > ndcspace[1]) || (ymax > ndcspace[1]))
      {
      errnum = 46;
      errhand(funcname,errnum);
      return(1);
      }
   if ((xmin < 0.0) || (xmax < 0.0) || (ymin < 0.0) || (ymax < 0.0))
      {
      errnum = 46;
      errhand(funcname,errnum);
      return(1);
      }
   if (osexists)
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(2);
      }
   if ((xmin > xmax) || (ymin > ymax))
      {
      errnum = 47;
      errhand(funcname,errnum);
      return(3);
      }

   viewport.xmin = xmin;
   viewport.xmax = xmax;
   viewport.ymin = ymin;
   viewport.ymax = ymax;

   /*******************************************/
   /***                                     ***/
   /*** COMPUTE LENGTH OF SIDES OF VIEWPORT ***/
   /***                                     ***/
   /*******************************************/

   ndcsidx = viewport.xmax - viewport.xmin;
   ndcsidy = viewport.ymax - viewport.ymin;

   /********************************************/
   /***                                      ***/
   /*** COMPUTE VIEWING TRANSFORMATION ARRAY ***/
   /***                                      ***/
   /********************************************/

   vwxform[0][0] = ndcsidx / wldsidx;
   vwxform[0][1] = viewport.xmin - window.xmin * vwxform[0][0];
   vwxform[1][0] = ndcsidy / wldsidy;
   vwxform[1][1] = viewport.ymin - window.ymin * vwxform[1][0];

   vfinvokd = TRUE;
   ndcspuse = TRUE;
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: vwup2                                                      */
/*                                                                          */
/*     PURPOSE: SPECIFIES THE WORLD COORDINATE "up" DIRECTION, SO THAT THE  */
/*              WORLD COORDINATE Y-AXIS NEED NOT BE "up" ON THE VIEW        */
/*              SURFACE.                                                    */
/*     NOTE: viewup[0] IS ACTUALLY FIGURED TO BE -COSINE OF THE ANGLE OF    */
/*           ROTATION.                                                      */
/*           viewup[1] IS ACTUALLY FIGURED TO BE SINE OF THE ANGLE OF       */
/*           ROTATION.                                                      */
/*           BOTH WILL BE USED LATER IN ROTATION ROUTINE.                   */
/*                                                                          */
/****************************************************************************/

vwup2(dx,dy)
   float dx,dy;
   {
   char *funcname;
   int errnum;
   float hypot;
   double sqrt();

   funcname = "vwup2";
   if (osexists)
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(1);
      }
   if ((dx == 0.0) && (dy == 0.0))
      {
      errnum = 39;
      errhand(funcname,errnum);
      return(2);
      }

   /**********************************************************************/
   /***                                                                ***/
   /*** FIGURE VIEWUP VECTOR IN TERMS OF SINE AND COSINE FOR USE IN    ***/
   /*** ROTATION ROUTINE. BECAUSE THE VIEWUP VECTOR IS SPECIFIED WITH  ***/
   /*** RESPECT TO POSITIVE Y-AXIS INSTEAD OF POSITIVE X-AXIS,         ***/
   /*** viewup[0] IS EQUATED TO -COSINE , NOT COSINE.                  ***/
   /***                                                                ***/
   /**********************************************************************/

   hypot = sqrt((dx * dx) + (dy * dy));
   viewup[0] = -dx/hypot;
   viewup[1] = dy/hypot;

   if(dx == 0.0 && dy > 0.0)
      {
      vwrotate = FALSE;
      }
   else
      {
      vwrotate = TRUE;
      }

   vfinvokd = TRUE;
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: window2                                                    */
/*                                                                          */
/*     PURPOSE: PARAMETERS SPECIFY A RECTANGLE IN WORLD COORDINATES WHICH   */
/*              WHEN ROTATED(IF NECESSARY) DEFINES THE WINDOW.              */
/*                                                                          */
/****************************************************************************/

window2(xmin,xmax,ymin,ymax)
   float xmin,xmax,ymin,ymax;
   {
   char *funcname;
   int errnum;

   funcname = "window2";
   if (osexists)
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(1);
      }
   if ((xmin > xmax) || (ymin > ymax))
      {
      errnum = 47;
      errhand(funcname,errnum);
      return(2);
      }

   window.xmin = xmin;
   window.xmax = xmax;
   window.ymin = ymin;
   window.ymax = ymax;

   /*****************************************/
   /***                                   ***/
   /*** COMPUTE LENGTH OF SIDES OF WINDOW ***/
   /***                                   ***/
   /*****************************************/

   wldsidx = window.xmax - window.xmin;
   wldsidy = window.ymax - window.ymin;

   /********************************************/
   /***                                      ***/
   /*** COMPUTE VIEWING TRANSFORMATION ARRAY ***/
   /***                                      ***/
   /********************************************/

   vwxform[0][0] = ndcsidx / wldsidx;
   vwxform[0][1] = viewport.xmin - window.xmin * vwxform[0][0];
   vwxform[1][0] = ndcsidy / wldsidy;
   vwxform[1][1] = viewport.ymin - window.ymin * vwxform[1][0];

   vfinvokd = TRUE;
   return(0);
   }

/****************************************************************************/
/*                                                                          */
/*     FUNCTION: clipwindow                                                 */
/*                                                                          */
/*     PURPOSE: ENABLE OR DISABLE CLIPPING AGAINST THE WINDOW IN THE VIEW   */
/*              PLANE.                                                      */
/*                                                                          */
/****************************************************************************/

clipwindow(onoff)
   int onoff;
   {
   char *funcname;
   int errnum;

   funcname = "clipwindow";

   if(osexists)
      {
      errnum = 8;
      errhand(funcname,errnum);
      return(1);
      }

   wndwclip = onoff;

   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: coordinatesystemtype                                       */
/*                                                                          */
/*     PURPOSE: THE PARAMETER 'type' SELECTS WHETHER THE WORLD COORDINATE   */
/*              SYSTEM IS RIGHT-HANDED OR LEFT-HANDED.                      */
/*                                                                          */
/****************************************************************************/

coordinatesystemtype(type)
   int type;
   {
   char *funcname;
   int errnum;

   funcname = "coordinatesystemtype";

   if(corsyset)   /*** FUNCTION ALREADY BEEN CALLED? ***/
      {
      errnum = 11;
      errhand(funcname,errnum);
      return(1);
      }
   if(vfinvokd || prevseg)   /*** VIEW FUNCTION ALREADY CALLED OR SEGMENT PREVIOUSLY CREATED ? ***/
      {
      errnum = 19;
      errhand(funcname,errnum);
      return(2);
      }
   if(type != LEFT && type != RIGHT)   /*** PROPER TYPE ? ***/
      {
      errnum = 20;
      errhand(funcname,errnum);
      return(3);
      }

   coordsys = type;

   corsyset = TRUE;
   return(0);
   }

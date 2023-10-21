/* Liang-Barsky Polygon Clipping [CACM, Vol 26 (Nov, 1983)]
 *    (with corrections from [CACM (April, 1984)])
 */

/*   Note that this assumes the last point == the first in the
 *    polygon representation the code in the article adds the
 *    last point in at the start of the algorithm (probably
 *    a better approach).
 */

	/* The following include brings in some macro definitions
	 * for accessing information about a polygon:
	 *
	 *   NPNTS()  is the number of vertices in the polygon
	 *            (counting first point twice)
         *
         *   GETPNT() accesses the specified point out of the
	 *    	      polygon.
	 *
	 *   XCOORD/YCOORD() access the x(y)-coordinate in a point
	 *
	 * Macros were used because this code must ultimately tie
	 *  into my wife's animation package, and I don't know
	 *  how she is going to represent polygons yet.
	 */

# include "pln.h"

# define INFINITY	(1.0e+30)

	/* add a new vertex into the output polygon */

# define add(x,y) {\
                   XCOORD(GETPNT(npoly,npnt)) = x;\
                   YCOORD(GETPNT(npoly,npnt)) = y;\
                   ++npnt;\
                  }

	/* window bounds (xleft,ybottom), (xright,ytop) */
extern float wx1,wy1,  wx2,wy2;

	/* The Liang-Barsky Polygon Clipping Algorithm */
fclip(opoly,npoly)
   PLN *opoly, *npoly;

   {
   register int i, npnt;
   float deltax, deltay, xin,xout,  yin,yout;
   float tinx,tiny,  toutx,touty,  tin1, tin2,  tout1,tout2;
   float x1,y1, x2,y2;
   
   npnt = 0;

   for (i = 0; i < NPNTS(opoly)-1; ++i) {

      x1 = XCOORD(GETPNT(opoly,i));
      y1 = YCOORD(GETPNT(opoly,i));
      x2 = XCOORD(GETPNT(opoly,i+1));
      y2 = YCOORD(GETPNT(opoly,i+1));

      deltax = x2-x1;
      deltay = y2-y1;

      if (deltax > 0 || (deltax == 0 && x1>wx2)) { /*  points to right */
         xin = wx1;
         xout = wx2;
         }
      else {
         xin = wx2;
         xout = wx1;
         }
      if (deltay > 0 || (deltay == 0 && y1>wy2)) { /*  points up */
         yin = wy1;
         yout = wy2;
         }
      else {
         yin = wy2;
         yout = wy1;
         }

      tinx = (deltax != 0) ? ((xin - x1)/deltax) : -INFINITY ;
      tiny = (deltay != 0) ? ((yin - y1)/deltay) : -INFINITY ;
   
      if (tinx < tiny) {	/* hits x first */
         tin1 = tinx;
         tin2 = tiny;
         }
      else			/* hits y first */
         {
         tin1 = tiny;
         tin2 = tinx;
         }

      if (1 >= tin1) {
         if (0 < tin1) {
            add(xin,yin);
            }
         if (1 >= tin2) {
	    if (deltax != 0) toutx = (xout-x1)/deltax;
	    else {
               if (wx1 <= x1 && x1 <= wx2) toutx = INFINITY;
               else                        toutx = -INFINITY;
	       }
	    if (deltay != 0) touty = (yout-y1)/deltay;
	    else {
               if (wy1 <= y1 && y1 <= wy2) touty = INFINITY;
               else                        touty = -INFINITY;
	       }

	    tout1 = (toutx < touty) ? toutx : touty ;
   
            if (0 < tin2 || 0 < tout1) {
               if (tin2 <= tout1) {
                  if (0 < tin2) {
                     if (tinx > tiny) {
                        add (xin, y1+tinx*deltay);
                        }
                     else {
                        add (x1 + tiny*deltax, yin);
                        }
                     }
                  if (1 > tout1) {
                     if (toutx < touty) {
                        add (xout, y1+toutx*deltay);
                        }
                     else {
                        add (x1 + touty*deltax, yout);
                        }
                     }
                  else {
                     add (x2,y2);
                     }
                  }
               else {
                  if (tinx > tiny) {
                     add (xin, yout);
                     }
                  else {
                     add (xout, yin);
                     }
                  }
               }
            }
         }
      }

   if (npnt) {
      add(XCOORD(GETPNT(npoly,0)),YCOORD(GETPNT(npoly,0)));
      }
   NPNTS(npoly) = npnt;
   }

#___________ cut mark

/* Modified Liang-Barsky Polygon Clipping [CACM, Vol 26 (Nov, 1983)] */

/* see the comments at the start of the unmodified version for more
 * details.
 */

# include "pln.h"

# define INFINITY	(1.0e+30)
# define NEARZERO	(1.0e-30)	/* 1/INFINITY */

# define add(x,y) {\
                   XCOORD(GETPNT(npoly,npnt)) = x;\
                   YCOORD(GETPNT(npoly,npnt)) = y;\
                   ++npnt;\
                  }

extern float wx1,wy1,  wx2,wy2;	/* window boundaries */

fclip(opoly,npoly)
   PLN *opoly, *npoly;

   {
   register int i, npnt;
   float deltax, deltay, xin,xout,  yin,yout;
   float tinx,tiny,  toutx,touty,  tin1, tin2,  tout1,tout2;
   float x1,y1, x2,y2;
   
   npnt = 0;

   for (i = 0; i < NPNTS(opoly)-1; ++i) {

      x1 = XCOORD(GETPNT(opoly,i));
      y1 = YCOORD(GETPNT(opoly,i));
      x2 = XCOORD(GETPNT(opoly,i+1));
      y2 = YCOORD(GETPNT(opoly,i+1));

      deltax = x2-x1;
      if (deltax == 0) { /* bump off of the vertical */
         deltax = (x1 > wx1) ? -NEARZERO : NEARZERO ;
         }
      deltay = y2-y1;
      if (deltay == 0) { /* bump off of the horizontal */
         deltay = (y1 > wy1) ? -NEARZERO : NEARZERO ;
         }

      if (deltax > 0) {		/*  points to right */
         xin = wx1;
         xout = wx2;
         }
      else {
         xin = wx2;
         xout = wx1;
         }
      if (deltay > 0) {		/*  points up */
         yin = wy1;
         yout = wy2;
         }
      else {
         yin = wy2;
         yout = wy1;
         }

      tinx = (xin - x1)/deltax;
      tiny = (yin - y1)/deltay;
   
      if (tinx < tiny) {	/* hits x first */
         tin1 = tinx;
         tin2 = tiny;
         }
      else			/* hits y first */
         {
         tin1 = tiny;
         tin2 = tinx;
         }

      if (1 >= tin1) {
         if (0 < tin1) {
            add(xin,yin);
            }
         if (1 >= tin2) {
            toutx = (xout - x1)/deltax;
            touty = (yout - y1)/deltay;

            tout1 = (toutx < touty) ? toutx : touty ;
   
            if (0 < tin2 || 0 < tout1) {
               if (tin2 <= tout1) {
                  if (0 < tin2) {
                     if (tinx > tiny) {
                        add (xin, y1+tinx*deltay);
                        }
                     else {
                        add (x1 + tiny*deltax, yin);
                        }
                     }
                  if (1 > tout1) {
                     if (toutx < touty) {
                        add (xout, y1+toutx*deltay);
                        }
                     else {
                        add (x1 + touty*deltax, yout);
                        }
                     }
                  else {
                     add (x2,y2);
                     }
                  }
               else {
                  if (tinx > tiny) {
                     add (xin, yout);
                     }
                  else {
                     add (xout, yin);
                     }
                  }
               }
            }
         }
      }

   if (npnt) {
      add(XCOORD(GETPNT(npoly,0)),YCOORD(GETPNT(npoly,0)));
      }
   NPNTS(npoly) = npnt;
   }

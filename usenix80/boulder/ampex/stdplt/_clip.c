#define	XMIN	warr[0]		/* window limits */
#define XMAX	warr[1]
#define	YMIN	warr[2]
#define	YMAX	warr[3]
#define code(x,y) (((x)<XMIN?1:((x)>XMAX?2:0))|((y)<YMIN?4:((y)>YMAX?8:0)))
#define solve(p0,p1,q1,p2,q2) (((q2)-(q1))/((p2)-(p1))*((p0)-(p1))+(q1))

int
_clip(x1,y1,x2,y2,warr)

float *x1,*y1,*x2,*y2;
float warr[];		/* array containing window dimensions */

/* Windows a line segment.  Returns 0 if segment is already inside window, 
 * -1 if totally outside window, sets bit 0 if first endpoint changed,
 *  sets bit 1 if second endpoint changed.
 */
  {
	register int c1, c2;
	int retcd;			/* return code */

	c1=code(*x1,*y1);
	c2=code(*x2,*y2);
	retcd = (c1 ? 1 : 0) | (c2 ? 2 : 0);

	if(!(c1 || c2)) return(0); 	/* line already completely in bounds */
	while (c1 | c2)
	  { 	if(c1 & c2) return(-1);	/* line completely out of bounds */

		if (c1)
		  {	if (c1 & 1) 
			  {	*y1 = solve(XMIN,*x1,*y1,*x2,*y2);
				*x1 = XMIN;
			  }
			else if (c1&2) 
			  {	*y1 = solve(XMAX,*x1,*y1,*x2,*y2);
				*x1 = XMAX;
			  }
			else if (c1&4) 
			  {	*x1 = solve(YMIN,*y1,*x1,*y2,*x2);
				*y1 = YMIN;
			  }
			else if (c1&8) 
			  {	*x1 = solve(YMAX,*y1,*x1,*y2,*x2);
				*y1 = YMAX;
			  }
			c1 = code(*x1,*y1);
			if(c1 & c2) return(-1);	/* line completely out of bounds */
		  }
		if (c2)
		  {	if (c2&1) 
			  {	*y2 = solve(XMIN,*x1,*y1,*x2,*y2);
				*x2 = XMIN;
			  }
			else if (c2&2) 
			  {	*y2 = solve(XMAX,*x1,*y1,*x2,*y2);
				*x2 = XMAX;
			  }
			else if (c2&4) 
			  {	*x2 = solve(YMIN,*y1,*x1,*y2,*x2);
				*y2 = YMIN;
			  }
			else if (c2&8) 
			  {	*x2 = solve(YMAX,*y1,*x1,*y2,*x2);
				*y2 = YMAX;
			  }
			c2 = code(*x2,*y2);
		  }
	  }
	return(retcd);		/* line clipped to fit bounds */
   }

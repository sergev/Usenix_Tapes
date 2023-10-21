/*
 *	Copyright (c) 1987 by Richard Harter.  Permission to use or
 *	copy is granted, including commercial distributions, provided
 *	that this notice is included.
 *
 *	Synopsis:
 *
 *	This is a secant root finder.  It searches for a root of a
 *	real function in the neighbourhood of two initial guesses of
 *	the root.
 *
 *	Author: Richard Harter
 *	Date:   March, 1987
 *
 *	Entry points:
 *
 *	int firstguess(x,y)	presents first guess to algorithm
 *		double x -- first guess at root
 *		double y -- value of function at first guess
 *
 *	double nextguess(x,y) returns next guess
 *		double x -- current guess at root
 *		double y -- value of function at current guess
 *		value returned is next guess to process
 *
 *	int rootstatus() Status of iteration
 *			 = -1, root not bracketed
 *			 =  0, root has been found
 *			 =  1, root bracketed, still converging
 *
 *	double rooterror() estimated error of current iterate
 *		returns -1. if in hunt mode
 *		returns  0. if root is exact
 *		returns  bracket interval otherwise
 *
 *
 *	Usage:
 *
 *	Initialize the algorithm by calling firstguess with an
 *	approximation to the desired root, and the value of the
 *	function at that point.  Call nextguess with the second
 *	approximation and the value of the function at that point.
 *	Nextguess will return the next iterate.  Evaluate the
 *	function at the iterate and repeat in a loop.  Terminate
 *	the loop if (a) function rootstatus returns a 0 or (b)
 *	function rootstatus returns a -1 and and your iteration
 *	count limit has been exceeded.  Call function rooterror
 *	to get error interval containing the root.
 *
 *	Note that the best root estimate is the value returned
 *	by nextguess after rootstatus indicates convergence.
 *	See the example.
 *
 *	Abstract:
 *
 *	The traditional secant iteration formula is
 *
 *		x[n+1] = x[n] - y[n]*(x[n]-x[n-1])/(y[n]-y[n-1])
 *
 *	with a convergence power of 1.6.  The method used here is
 *	to bracket the root and use the end point with the smaller
 *	absolute function value rather than the previous iterate.
 *	This reduces to the secant method when the iterates are
 *	sufficiently close to the root and is superior when the
 *	two methods differ. (Note: this is not the same as interpolation
 *	with the end point with opposite sign which yields geometric
 *	convergence and can be tediously slow.)
 *
 *	It is not assumed that the first two iterates bracket the
 *	desired root -- if they do not a hunt mode is initiated.
 *	The hunt mode is primitive; linear interpolation is used
 *	for the last two iterates with the delta being overestimated
 *	by 50% to attempt to force bracketing of the root.
 *
 *	The code contains defensive measures against floating point
 *	underflow in the delta calculations and against out of range
 *	extrapolations.
 *
 *
 *	Example:
 *
 *	main(){
 *	  double x,y,nextguess(),rooterror(),func();
 *	  int i,rootstatus();
 *	  x = 0.;
 *	  y = func(x);
 *	  firstguess(x,y);
 *	  x = 1.;
 *	  for (i=0;i<25 && rootstatus();i++) {
 *	    y = func(x);
 *	    printf("%20.16f %20.16f\n",x,y);
 *	    x = nextguess(x,y);
 *	    }
 *	  printf("root = %20.16f, interval = %20.16f\n",x,rooterror());
 *	  }
 *	double func(x) double x; {return x*x -2.;}
 */

#define BEGIN {
#define END }
#define true (1)
#define false (0)

/*	Bracket values when root is bracketed				*/

static double		 xl;		/* Low bracket value for x	*/
static double		 yl;		/* Func value at xlo		*/
static double		 xh;		/* High bracket value for x	*/
static double		 yh;		/* Func value at xhi		*/

/*	Previous iterates						*/

static double		 x1;		/* Previous x value		*/
static double		 y1;		/* Previous y value		*/

/*	Logic control							*/

static int		 mode;		/* Processing case		*/
					/*  0: Normal second guess	*/
					/*  1: Hunt mode		*/
					/* -1: Normal bracket mode	*/
static int		 found;		/* True is root already found	*/
static int		 increasing;	/* True is increasing func	*/
static int		 exact;		/* Root is exact		*/
static double		 xf;		/* Value of found root		*/

firstguess(x,y)				/* Processes first guess	*/
  double		 x;		/* Argument value for 1st guess */
  double		 y;		/* Function value for 1st guess	*/
BEGIN

  x1	= x;				/* Set previous arg value	*/
  y1	= y;				/* Set previous func value	*/
  mode  = 0;				/* Initialize mode		*/
  if (y==0.0) BEGIN			/* First guess is good		*/
    xf	= x;				/* Set found arg		*/
    found = true;			/* Set flag			*/
    exact = true;			/* Set exact root flag		*/
    END
  else BEGIN				/* Not lucky on first guess	*/
    found = false;			/* Not found, say so		*/
    exact = false;			/* Not an exact root		*/
    END
  END

double nextguess(x,y)			/* Yields nextguess from current*/
  double		 x;		/* Current arg value		*/
  double		 y;		/* current func value		*/
BEGIN

  double		 xret;		/* Next arg value to try	*/
  double		 delta;		/* Change in iterate		*/
  double		 tdelta;	/* L.B. for delta in y= case	*/
  double		 setfinal();	/* Sets final x on convergence	*/
  int			 uselow;	/* Flag signifying smallest y	*/

  if (found) return xf;			/* Root already found		*/
  if (y==0.0) BEGIN			/* Root has just been found	*/
    xf = x;				/* Set found value		*/
    found = true;			/* Set flag			*/
    exact = true;			/* Set exact flag		*/
    return xf;				/* Return root			*/
    END
  if (!mode) BEGIN			/* This is 2nd guess, startup	*/
    if (y<0 && y1>0)      mode = -1;	/* Bracket established		*/
    else if (y>0 && y1<0) mode = -1;	/* Likewise			*/
    else                  mode =  1;	/* Must be hunt mode		*/
    if (mode<0) BEGIN			/* Bracketed, load bracket	*/
      if (x<x1) BEGIN			/* Order is {x,x1}		*/
        xl = x; xh = x1;		/* Load x values		*/
        yl = y; yh = y1;		/* Load y values		*/
        END
      else BEGIN			/* Order is {x1,x}		*/
        xl = x1; xh = x;		/* Load x values		*/
        yl = y1; yh = y;		/* Load y values		*/
        END
      if (yl<yh) increasing = true;	/* Function is increasing	*/
      else       increasing = false;	/* Function is decreasing	*/
      xret = xl - yl*(xh-xl)/(yh-yl);	/* Straight interpolation 	*/
      return xret;			/* Return it			*/
      END
    END

  if (mode<0) BEGIN			/* Normal bracket mode		*/
    if (x<xl || x>xh) BEGIN		/* Arg is out of range		*/
      xret = (xl + xh)/2.0;		/* Get bracket midrange		*/
      return xret;			/* Return it			*/
      END
    if (increasing) BEGIN		/* Cose to find smallest y	*/
      if (yh > -yl) uselow = true;	/* Smallest is |yl|		*/
      else          uselow = false;	/* Smallest is |yh|		*/
      END
    else BEGIN				/* Function is decreasing	*/
      if (yl < -yh) uselow = true;	/* Smallest is |yl|		*/
      else          uselow = false;	/* Smallest is |yh|		*/
      END
    if (uselow) BEGIN			/* Use low end x,y		*/
      if (y==yl) BEGIN			/* y=yl is special case		*/
        delta = 2.0*(x-xl);		/* Explode small end delta	*/
        tdelta= .1*(xh-xl);		/* Lower bound for delta size	*/
        if (delta<tdelta) delta=tdelta;	/* Use L.B. if needful		*/
        END
      else delta = -y*(x-xl)/(y-yl);	/* Normal secant delta		*/
      xret = x + delta;			/* Compute expected next	*/
      if (delta>0.0) BEGIN		/* Increasing delta		*/
        if (xret>xh) BEGIN		/* Next x out of range		*/
          delta = (xh-x)/2.0;		/* Split the range		*/
          xret  = x + delta;		/* Recompute next x		*/
          END
        while (xret<=x) BEGIN		/* Loop to catch underflow	*/
          delta *= 2.0;			/* Double delta to try again	*/
          xret = x + delta;		/* Recompute next x		*/
          END
        if (xret>=xh) BEGIN		/* Can't make good delta	*/
          found = true;			/* Must be final iterate	*/
          xf = setfinal(x,y);		/* Pick best final iterate	*/
          xret = xf;			/* Return found root		*/
          END
        END
      else if (delta<0.0) BEGIN		/* Decreasing delta		*/
        if (xret<xl) BEGIN		/* Next x out of range		*/
          delta = (xl-x)/2.0;		/* Split the range		*/
          xret  = x + delta;		/* Recompute next x		*/
          END
        while (xret>=x) BEGIN		/* Loop to catch underflow	*/
          delta *= 2.0;			/* Double delta to try again	*/
          xret = x + delta;		/* Recompute next x		*/
          END
        if (xret<=xl) BEGIN		/* Can't make good delta	*/
          found = true;			/* Must be final iterate	*/
          xf = setfinal(x,y);		/* Pick best final iterate	*/
          xret = xf;			/* Return found root		*/
          END
        END
      else BEGIN			/* Delta exactly 0.0, done	*/
        xf = x;				/* Accept root			*/
        found = true;			/* Record as found		*/
        xret = x;			/* Return the root		*/
        END
      END
    else BEGIN				/* Use high end x,y		*/
      if (y==yh) BEGIN			/* y=yh, special case		*/
        delta  = 2.*(x-xh);		/* Explode small delta		*/
        tdelta = .1*(xl-x);		/* L.B. for delta		*/
        if (delta>tdelta) delta=tdelta;	/* Use LB if needful		*/
        END
      else delta = -y*(xh-x)/(yh-y);	/* Normal secant delta		*/
      xret = x + delta;			/* Compute expected next	*/
      if (delta>0.0) BEGIN		/* Increasing delta		*/
        if (xret>xh) BEGIN		/* Next x out of range		*/
          delta = (xh-x)/2.0;		/* Split the range		*/
          xret  = x + delta;		/* Recompute next x		*/
          END
        while (xret<=x) BEGIN		/* Loop to catch underflow	*/
          delta *= 2.0;			/* Double delta to try again	*/
          xret = x + delta;		/* Recompute next x		*/
          END
        if (xret>=xh) BEGIN		/* Can't make good delta	*/
          found = true;			/* Must be final iterate	*/
          xf = setfinal(x,y);		/* Pick best final iterate	*/
          xret = x;			/* Return found root		*/
          END
        END
      else if (delta<0.0) BEGIN		/* Decreasing delta		*/
        if (xret<xl) BEGIN		/* Next x out of range		*/
          delta = (xl-x)/2.0;		/* Split the range		*/
          xret  = x + delta;		/* Recompute next x		*/
          END
        while (xret>=x) BEGIN		/* Loop to catch underflow	*/
          delta *= 2.0;			/* Double delta to try again	*/
          xret = x + delta;		/* Recompute next x		*/
          END
        if (xret<=xl) BEGIN		/* Can't make good delta	*/
          found = true;			/* Must be final iterate	*/
          xf = setfinal(x,y);		/* Pick best final iterate	*/
          xret = x;			/* Return found root		*/
          END
        END
      else BEGIN			/* Delta exactly 0.0, done	*/
        xf = x;				/* Accept root			*/
        found = true;			/* Record as found		*/
        xret = x;			/* Return the root		*/
        END
      END
    if ((increasing && y>0.0) || (!increasing && y<0.0)) BEGIN
      xh = x;				/* Move high end down		*/
      yh = y;				/* Same for func value		*/
      END
    else BEGIN				/* Moving low end forward	*/
      xl = x;				/* Moving x			*/
      yl = y;				/* Moving y			*/
      END
    return xret;			/* Return accepted root		*/
    END

  if (mode>0) BEGIN			/* Hunt mode			*/
    if ((y<0. && y1>0.) || (y>0. && y1<0.)) BEGIN
      mode = 0;				/* Reset as though 2nd guess	*/
      return nextguess(x,y);		/* Reuse switch to bracket code	*/
      END
    xret = x - 1.5*y*(x-x1)/(y-y1);	/* Overestimate delta		*/
    x1 = x;				/* Push x			*/
    y1 = y;				/* Push y			*/
    END
  return xret;				/* Return next guess		*/
  END

double setfinal(x,y)			/* Picks best final iterate	*/
  double		 x;		/* Last x given us		*/
  double		 y;		/* Last y given us		*/
BEGIN

  if (increasing) BEGIN			/* Function is increasing	*/
    if (y>0.) BEGIN			/* Choose between xl and x	*/
      if (y > -yl) return xl;		/* |y|>|yl| use xl		*/
      else         return x;		/* |yl|>=|y| use x		*/
      END
    else BEGIN				/* Choose between x and xh	*/
      if (-y > yh) return xh;		/* |y|>|yh| use xh		*/
      else         return x;		/* |yh|>=|y| use x		*/
      END
    END
  else BEGIN				/* Function is decreasing	*/
    if (y>0.) BEGIN			/* Choose between x and xh	*/
      if (y > -yh) return xh;		/* |y|>|yh| use xh		*/
      else         return x;            /* |y|<=|yh| use x		*/
      END
    else BEGIN				/* Choose between x and xl	*/
      if (-y > yl) return xl;		/* |y|>|yl| use xl		*/
      else         return x;		/* |y|<=|yl| use x		*/
      END
    END
  END

int rootstatus() BEGIN			/* Returns status of iteration	*/

  if (found) return 0;			/* 0 is root is found		*/
  if (mode<1) return 1;			/* 1 is bracketed, converging	*/
  return -1;				/* Still in hunt mode		*/
  END

double rooterror() BEGIN		/* Returns root interval	*/
  if (exact)  return  0.;		/* Root is exact		*/
  if (mode>0) return -1.;		/* Still in hunt mode		*/
  return (xh-xl);			/* Return bracket size		*/
  END

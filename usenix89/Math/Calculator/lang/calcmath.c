/*****************************************************
 * Calculator RPN      calcmath.c                    *
 *  		file containing the functions part   *
 *		of the calculator		     *
 * by Jerome M. Lang   17 January 1986               *
 *****************************************************/
/*
 * 1986-02-18:add atan, acos, asin
 * 1986-02-18:fixed pi, compiler bug, doesn't accept long double constants.
 * 1986-02-18:fixed precision problem with sinh near 0.
 * 1986-02-20:precision problem of atan near .42
 * 1986-02-22:split in two files
 */
#include <obdefs.h>
#include <gemdefs.h>
#include <define.h>
#include <osbind.h>
#include "calculat.h"

#define BEEP	Cconout(7)
#define E	2.718281828
/* compiler bug here, if pi is given with one more digit of precision, 
 * it is not accepted
 */
#define Pi	3.141592653589793238
#define PIHALF	1.570796326794896619
#define PIQUART	0.785398163397448309
#define SQ2M1   0.4142136	/* sqrt(2) - 1 */
#define hypot(x,y) sqrt(x*x + y*y)

/* flags for atan */
#define NEGATIVE 0x0001	/* true if x was negative */
#define BIGGER1  0x0002 /* true if x was bigger than 1, after going positive */
#define SQ2M1T	 0x0004 /* true if x was bigger that SQ2M1T,after going 0<x<1*/

/*& extern	desel_obj(), cleartext(), push(), update1();
/*& &*/
extern	double	sin(), cos(), sqrt(), exp(), pow(), log(), atof(), pop(),
		 fabs();
extern  int	Xdesk, Ydesk, Wdesk, Hdesk;
/*
 * More mathematics
 */

double
log10( x )
double x;
{
	return( log(x)/log(10.0) );
}

double
sinh( x )
register double x;
{
	register double y;

	if(fabs(x)<0.5)
	{ 		/* via Taylor, rel. prec. should be better than 2^-24 */
		y = x*x;
		return( x*( ((y/42.0+1.0)*y/20.0+1.0)*y/6.0+1.0) );
	}
	else
	{
		y = exp(x);
		return( (y - 1.0/y) * 0.5 );
	}
}

double
asinh( x )
register double x;
{
	return( log(x+sqrt(x*x+1.0)) );
}

double
cosh( x )
register double x;
{
	register double y = exp(x);

	return( (y + 1.0/y) * 0.5 );
}

double
acosh( x )
register double x;
{
	return( log(x+sqrt(x*x-1.0)) );
}

double 
tanh( x )
double x;
{
	register double y = exp(x);

	return( (y - 1.0/y) / (y + 1.0/y) );
}

double
atanh( x )
register double x;
{
	return( 0.5*log( (1.0 + x)/(1.0 - x) ) );
}

double
tan( x )
double x;
{
	return( sin(x) / cos(x) );
}

double
atan( x )
register	double x;
{
	register	unsigned int	flags = 0;
	register	double		y;
	register	double		ans;

	if( x<0.0 ) /* use identity tan(-x) = - tan(x)  for x<0 */
	{
		x = -x;
		flags |= NEGATIVE;
	}
	/* x is positive here */
	if( x>1.0 )	/* use identity atan x + atan (1/x) = pi/2
			 * for numbers > 1 */
	{
		x = 1.0 / x ;
		flags |= BIGGER1;
	}
	/* 0<=x<=1 here */
	if( x > SQ2M1 ) /* use identity atan x + atan((1-x)/(1+x)) = pi/4 
			 * for sqrt(2)-1< x <= 1 */
	{
		x = -1.0 + 2.0/(1.0+x);
		flags |= SQ2M1T;
	}
	/* 0<=x<=sqrt(2)-1 here */
	y = x*x;
	/* hope compiler is intelligent enough to compute the constants
		instead of dividing, should not count on it though */
	/* should Chebyshev it */
	/* relative accuracy should be <2^-24 */
	/* here 0 <= x <= sqrt(2) - 1 */
	ans = x*( ((((((((1/17.0-y/19.0)*y-1/15.0)*y+1/13.0)*y
		-1/11.0)*y+1/9.0)*y-1/7.0)*y+1/5.0)*y-1/3.0)*y+1.0);
	if( flags&SQ2M1T )
		ans = PIQUART - ans;
	if( flags&BIGGER1 )
		ans = PIHALF - ans;
	if( flags&NEGATIVE )
		ans = -ans;
	return(ans);
}

double
asin( x ) /* no error checking on range */
register double x;
{
	return( atan( x/sqrt( 1.0 - x*x ) ) );
}

double
acos( x ) /* no error checking on range */
register double x;
{
	return( (x<0.0 ? Pi : 0.0)+ atan( sqrt(1.0 - x*x)/x ) );
}

do_calc()
{
	OBJECT	*fo_calc;
	int	xdial, ydial, wdial, hdial;
	int	button;
	int	more = TRUE;
	register double	temp, temp2;

	rsrc_gaddr(R_TREE, CALCUL, &fo_calc);

	form_center(fo_calc, &xdial, &ydial, &wdial, &hdial);
	wind_update( BEG_UPDATE ); /*  so other windows don't mess us up */
	form_dial( FMD_START, Xdesk, Ydesk, Wdesk, Hdesk,
				xdial, ydial, wdial, hdial);
	objc_draw(fo_calc, ROOT, MAX_DEPTH,
				xdial, ydial, wdial, hdial);
				/* draw calculator on screen */
	update(fo_calc, xdial, ydial, wdial, hdial); /* display top 4 of stack*/

	while( more ){
		button = form_do( fo_calc, DISPLAY0) & 0x7fff;
		more = handle_select( fo_calc, button );
				/* what did user do? */
		update(fo_calc, xdial, ydial, wdial, hdial);
				/* Refresh display */
		desel_obj(fo_calc, button); /* deselect object */
		objc_draw(fo_calc, button, 0,
				xdial, ydial, wdial, hdial);
				/* draw deselected object */
	}

	form_dial(FMD_FINISH, Xdesk, Ydesk, Wdesk, Hdesk,
				xdial, ydial, wdial, hdial);
				/* release reserved screen */
	wind_update( END_UPDATE );
}

int
handle_select( tree, button )
register int button;
OBJECT	*tree;
{
	register double temp,temp2;
	register ret = TRUE; /* calculator still on */

	switch( button ){

		case OFF:	ret = FALSE; /* calculator shut off */
				break;

		case DISPLAY0:	/* new number in reflected by <Return> */
				push( atof (
				((TEDINFO *)tree[DISPLAY0].ob_spec)->te_ptext));
				cleartext(tree,DISPLAY0);
				break;

		case ENTER:	temp=pop();
				push(temp);
				push(temp);
				break;

		case EXCHG:	temp=pop();
				temp2=pop();
				push(temp);
				push(temp2);
				break;

		case POP:	pop(); /* ignore result */
				break;

		case SIN:	push(sin(pop()));
				break;

		case ASIN:	temp = pop();
				if(1.0>=fabs(temp))push(asin(temp));
				else {BEEP;push(0.0);}
				break;

		case COS:	push(cos(pop()));
				break;

		case ACOS:	temp=pop();
				if(1.0>=fabs(temp))push(acos(temp));
				else {BEEP;push(0.0);}
				break;

		case TAN:	push(tan(pop()));
				break;

		case ATAN:	push(atan(pop()));
				break;

		case SINH:	push(sinh(pop()));
				break;

		case ASINH:	push(asinh(pop()));
				break;

		case COSH:	push(cosh(pop()));
				break;

		case ACOSH:	push(acosh(pop()));
				break;

		case TANH:	push(tanh(pop()));
				break;

		case ATANH:	push(atanh(pop()));
				break;

		case ONEX:	temp=pop();
				if(0.0 != temp)push(1.0/temp);
				else {BEEP;push(0.0);}
				break;

		case SQR:	temp=pop();
				push(temp*temp);
				break;

		case SQRT:	temp=pop();
				if(0.0<=temp)push(sqrt(temp));
				else {BEEP;push(0.0);}
				break;

		case ETOX:	push(exp(pop()));
				break;

		case TENTOX:	push(pow(10.0,pop()));
				break;

		case YTOX:	temp=pop();
				push(pow(pop(),temp));
				break;

		case LOG:	temp=pop();
				if(0.0<=temp)push(log10(temp));
				else {BEEP;push(0.0);}
				break;

		case LN:	temp=pop();
				if(0.0<=temp)push(log(temp));
				else {BEEP;push(0.0);}
				break;

		case PLUSMIN:	push(-pop());
				break;

		case DIV:	temp=pop();
				if(0.0!=temp)push(pop()/temp);
				else {BEEP;push(0.0);}
				break;

		case TIME:	push(pop()*pop());
				break;

		case MINUS:	temp=pop();
				push(pop()-temp);
				break;

		case PLUS:	push(pop()+pop());
				break;

		case PI:	push(Pi);
				break;

		case PTOR:	temp=pop(); /* r */
				temp2=pop(); /* theta */
				push( temp * sin(temp2) ); /* y */
				push( temp * cos(temp2) ); /* x */
				break;

		case RTOP:	temp=pop(); /* x */
				temp2=pop(); /* y */
				push( atan( temp2/temp ) ); /* theta */
					/* ignore temp==0 */
				push( hypot( temp , temp2 ) ); /* r */
				break;

	}
	return( ret );
}

	/* update all 4 displays */
update(tree, xdial, ydial, wdial, hdial)
register OBJECT	*tree;
int	xdial, ydial, wdial, hdial;
{

	update1(tree, DISPLAY0, 0, xdial, ydial, wdial, hdial);
	update1(tree, DISPLAY1, 1, xdial, ydial, wdial, hdial);
	update1(tree, DISPLAY2, 2, xdial, ydial, wdial, hdial);
	update1(tree, DISPLAY3, 3, xdial, ydial, wdial, hdial);
}

#if 0				/* 1 = enable debugging, 0 = disable debugging */
#	define DEBUG 1		/* enable debugging code */
#else
#	undef DEBUG		/* disable debugging code */
#endif
/*
 * compound.c
 * dennis 01 15 87	dennis@rlgvax.UUCP
 *
 * Written by:
 *	Dennis Bednar
 *	Computer Consoles Inc
 *	11490 Commerce Park Dr.
 *	Reston VA 22090
 *
 * determine money because of compound interest.
 *
 * TODO: Handle multi-shot when deposit period != compound period.
 *
 * 01 30 87: Ask for inflation rate, and besides printing the total, print
 * what the total would be worth in today's dollars, given the inflation
 * rate chosen by the user.
 *
 * One-shot means one deposit into a bank which gains because of compounded
 * interest.
 *
 * Multi-shot means that you keep making deposits at regular intervals into
 * a bank, and the bank compounds your interest. For example, if your
 * employer deposits into a savings account every pay period.
 *
 * Note: For people who don't understand compound interest, here is a simple
 * example:
 * $100 for one year at 4% would yield $104.00 after one year if compounded
 * annually (once per year).  The money *must* be deposited before the beginning
 * of a period that the bank chooses.
 * $100 for one year at 4%, compounded quarterly, would yield 1% each quarter.
 *	101.00     at end of first quarter ($100 * 1.01)
 *	102.01     at end of 2nd quarter ($101 * 1.01 = 101 + 1.01)
 *	103.0301   at end of 3rd quarter (102.01 * 1.01 = 102.01 + 1.0201)
 *	104.060401 at end of 4th quarter ($103.0301 * 1.01 = 103.0301 + 1.030301)
 * Because of floating point round-off, the number produced on a cci632
 * is really 104.060402 (when float is used), but is 104.060401 when double
 * is used.
 *
 *
 * Now I derive the formulas in general:
 *
 * FORMULA for one-shot:
 *
 * One-shot deposit:
 * t = (p) * (1 + r/(n*100))**u
 * t = total after compounded
 * p = principal (amount deposited)
 * R = r/(n * 100)
 * R = interest rate per pay period (if 4% compounded 356 days a year,
 *     R would be .04/365.
 * r = interest per year as a number (if 7% interest, r = 7)
 * n = number of times compounded per year
 * u = number of pay units ( if compounded daily for y years, would be
 *	y*365).
 *
 * However, if compounded continuously, the value after 1 years is:
 *	t = p * e^(r/100)	[see page 298, 556, of "Calculus",
 *				 by Michael Spivak]
 * and after n years is,
 *	t = p * (e ^ (r/100))^^n = p * (e ^ ((r*n)/100) )
 *
 *
 * FORMULA for multi-shot:
 *
 * Multiple-shot deposit: assumes deposit money at the same
 * rate as the compounding.  This formula below is *not* general
 * purpose enough to handle the case when the deposit period
 * is not the same as the compound period.  For example, it cannot
 * handle the case when deposits occur every two weeks, but interest
 * is compounded daily.  Nor can it handle the case when deposits
 * occur every two weeks, but interest is compounded quarterly.
 *
 * t=total
 * t0 = initial total
 * t1 = total after one unit of time
 * tu = total after u units of time
 * p = principal (amount deposited each time unit)
 * r = annual interest (7 means 7% interest per year)
 *	R = (1.0 + r/(n*100) ) multiplier for each pay period.
 * n = number of compoundings per year
 *		1 = annual
 *		2 = bi-annual
 *		4 = quarterly
 *		365 = daily
 * u = number of units
 *
 * At time t0: t = p
 * t1: t = t0*R + p
 *	 = (p*R) + p,
 *	R is (1+ r/(n*100))
 * t2: t = t1*R + p
 *	 = (p*R + p)*R + p
 *	 = p*(R**2) + p*R + p
 * t3: t = t2*R + p
 *	 = p*(R**3) + p*(R**2) +p*R + p
 * tu: t = p*R**(u-1) + ... + p*R + p
 *	 = p * [1 + R + R**2 + R**3 + R**4 + ... + R**(u) ]
 *	 = p * A							(1)
 *	where A = [1 + R + R**2 + ... + R**(u) ]
 *
 * Solving for A:
 *	A   = [1 + R + R**2 + R**3 + R**4 + ... + R**(u) ]		(2)
 * multiplying both sides of (2) by R we obtain equation (3)
 *	A*R = [  + R + R**2 + R**3 + R**4 + ... + R**(u) + R**(u+1)]	(3)
 *
 * Now subtracting (2) - (3) we obtain
 *	A - A*R = 1 - R**(u+1)						(4)
 *
 * factoring,
 *	A (1-R) = 1- R**(u+1)
 *
 * dividing both sides by (1-R), we obtain:
 *
 *	     (1- R**(u+1))
 *	A = ------------------						(5)
 *	        (1-R)
 *
 * (1 and 5 are the only two formulas that we are interested in now).
 *
 */
#include <stdio.h>

#define DOUBLE	double	/* Sys 5 R2 exp(3) *must* use a double		*/
#define FLOAT	double	/* if your machine doesn't have double you	*/
			/* may redefine FLOAT to be float.		*/
			/* but double gives more accurate answers	*/


/* fw non-int functions */
FLOAT	raise();
FLOAT	ask();

extern	DOUBLE	exp();	/* exp(3M) - exp(x) returns e to the power of x */
#ifdef DEBUG
	/* EXP() is the debug routine */
	DOUBLE EXP();	/* fw reference */
#else
#	define EXP exp	/* use exp() in libm.a */
	extern	DOUBLE EXP();
#endif


main()
{
	setbuf(stdout, (char *)NULL);	/* so automatic fflush(stdout) without newlines */
	if (yesno( "Do you want to make one deposit"))
		oneshot();
	else if (yesno( "Do you want to make multiple deposits, where interest is compounded at same period"))
		multshot();
	else
		error("sorry cannot handle multiple deposits when 'deposit period != interest period'");
	exit(0);
}

/*
 * t = (p) * (1 + r/(n*100))**u		( t = total with no inflation)
 *
 * Now if inflation were factored in, we ask what principle today,
 * if invested one shot, would be worth t dollars with inflation rate
 * "r_inf", and that is the value of p_inf when solved in (same formula
 * as above, only replace "r" with "r_inf" and replace "p" with "p_inf):
 *
 * t = (p_inf) * (1 + r_inf/(n*100))**u
 *
 * or solving for p_inf, we have
 *
 *	               t
 *    p_inf = ---------------------
 *	      (1 + r_inf/(n*100) )**u
 *
 *
 */
oneshot()
{
	FLOAT p,	/* principal */
		r,	/* annual interest rate */
		t;	/* total amount after compounded interest */
	FLOAT	r_inf;	/* inflation interest rate */
	FLOAT	p_inf;	/* principle - non-inflationary */
	int	u;	/* number of compound periods total */
	int	n;	/* number of compounds per year */


	p = ask("principal (amount deposited)");
	r = ask("annual interest rate (percent)");
	n = aski("number times compounded annually (1=yearly, 4=quarterly, 12=monthly, 26=bi-weekly, 52=weekly, 365=daily, 0=continuously)");
	switch (n) {
		case 365:
			u = aski("total number of days");
			break;
		case 52:
			u = aski("total number of weeks");
			break;
		case 26:
			u = aski("total number of bi-weeks");
			break;
		case 12:
			u = aski("total number of months");
			break;
		case 4:
			u = aski("total number of quarters");
			break;
		case 2:
			u = aski("total number of half-years");
			break;
		case 1:
			u = aski("total number of years");
			break;
		case 0:		/* compounded continuously */
			u = aski("total number of years");
			t = p * EXP( (((DOUBLE)r*(DOUBLE)u) / ((DOUBLE)100.0)) );
			printf("%f\n", t);
			return;
		default:
			u = aski("total number of compounded periods?");
			break;
		}
	t = p * raise( (double) 1.0 + (r/(double)(n*100.0)), u);
	r_inf = ask("annual inflation rate (percent)");
	printf("Total = %f\n", t);

	/* inflationary computation */
	p_inf =  t / raise( ( (double) 1.0 + (r_inf/(double)(n*100.0)) ), u);
	printf("Total at today's dollars = %f\n", p_inf);
}

/*
 * raise a real number to an non-negative integer power
 * raise(r, n) returns r to the n power, where n >= 0.
 * return -1 (error) if n < 0.
 */
FLOAT
raise(m, n)
	FLOAT	m;
	int	n;
{
	FLOAT	t;	/* total returned */
	register	int	N;

	if (n < 0)
		return -1;
	N=n;
	for (t = 1; N > 0; --N)
		t = t*m;
#ifdef DEBUG
	printf("DEBUG: raise(%f, %d) = %f\n", m, n, t);
#endif
	return t;
}

FLOAT
ask(q)
	char	*q;	/* question */
{
	FLOAT r;
	int	rtn;

	do {
		printf("%s? ", q);
		rtn = scanf("%f", &r);
		if (rtn == EOF)
			exit(0);
	} while (rtn != 1);
	return r;
}

int
aski(q)
	char	*q;	/* question */
{
	int r;
	int	rtn;

	do {
		printf("%s? ", q);
		rtn = scanf("%d", &r);
		if (rtn == EOF)
			exit(0);
	} while (rtn != 1);
	return r;
}

/*
 *	R is (1+ r/(n*100))
 * tu: t = p*R**(u-1) + ... + p*R + p
 *	 = p * A
 *	     (1- R**(u+1))
 *	A = ------------------
 *	        (1-R)
 */
multshot()
{
	FLOAT	p;	/* principal */
	FLOAT	r;	/* annual interest rate */
	int	n;	/* number of periods per year */
	int	u;	/* total number of periods */
	FLOAT	R;	/* intermediate, makes computation easier */
	FLOAT	A;	/* intermediate, makes computation easier */
	FLOAT	t;	/* total */
	FLOAT	Num;	/* numerator in A */
	FLOAT	Den;	/* denominator in A */
	FLOAT	r_inf;	/* inflation interest rate */
	FLOAT	p_inf;	/* principle - non-inflationary */

	p = ask("amount deposited each time period");
	r = ask("annual interest rate (percent)");
	n = aski( "number of deposits per year");
	if (n <= 0)
		error("must be positive integer (1,2,3,...)");
	R = (1.0 + (r/ ((FLOAT)n*100.0) ) );
#ifdef DEBUG
	printf("DEBUG: R = %f\n", R);
#endif
	u = aski( "total number of deposits");
	Num = (1.0 - raise(R, u+1));
	Den = (1.0 - R);
	A = Num / Den;
#ifdef DEBUG
	printf("DEBUG: Num = %f, Den = %f, Num/Den=%f\n", Num, Den, A);
#endif
	t = p * A - p;			/* don't include last deposit! */
	r_inf = ask("annual inflation rate (percent)");
	printf("Total = %f\n", t);

	/* inflationary computation */
	p_inf =  t / raise( ( (double) 1.0 + (r_inf/(double)(n*100.0)) ), u);
	printf("Total at today's dollars = %f\n", p_inf);
}


#ifdef DEBUG
DOUBLE
EXP(n)
	DOUBLE n;
{
	DOUBLE r;
	r = exp(n);
	printf("DEBUG: exp(%f) = %f\n", n, r);
	return r;
}
#endif

/*
 * ask question until yes or no is answered,
 * 	return 1 iff yes,
 *	return 0 iff no.
 */
yesno( msg )
	char	*msg;
{
	char	answer[100];
	int	rtn;

	while (1)
		{
		printf("%s ? ", msg);
		rtn = scanf("%s", answer);

		if (rtn == 0)
			continue;
		else if (rtn == EOF)
			exit(10);

		if (answer[0] == 'y' || answer[0] == 'Y')
			return 1;
		else if (answer[0] == 'n' || answer[0] == 'N')
			return 0;
		}
}


error( msg )
	char	*msg;
{
	fprintf(stderr, "compound: error: %s, exitting\n", msg);
	exit (1);
}

/*
 * I derived the formula, when multiple contributions and time period
 * of interest is less than time period of deposit.
 * But I didn't finish writing the code to handle it.
 *
 * multiple compounding in general
 *
 * p = principal
 * tc = time period of contribution
 * ti = time period of interest
 *
 * if ti <= tc (eg interest compounded daily, deposit bi-weekly)
 * time t0: p
 * time t1: p * X + p,
 *	where X = (1 + r/(ni*100) )^(ti/tc)
 *	where r = annual interest rate,
 *	where ni = number of time periods ti per year
 *	where ti/tc is the number of times interest compounded between contributions
 * time t2: t2 = t1 * X + p
 *		= (p * X + p) * X + p
 *		= p*X^2 + p*X + p
 * time t3: t3 = t2 * X + p
 *		= (p*X^2 + p*X + p) * X + p
 *		= p*X^3 + p*X^2 + p*X + p
 * ...
 * time tn: tn = p*X^n + p*X^(n-1) + ... + p*X + p
 *		= p * (X^n + X^(n-1) + ... + 1)
 *		= p * A, where A represents (X^n + ... + 1).

 * Solving for A:
 *	A = X^n + X^(n-1) + ... + 1
 *	A*X = X^(n+1) + ....... + X

 * A(X-1) = X^(n+1) - 1.
 *
 * A = [X^(n+1) - 1] / (X - 1)
 */
#if 0
ilessc()
{
	FLOAT	p;	/* principal */
	FLOAT	r;	/* annual interest rate */
	int	ni;	/* number of interest periods per year */
	int	cperi;	/* number of compound periods per interest period */
	int	X;	/* easier */
	int	A;	/* easier */

	p = ask(" amount deposited periodically" );
	r = ask( "annual interest rate (percent)" );
	ni = aski( "number of interest units per year" );
	cperi = aski( "number of compound periods per interest period" );
	/* unfinished !!! */
#endif 0

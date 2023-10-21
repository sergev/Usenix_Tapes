
/*
 * frange.c 1.1 of 4/6/87 by Sin-Yaw Wang, Sun MicroSystems.
 * @(#)frange.c	1.1
 */

/*
 * This is a handy utility to get you the exponential and the matissa
 * fields of a 4 bytes floating point variable in IEEE format.  It knows
 * what is byte swapping, but assumes float uses the same swaapping scheme
 * as long.  It has been tested on Sun and IBM PC/AT (Xenix).  The Vax
 * version needs to be done with conditional compilation.  Please try it
 * on your local machine and tell me if it works.  If you need to modify
 * it, please mail me your changes.
 * 
 *		Sin-Yaw Wang, sinyaw@sun
 *
 * Compilation instructions:
 * 	For Sun and PC/AT
 *	cc -O -o frange -DIEEE -DTEST filename.c
 * 	For Vax
 *	cc -O -o frange -DVAX -DTEST filename.c
 *
 * 	Don't define TEST if you don't need a "main()"
 */
#include <stdio.h>

#define DONTKNOW 4
#define REGULAR 3
#define SWAP	0

int             byteorder;

#ifdef IEEE

/*	macro to compute byte sequence at run time */
#define FIRSTBYTE	(byteorder ^ 3)
#define SECONDBYTE	(byteorder ^ 2)
#define THIRDBYTE	(byteorder ^ 1)
#define FOURTHBYTE	(byteorder & 3)
#define FBIAS		127	       /* IEEE standard */

#endif

#ifdef VAX			       /* what a funny byte sequence */

/*
 * 	I should come up with a run time switching scheme instead of doing this
 *	at compile time.
 */
#define FIRSTBYTE	(byteorder ^ 1)
#define SECONDBYTE	(byteorder & 3)
#define THIRDBYTE	(byteorder ^ 3)
#define FOURTHBYTE	(byteorder ^ 2)
#define FBIAS		129	       /* this is strange */

#endif

#ifdef TEST

#define VALUE		0.0
#define ENDV		10.0
#define INC		(1.0/6.0)

main()
{
	float           f;
	int             exp;
	float           mat;

	for (f = VALUE; f < ENDV; f += INC)
		if (frange(f, &exp, &mat) >= 0)
			printf("%15.10f = %15.13f * 2 exp %3d\n", f, mat, exp);
		else
		{
			printf("unknown floating format\n");
			return;
		}
}

#endif

int
frange(value, exp, mat)
	double          value;	       /* compiler convert all float to
				        * double */
	int            *exp;
	float          *mat;	       /* but not float pointers */
{
	float           f;	       /* 4 bytes */
	float           matissa();
	int             e;
	float           m;

	byteorder = order();	       /* should be called at initialization */
	if (byteorder == DONTKNOW)
		return -1;
	f = (float) value;	       /* make sure it is a 4 byte value */
	e = exponent((char *) &f);     /* is it portable?  I don't know */
	m = matissa((char *) &f);
	if (e)			       /* should check for both zero and all
				        * ones */
	{
		e -= FBIAS;
		m += 1.0;	       /* de-normalized if e is zero */
	}
	*exp = e;
	*mat = m;
	return 0;
}

int
exponent(f)
	char           *f;
{
	unsigned int    e1,
	                e2;
	int             e;

	/*
	 * the lowest 7 bits of the first byte and the highest bit of the
	 * second byte make up the exponetial part. But which one is the
	 * "first byte"... 
	 *
	 * another idea  e = (((short *) f)[0] & 0x7f80) >> 7; 
	 */
	e1 = f[FIRSTBYTE] & 0x7f;
	e2 = f[SECONDBYTE] & 0x80;
	e = (e1 << 1) | (e2 >> 7);
	return e;		       /* exponential value (based 2) */
}

float
matissa(f)
	char           *f;
{
	register int    loop = 24;     /* loop counter */
	unsigned long   m1,
	                m2;	       /* 32 bits */
	long            m;	       /* matissa bit pattern */
	float           mat;	       /* matisaa value */

	m1 = f[SECONDBYTE] & 0x7f;     /* lowset 7 bits of the second byte */
	m1 <<= 16;
	m2 = f[THIRDBYTE] & 0xff;      /* every bits of the third byte */
	m2 = m2 << 8 | (f[FOURTHBYTE] & 0xff);	/* and the fourth byte */
	m = m1 | m2;		       /* the bit pattern of the matissa */
	mat = 0.0;
	if (m)
		while (--loop)
		{
			mat /= 2.0;
			mat += (float) (m & 1) / 2.0;
			m >>= 1;
		}
	return mat;
}

static int
order()
/*
 * kind of naive to assume 4 byte float has the same seqence of 4 byte
 * integer.  Fortuately, this is true for most of the machines.
 * Unfortunately it is false for VAX, a popular computer.
 */
{
	static long     swap_test;
	static char    *i;

	swap_test = 0x0102;	       /* assign first 2 bytes */
	swap_test <<= 16;	       /* shift over 16 bits  */
	swap_test |= 0x0408;	       /* assign last 2 bytes */

	i = (char *) &swap_test;

	if ((i[0] & 0x08) &&
	    (i[1] & 0x04) &&
	    (i[2] & 0x02) &&
	    (i[3] & 0x01))
		return SWAP;
	else if ((i[0] & 0x01) &&
		 (i[1] & 0x02) &&
		 (i[2] & 0x04) &&
		 (i[3] & 0x08))
		return REGULAR;
	else
		return DONTKNOW;
}

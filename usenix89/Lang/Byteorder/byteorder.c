/* #define STAND 1 */
/*
 * byteorder.c
 * June 18, 1986
 * seismo!rlgvax!dennis		dennis@rlgvax.UUCP
 * Compile with -DSTAND for standalone program that should run,
 * compile without -DSTAND for normal library package.
 *
 * Public domain byte order routines.
 * You use this package to store binary shorts and ints
 * such that when written to the disk, will be in a network
 * independent order.
 * Tested on CCI Power6/32.
 * Tested on VAX.
 *
 * The neat think about this code is that it doesn't use
 * any #ifdef tricks, rather it figures out itself what
 * the byte ordering should be on the first call to the
 * package.
 *
 * The order chosen was such that if you step thru the
 * binary number using a char * pointer, you will go from
 * MSB to LSB.
 *
 * There are two good applications of this package.
 *	- You are trying to create binary disk files that
 *	  are guaranteed to be able to be read on another
 *	  machine (public domain cpio's, etc.).
 *	  To do this you should do
 *		long	result = htonl(l);
 *	  *just* before you write to the disk, and you should
 *	  call
 *		long	result = ntohl(l);
 *	  *just* after you read from the disk.
 *
 *	- and writing protocol programs which write to networks,
 *	  applications which write binary data over a network.
 *
 * Presently ntohl() and htonl() are internally really the same thing,
 * but these interfaces were modelled on the 4.2BSD, and that's the
 * way they did it, so don't complain.
 *
 * Longs should be 4 bytes, and shorts should be 2 bytes.
 * ARE THERE ANY MACHINES ON WHICH THIS IS NOT TRUE???
 *
 * We don't use int's because they are 2 bytes on some machines,
 * and 4 bytes on other machines.
 *
 */

/* forward refs */
long	ntohl(),
	htonl();
short	ntohs(),
	htons();

static	int	called = 0;	/* none of the routines called yet */

/*
 * there are 4 possiblities
 *						MSB   LSB
 *	Order on the disk			3 2 1 0
 *
 *	Order in memory if you stepped a char * ptr thru the long
 *	- noswap:				3 2 1 0		m68k
 *	- byteswap: (swap bytes in short)	2 3 0 1		pdp11
 *	- halfswap: (swap shorts in long)	1 0 3 2		?
 *	- bothswap: (swapbyte && halfswap)	0 1 2 3		vax
 *
 * Init2() only checks the first byte to know which of these 4 cases
 * we are using.
 */

/* boolean flags, see the above comment to understand what they mean */
static char
	noswap,
	byteswap,
	halfswap,
	bothswap;

/*
 * just to play it safe, init passes the argument on the stack so
 * that there are no tricky problems caused by differences in byte
 * order for binaries on the stack vs. binaries in static variables.
 * Thus the byte-ordering on the stack for init() is the same as
 * the byte-ordering on the stack for ntohl(), ntonl().
 * Am I being too cautious???
 */
static
init()
{
	init2( (long) 0x03020100 );
}



/*
 * basically intialize the 4 boolean flags, so that the conversion
 * routines know what to do.
 */
static
init2(l)
	long	l;
{
	char	*cp;

	if (sizeof(long) != 4)
		{
		write(1, "byteorder: sizeof(long) != 4\n", 29);
		exit(1);
		}

	if (sizeof(short) != 2)
		{
		write(1, "byteorder: sizeof(short) != 2\n", 30);
		exit(1);
		}

	cp = (char *) &l;

	switch (*cp) {
	case 03:
		++noswap;
		break;
	case 02:
		++byteswap;
		break;
	case 01:
		++halfswap;
		break;
	case 00:
		++bothswap;
		break;
	default:
		write(1, "byteorder: Unknown machine\n", 27);
		exit(1);
	}
}


/*
 * network to host long
 * call this *just* before you write to the disk or the network.
 */
long
ntohl(l)
	long	l;
{
	return(htonl(l));
}

/*
 * host to network long
 * call this *just* after you read from the disk or the network.
 */
long
htonl(l)
	long	l;
{
	register	char	*sp,	/* source pointer */
				*dp;	/* dest pointer in r */
	long	r;			/* result - cannot be register */

	if (!called)
		init();

	sp = (char *) &l;
	dp = (char *) &r;

	if (noswap)
		return l;

	if (byteswap)		/* bytes swapped within shorts */
		{
		*dp++ = sp[1];
		*dp++ = sp[0];
		*dp++ = sp[3];
		*dp++ = sp[2];
		return r;
		}

	if (halfswap)		/* swap halfwords (shorts) within long */
		{
		*dp++ = sp[2];
		*dp++ = sp[3];
		*dp++ = sp[0];
		*dp++ = sp[1];
		return r;
		}

	if (bothswap)		/* swap bytes within long  3<->0 && 2<->1 */
		{
		*dp++ = sp[3];
		*dp++ = sp[2];
		*dp++ = sp[1];
		*dp++ = sp[0];
		return r;
		}
}

/*
 * host to network short.
 * call *just* before writing to disk or network.
 */
short htons(s)
	short	s;
{
	return ntohs(s);
}

/*
 * network to host short.
 * call *just* after reading from disk or network.
 */
short ntohs(s)
	short	s;
{
	register	char	*sp,	/* src ptr */
				*dp;	/* dst ptr */
	short	r;			/* result - cannot be register */

	/* see the table in the comments at the top of the
	 * source listing to understand why.  The figure is
	 * worth a thousand comments. :-)
	 */

	if (!called)
		init();

	if (noswap || halfswap)
		return s;

	if (byteswap || bothswap)
		{
		sp = (char *) &s;
		dp = (char *) &r;
		*dp++ = sp[1];
		*dp++ = sp[0];
		return r;
		}
}


#ifdef STAND

#include <stdio.h>

main()
{
	long l = 0x61626364;	/* ASCII abcd */
	short s = 0x6162;	/* ASCII ab */
	long ldum;		/* long dummy */
	short sdum;		/* short dummy */

	/* so partial lines to stdout are printed immediately,
	 * so that we can mix stdio with raw i/o.
	 */
	setbuf( stdout, (char *)NULL );

	ldum = ntohl(l);
	printf("You should see \"abcd\" if ntohl() works: \"");
	write(1, &ldum, sizeof(ldum));
	printf("\"\n");

	printf("You should see \"ab\" if ntohs() works: \"");
	sdum = ntohs(s);
	write (1, &sdum, sizeof(sdum));
	printf("\"\n");

	printf("If you see any bugs, send a bug report to rlgvax!dennis\n");
	printf("Thanks.\n");
}
#endif STAND

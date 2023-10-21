
/*
 *	schwab.c
 *
 *	schwab:	calculate commisions and print gains/losses on stock
 *		transactions made through Charles Schwab Discount Brokers.
 *		Commission table accurate as of 03/01/87, for stocks over
 *		$1.00 in price.
 *
 *	Usage:
 *		schwab Nshares BuyPrice
 *			e.g. schwab 100 7
 *			prints commission cost and approximate table of
 *			sell prices for 0 - 100% net profit in 5% increments.
 *
 *		schwab Nshares BuyPrice SellPrice
 *			e.g. schwab 100 7.25 9.125
 *			prints net profit after commissions of 100 shares
 *			of stock purchased at 7 1/4 and sold at 9 1/8.
 *			
 *		schwab Nshares BuyPrice +DeltaPrice
 *			e.g. schwab 100 7.25 +.125
 *			prints net profit table for sale of 100 shares of
 *			stock purchased at 7.25 and sold at 7.25, 7.375,
 *			7.5, ... 7.25+20*.125.  Also works for negative delta.
 *
 *	Have Fun,
 *		Rick Richardson, PC Research, Inc.
 */
#include <stdio.h>

double commission(shares, price)	/* Charles Schwab */
int	shares;
double	price;
{
	double	basis, min, max, ave, comm, atof();

	basis = shares * price;
	min = .08 * first(900, shares) + .04 * thereafter(900, shares);
	max = 49 + thereafter(100, shares) * .45;
	if (basis <= 2500) ave = 19 + .016 * basis;
	else if (basis <= 6000) ave = 44 + .006 * basis;
	else if (basis <= 22000) ave = 62 + .003 * basis;
	else if (basis <= 50000) ave = 84 + .002 * basis;
	else if (basis <= 500000) ave = 134 + .001 * basis;
	else ave = 234 + .0008 * basis;
	comm = min;
	if (ave > comm) comm = ave;
	if (comm > max) comm = max;
	return (comm);
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i;
	int	shares;
	double	price;
	double	basis;
	double	nbasis;
	double	delta;
	double	even, comm, atof();

	if (argc < 3)
	{
		usage();
		exit();
	}
	shares = atoi(argv[1]);
	price = atof(argv[2]);
	basis = shares * price;
	comm = commission(shares, price);
	if (argc > 3 && (argv[3][0] == '+' || argv[3][0] == '-') )
	{
		delta = atof(argv[3]);
		argc--;
	}
	else
		delta = 0;
	if (argc > 3)
	{
		double	sprice;
		double	scomm;
		double	profit;
		sprice = atof(argv[3]);
		scomm = commission(shares, sprice);
		profit = sprice * shares - price*shares - comm - scomm;
		printf(
"Profit on %d shares bought at %.3f and sold at %.3f is $%.2f(%4.1f%%)\n",
		shares, price, sprice, profit, profit/basis*100);
		exit(0);
	}
	printf("Commission on purchase of %d shares at $%.3f is $%.2f\n",
		shares, price, comm +.005);
	if (delta) for (i = 0; i < 20; ++i)
	{
		double	sprice;
		double	scomm;
		double	profit;
		sprice = price + i * delta;
		if (sprice < 0) break;
		scomm = commission(shares, sprice);
		profit = sprice * shares - price*shares - comm - scomm;
		printf(
"Profit on %d shares bought at %.3f and sold at %.3f is $%.2f(%4.1f%%)\n",
		shares, price, sprice, profit, profit/basis*100);
	}
	else for (i = 0; i < 100; i += 5)
	{
		double	profit;

		nbasis = (basis+comm)*(100+i);
		nbasis /= 100;
		profit = nbasis - price*shares - comm;
		nbasis += commission(shares, nbasis/shares);
		printf("Make %d%%($%.2f) if sold at about $%.2f\n",
			i, profit, nbasis/shares);
	}
}

first(n, shares) { if (shares < n) return (shares); else return (n); }

thereafter(n, shares) { if (shares > n) return (shares-n); else return (0); }

usage()
{
	fprintf(stderr, "Usage:	schwab Nshares BuyPrice\n");
	fprintf(stderr, "Or:	schwab Nshares BuyPrice SellPrice\n");
	fprintf(stderr, "Or:	schwab Nshares BuyPrice +DeltaPrice\n");
	fprintf(stderr, "Or:	schwab Nshares BuyPrice -DeltaPrice\n");
}

/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

double sqrt ();
double	critf ();
double
scheffe (p, df1, df2, mse, n) double p, mse;
	{
	return (sqrt (df1 * critf (p, df1, df2) * mse * 2 / n));
	}
double
lsd (p, df1, df2, mse, n) double p, mse;
	{
	return (sqrt (critf (p, 1, df2) * mse * 2 / n));
	}
main (argc, argv) char **argv;
	{
	double	atof ();
	double	prob = atof (argv[1]);
	int	df1 = atoi (argv[2]);
	int	df2 = atoi (argv[3]);
	double	mse = atof (argv[4]);
	int	n = atoi (argv[5]);
	if (argc != 6)
		{
		printf ("USAGE: %s prob df1 df2 mse n\n", argv[0]);
		exit (1);
		}
	printf ("LSD = %.3g\n", lsd (prob, df1, df2, mse, n));
	printf ("S   = %.3g\n", scheffe (prob, df1, df2, mse, n));
	}

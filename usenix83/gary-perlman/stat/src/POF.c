/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

#include <stdio.h>
main (argc, argv) int argc; char **argv;
	{
	double	F;
	double	p;
	double	pof ();
	double	atof ();
	int	df1;
	int	df2;
	if (argc != 4)
		eprintf ("USAGE: %s F df1 df2\n", argv[0]);
	if ((F	=	atof (argv[1])) <= 0.0)
		eprintf ("%s: F-ratio must be positive\n", argv[0]);
	df1	=	atoi (argv[2]);
	df2	=	atoi (argv[3]);
	if (df1 <= 0 || df2 <= 0)
		eprintf ("%s: df must be positive\n", argv[0]);
	p	=	pof  (F, df1, df2);
	printf ("F(%d,%d) = %4.3f, p = %4.3f\n", df1, df2, F, p);
	}

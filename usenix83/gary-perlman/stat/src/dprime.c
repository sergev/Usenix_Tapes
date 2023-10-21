/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

#include <stdio.h>
#define	MAXCOLS	3
#define	MAXSTRING 10
char	program[50];
FILE	*datafile;
char	column[MAXCOLS][MAXSTRING];
int	ncols;
int	presented, obtained;
int	hit, miss, false_alarm, correct_rejection;
double	hr, far;
double	dprime, beta;
main (argc, argv) char **argv;
	{
	double	atof (), ptoz (), normaldensity ();
	strcpy (program, argv[0]);
	if (argc == 2)
		{
		if ((datafile = fopen (argv[1], "r")) == NULL)
			eprintf ("%s: can't open %s\n", program, argv[1]);
		}
	else
		{
		datafile = stdin;
		checkstdin (argv[0]);
		}
	if (argc == 3)
		{
		hr = atof (argv[1]);
		far = atof (argv[2]);
		goto fastmode;
		}
	
	while ((ncols = fstrings (datafile, column, MAXCOLS, MAXSTRING)) != EOF)
		{
		if (ncols != 2)
			eprintf ("%s: each line must have 2 columns\n", program);
		presented = yesno (column[0]);
		obtained  = yesno (column[1]);

		if (presented)
			if (obtained) hit++;
			else miss++;
		else if (obtained) false_alarm++;
		else correct_rejection++;
		}
	printf ("%15s%15s%15s\n", "", "signal", "noise");
	printf ("%15s%15d%15d\n", "yes", hit, false_alarm);
	printf ("%15s%15d%15d\n", "no", miss, correct_rejection);
	printf ("\n");
	hr = hit / (double) (hit + miss);
	far = false_alarm / (double) (false_alarm + correct_rejection);
fastmode:
	if (hr < 0.0 || hr > 1.0)
	    eprintf ("%s: The hit rate %f is not a probability\n", program, hr);
	if (far < 0.0 || far > 1.0)
		eprintf ("%s: The false-alarm rate %f is not a probability\n",
			program, far);
	printf ("%15s%15s%15s\n", "rates:", "hit", "false alarm");
	printf ("%15s%15.2f%15.2f\n", "", hr, far);

	dprime = ptoz (hr) - ptoz (far);
	beta   = normaldensity (ptoz (hr)) / normaldensity (ptoz (far));

	printf ("%15s%15.2f\n", "dprime", dprime);
	printf ("%15s%15.2f\n", "beta", beta);
	}
char	*yesses[10] = {"yes", "1", "1.0000", "signal"};
char	*noes[10]   = {"no",  "0", "0.0000", "noise"};
yesno (string) char *string;
	{
	int	strno = 0;
	while (*yesses[strno] && strcmp (string, yesses[strno])) strno++;
	if (*yesses[strno]) return (1);
	strno = 0;
	while (*noes[strno] && strcmp (string, noes[strno])) strno++;
	if (*noes[strno]) return (0);
	eprintf ("%s: illegal value in datafile\n", program);
	}
double normaldensity (z) double z;
	{
	double exp (), sqrt ();
	return (exp (-z*z/2.0)/sqrt (2*3.1415926535));
	}

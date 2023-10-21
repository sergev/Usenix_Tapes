/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

double
critf (p, df1, df2) double p;
	{
	double	critf = 5.0;
	char	goingup = 1;
	double	step = 4.0;
	double	holdp = 1.1;
	double	fabs ();
	double	pof ();
	if (p <= 0.0 || p > 1.0) return (0.0);
	while (fabs (p-holdp) > 0.00001)
		{
		if ((holdp = pof (critf, df1, df2)) > p)
			{
			if (!goingup) step /= 2;
			goingup = 1;
			critf += step;
			}
		else
			{
			if (goingup) step /= 2;
			goingup = 0;
			critf -= step;
			}
		}
	critf += goingup ? step : -step;
	return (critf);
	}

#include <stdio.h>
#include <math.h>
main (argc, argv) char **argv;
	{
	char	datum[BUFSIZ];
	char	dummy[BUFSIZ];
	char	*numtype;
	double	fval;
	int	magnitude;
	char	*size;
	char	*sign;
	switch (argc)
		{
		case 1:
			checkstdin (argv[0]);
			if (scanf ("%s", datum) != 1)
				eprintf ("%s: No datum", argv[0]);
			if (scanf ("%s", dummy) == 1)
				eprintf ("%s: Too much data", argv[0]);
			break;
		case 2:
			strcpy (datum, argv[1]);
			break;
		default:
			eprintf ("%s: Only one argument datum allowed", argv[0]);
		}
	switch (number (datum))
		{
		case 0:
			eprintf ("%s: %s is not a number", argv[0], datum);
		case 1:
			numtype = "integer";
			break;
		case 2:
			numtype = "real number";
			break;
		}
	fval = atof (datum);
	if (fval == 0.0)
		printf ("%s looks like a zero to me.\n", datum);
	else
		{
		sign = fval < 0.0 ? "negative" : "positive";
		magnitude = floor (log10 (fabs (fval)));
		switch (magnitude)
			{
			case 0: size = "small"; break;
			case 1: size = "smallish"; break;
			case 2: size = "bigish"; break;
			case 3: size = "big"; break;
			case 4: size = "huge"; break;
			case 5: size = "humongous"; break;
			case 6: size = "humendous"; break;
			default:
				if (magnitude < 0)
					size = "tiny";
				else
					size = "gargantuan";
			}
		printf ("%s is a %s %s %s.\n",
			datum, size, sign, numtype);
		}
	}

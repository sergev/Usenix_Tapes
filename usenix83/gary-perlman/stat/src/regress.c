/*
                  Copyright 1980  Gary Perlman.
Distribution of this program and/or any aupporting documentation
for material or personal gain is prohibited.  Copies may be freely
distributed provided this copyright notice is included.

No guarantee of performance in made or this program, but it has
been extensively checked.  Comments and/or complaints are welcome
and should be directed to:  G. Perlman, Psychology C009, UCSD,
La Jolla, CA 92093.
	This program does multivariate linear regression/correlation.
	It was written by Gary Perlman of UCSD in May 1980.

		STRUCTURE of CORR/REGRESS
	main
		input
			number
			sstrings
		compute
		regress
			invert
			pof

		ALPHABETICAL ANNOTATION
	compute ()
		computes and prints means, sds, correlations, names
	errorexit (message)
		prints its message, beeps, and exits
	input (argc, argv)
		reads from the standard input int sums and covar
		checks for argument names and stores any in varname
		sets regress if program is called under alias "regress"
		sets nvar to number of variables
		checks data for validity
	invert (matrix, size)
		inverts the correlation matrix and returns determinant
	number (string)
		returns TRUE if its string argument is numerical
	pof (F, df1, df2)
		returns the probability of the F ratio
	regress ()
		computes and prints slopes, intercepts, Rsq's and F's
	sstrings (line, array, maxstrings, maxchars)
		reads from its line argument into an array of strings
		reads at most maxstrings strings of length at most maxchars-1
		returns the number of strings read in


*/
#include <stdio.h>			/* standard I/0 library		      */
#include <ctype.h>			/* standard I/0 library		      */
#define MAXVAR		30		/* max number of variables	      */
#define MAXLEN		30		/* maximum length of a string	      */
typedef int BOOLEAN;			/* no boolean type in C		      */
#define FALSE		 0
#define TRUE		 1
BOOLEAN	doreg = FALSE;			/* if TRUE then do regression analysis*/
char	varname[MAXVAR][MAXLEN]		/* names of variables: default below  */
	= { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
		"K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
		"U", "V", "W", "X", "Y", "Z" };
int	npoints = 0;			/* number of points/lines read in     */
int	nvar;				/* number of variables read in	      */
double	correl[MAXVAR][MAXVAR];		/* correlation matrix		      */
double	covar[MAXVAR][MAXVAR];		/* covariance matrix		      */
double	sum[MAXVAR];			/* sum of scores for each variable    */
float	min[MAXVAR];			/* minimum value of each variable     */
float	max[MAXVAR];			/* maximum value of each variable     */
/*				MAIN					      */
main (argc, argv) int argc; char **argv;
	{
	input (argc, argv);
	compute ();
	if (doreg) regress ();
	}

/*				INPUT					      */
input (argc, argv) int argc; char **argv;
	{
	char	 line[BUFSIZ];		/* each data line read in here	      */
	char	 *ptr;
	char	 in[MAXVAR][MAXLEN];	/* each input column of line in here  */
	extern	 double atof ();	/* ascii to floating point	      */
	float	 temp[MAXVAR];		/* each input column number in here   */
	register int row, col;		/* looping variables		      */

	checkstdin (argv[0]);
	/* CHECK argv[0] for regression option */
	if (!strcmp (argv[0], "regress") || !strcmp (argv[0], "a.out"))
		doreg = TRUE;

	/* STORE names of variables if there are any */
	for (row = 0; row < argc-1 && row < MAXVAR; row++)
		{
		if (strlen (argv[row+1]) >= MAXLEN)
			argv[row+1][MAXLEN-1] = NULL;
		strcpy (varname[row], argv[row+1]);
		}

	/* SET nvar */
    firstline:
	if (!fgets (line, BUFSIZ, stdin))
		errorexit ("No data");
	ptr = line;
	while (isspace (*ptr)) ptr++;
	if (*ptr == NULL) goto firstline;
	nvar = sstrings (line, (char *) in, MAXVAR, MAXLEN);
	if (nvar > MAXVAR)
		errorexit ("Can't handle this many variables");
	for (row = 0; row < nvar; row++)
		min[row] = max[row] = atof (in[row]);

	/* READ in rest of data points */
	do	{
		ptr = line;
		while (isspace (*ptr)) ptr++;
		if (*ptr == NULL) continue;
		if (sstrings (line, (char *) in, MAXVAR, MAXLEN) != nvar)
			errorexit ("Ragged input");
		for (row = 0; row < nvar; row++)
			{
			if (number (in[row]))
				temp[row] = atof (in[row]);
			else errorexit ("Non-numerical input");
			sum[row] += temp[row];
			if (temp[row] > max[row]) max[row] = temp[row];
			if (temp[row] < min[row]) min[row] = temp[row];
			for (col = 0; col <= row; col++)
				covar[row][col] += temp[row] * temp[col];
			}
			npoints++;
		} while (fgets (line, BUFSIZ, stdin));
	if (npoints <= 1) errorexit ("Not enough points for analysis");
	} /* ends input */

/*				COMPUTE					      */
compute ()
	{
	register int row, col;
	double	denom;
	extern	double sqrt ();

	/* NORMALIZE covariance matrix */
	for (row = 0; row < nvar; row++)
	for (col = 0; col <= row; col++)
		covar[row][col] = covar[row][col] - sum[row]*sum[col]/npoints;

	/* COMPUTE and PRINT means and standard deviations */
	printf ("Analysis for %d points of %d variables:\n", npoints, nvar);
	printf ("%-10s: ", "VARIABLE");
	for (row = 0; row < nvar; row++)
		printf ("%10.10s ", varname[row]);
		printf ("\n");
	printf ("%-10s: ", "MIN");
	for (row = 0; row < nvar; row++)
		printf ("%10.4f ", min[row]);
		printf ("\n");
	printf ("%-10s: ", "MAX");
	for (row = 0; row < nvar; row++)
		printf ("%10.4f ", max[row]);
		printf ("\n");
	printf ("%-10s: ", "MEAN");
	for (row = 0; row < nvar; row++)
		printf ("%10.4f ", sum[row]/npoints);
		printf ("\n");
	printf ("%-10s: ", "SD");
	for (row = 0; row < nvar; row++)
		printf ("%10.4f ", sqrt (covar[row][row]/(npoints-1)));
		printf ("\n");

	/* COMPUTE and PRINT cross correlations */
	printf ("CORRELATION MATRIX:\n");
	for (row = 0; row < nvar; row++)
		{
		printf ("%-10.10s: ", varname[row]);
		for (col = 0; col < row; col++)
			if ((denom = covar[row][row] * covar[col][col]) != 0.0)
				{
				correl[row][col] = covar[row][col]/sqrt (denom);
				printf ("%10.4f ", correl[row][col]);
				}
			else printf ("%10.4f ", 0.0);
		printf ("%10.4f\n", correl[row][row] = 1.0);
		}
		printf ("%-10s: ", "VARIABLE");
		for (col = 0; col < nvar; col++)
			printf ("%10.10s ", varname[col]);
			printf ("\n");
	} /* ends compute */

/*				REGRESSION				      */
regress ()
	{
	register int row, col;		/* looping variables		      */
	register int var;		/* looping variable		      */
	extern	double sqrt ();		/* square root function		      */
	double	pof ();			/* probability of F		      */
	double	invert ();		/* matrix inversion. returns det      */
	double	b[MAXVAR][MAXVAR];	/* regression coefficients slopes     */
	double	a[MAXVAR];		/* intercept in regression equation   */
	double	F[MAXVAR];		/* F ratio			      */
	double	Rsq[MAXVAR];		/* squared correlation coefficient    */
	int	npred = nvar - 1;	/* number of predictors		      */
	int	dferror = npoints - nvar;/* degrees of freedom for error      */
	char	Fline[20];		/* holds the F ratio line	      */

	/* INVERT the correlation matrix */
	for (row = 0; row < nvar; row++)
	for (col = 0; col < row; col++)
		correl[col][row] = correl[row][col];
	if (invert (correl, nvar) == 0.0)
		errorexit ("Non-singular correlation matrix");

	/* PRINT regression equation */
	for (var = 0; var < nvar; var++)
		{
		double	rdiag = correl[var][var];
		double	cdiag = covar[var][var];
		a[var] = sum[var];
		for (col = 0; col < nvar; col++)
		    if (col != var && rdiag * covar[col][col] != 0.0)
			{
			b[var][col] = (-correl[var][col] / rdiag) *
				sqrt (cdiag/covar[col][col]);
			a[var] -= b[var][col] * sum[col];
			}
		    else b[var][col] = 0.0;
		}
	printf ("REGRESSION EQUATIONS:\n");
	printf ("%-10s:\n", "SLOPES");
	for (var = 0; var < nvar; var++)
		{
		printf ("%-10.10s: ", varname[var]);
		for (col = 0; col < nvar; col++)
			if (col != var) printf ("%10.4f ", b[col][var]);
			else printf ("%10s ", "");
		printf ("\n");
		}
	printf ("%-10s: ", "INTERCEPT");
	for (col = 0; col < nvar; col++)
		printf ("%10.4f ", a[col] /= npoints);
		printf ("\n");

	/* PRINT sums of squares */
	printf ("%-10s: ", "SStotal");
	for (var = 0; var < nvar; var++)
		printf ("%10.4f ", covar[var][var]);
	printf ("\n");

	/* COMPUTE Rsq and PRINT significance test */
	printf ("%-10s: ", "R-Square");
	for (var = 0; var < nvar; var++)
		{
		if (correl[var][var] != 0.0)
			Rsq[var] = 1.0 - 1.0/correl[var][var];
		else Rsq[var] = 0.0;
		printf ("%10.4f ", Rsq[var]);
		}
		printf ("\n");
	sprintf (Fline, "F(%d,%d)", npred, dferror);
	printf ("%-10s: ", Fline);
	for (var = 0; var < nvar; var++)
		{
		if (Rsq[var] == 1.0) F[var] = 99999.999;
		else F[var] = Rsq[var] * dferror / (npred * (1.0 - Rsq[var]));
		printf ("%10.4f ", F[var]);
		}
		printf ("\n");
	printf ("%-10s: ", "prob (F)");
	for (var = 0; var < nvar; var++)
		printf ("%10.4f ", pof (F[var], npred, dferror));
		printf ("\n");
	} /* ends regress */

/*				INVERT					      */
/* returns the determinant of its input matrix. modifies matrix.	      */
double
invert (matrix, size) double matrix[MAXVAR][MAXVAR]; int size;
	{
	int	j,k,l;
	double	pivot;
	double	temp;
	double	det = 1.0;
	for (j = 0; j < size; j++)
		{
		pivot = matrix[j][j];
		det *= pivot;
		matrix[j][j] = 1.0;
		for (k = 0; k < size; k++)
			if (pivot != 0.0) matrix[j][k] /= pivot;
			else return (0.0);
		for (k = 0; k < size; k++)
			if (k != j)
				{
				temp = matrix[k][j];
				matrix[k][j] = 0.0;
				for (l = 0; l < size; l++)
					matrix[k][l] -= matrix[j][l] * temp;
				}
		}
	return (det);
	} /* ends invert */

errorexit (message) char *message;
	{
	fprintf (stderr, "%s.\n", message);
	fprintf (stderr, "Try 'man regress' for an explanation.\n");
	exit (1);
	}

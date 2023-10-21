/*
                  Copyright 1980  Gary Perlman.
Distribution of this program and/or any aupporting documentation
for material or personal gain is prohibited.  Copies may be freely
distributed provided this copyright notice is included.

No guarantee of performance in made or this program, but it has
been extensively checked.  Comments and/or complaints are welcome
and should be directed to:  G. Perlman, Psychology C009, UCSD,
La Jolla, CA 92093.
*/
/* vincentizing is a procedure in which an ordered list of numbers
   is divided into a chosen number of vincentiles and the average
   for each of these vincentiles is computed.  Vincentizing is useful
   for comparing trends in data for which unequal sample sizes exist.
   This program takes its first argument as the number of tiles it
   is to divide the subsequent files' data into.  vincent prints
   the number of numbers read in, the name of the file it read them from,
   and the means for each vincentile.

	This program was written by Gary Perlman in November of 1980.
*/
#include <stdio.h>
#define MAXPOINTS 5000		/* maximum number of points to store	*/
#define MAXTILES MAXPOINTS/10	/* maximum number of tiles		*/
main (argc, argv) char **argv;
	{
	float	store[MAXPOINTS];	/* stored data from each file	*/
	double	sum[MAXTILES];		/* sum of each n-tile		*/
	int	nfiles = argc-2;	/* number of files		*/
	int	ntiles = atoi (argv[1]);/* number of vincentiles	*/
	int	file;			/* loop variable		*/
	FILE	*ioptr, *fopen ();	/* data read from file pointer	*/
	int	n;			/* number of points from file	*/
	register int i;			/* loop variable		*/

	if (argc <= 2)
		eprintf ("USAGE: %s n-tiles file1 file2 ...\n", argv[0]);
	if (ntiles < 1 || ntiles > MAXTILES)
		eprintf ("%s: maximum number of n-tiles = %d\n", argv[0], MAXTILES);

	/* MAIN LOOP */
	for (file = 0; file < nfiles; file++)
		{
		if ((ioptr = fopen (argv[file+2], "r")) == NULL)
			eprintf ("%s: can't open %s\n", argv[0], argv[file+2]);
		n = fgetf (ioptr, MAXPOINTS, store);
		printf ("%4d %-6s ", n, argv[file+2]);
		if (n == 0)
			eprintf ("%s: not enough data\n", argv[0]);
		for (i = 0; i < ntiles; i++) sum[i] = 0.0;
		for (i = 0; i < n*ntiles; i++)
			sum[i/n] += store[i/ntiles];
		for (i = 0; i < ntiles; i++)
			printf ("%12.4f ", sum[i]/n);
		printf ("\n");
		fclose (ioptr);
		}
	}
/* fgetf is a floating version of fgets */
fgetf (ioptr, max, array) FILE *ioptr; float *array;
	{
	register int i;
	i = 0;
	while (i < max && (fscanf (ioptr, "%f", array+i) == 1)) i++;
	return (i);
	}

/*
                  Copyright 1980  Gary Perlman.
Distribution of this program and/or any aupporting documentation
for material or personal gain is prohibited.  Copies may be freely
distributed provided this copyright notice is included.
No guarantee of performance in made or this program, but it has
been extensively checked.  Comments and/or complaints are welcome
and should be directed to:  G. Perlman, Psychology C009, UCSD,
La Jolla, CA 92093.
	This program does a general analysis of variance.
	It was written by Gary Perlman at UCSD April 1980.
		STRUCTURE OF ANOVA
	main
		getlevels
			number
			sstrings
			copy
		readdata
			sstrings
			offset
		cellmeans
			offset
			setsource
			nextlevel
			printmean
		anova
			setsource
			pof
		ALPHABETICAL ANNOTATION
	anova ()
		prints the design summary
		prints F table with each systematic source
		being tested against its own error term
	cellmeans ()
		allocates space for sums of squares of sums
		computes brackets == sums of squares of sums
		determines whether type of factors is BETWEEN
		prints the cell means, counts, and standard deviations
		checks the integrity of design
		frees data array space
	copy (string)
		returns a runtime allocated copy of string
	errorexit (message)
		prints its argument message, beeps, and exits
	getlevels (argc, argv)
		reads from the standard input
		checks for arguments and stored them in factorname[i]
		opens datafile to store copy of input data
		sets nfactors, nlevels[factor], and levelnames
		checks whether data is numerical
	nextlevel (level, source, sourceflag)
		increments the indicies in level to loop through
		all nlevels of each factor.  If sourceflag is true,
		the source[factors] are incremented, otherwise the
		non-source[factors] are.  returns false if there
		is no nextlevel.
	number (string)
		returns true if its string argument is a number
		used to check the validity of input data
	offset (level)
		for its argument int array level, returns array index
		makes data array seem like a multidimensional array
	pof (F, df1, df2)
		computes the probability of F ratio
	printmean (count, sum, sumsq)
		prints N, MEAN, and SD if appropriate
	readdata ()
		allocates space for data and number of replications
		reads from temporary file containing copy of data
		stores the data by calling offset with level numbers
		averages the data over replications
	sstrings (line, matrix, maxstrings, maxchars)
		reads from line argument into matrix argument columns
		each column will hold at most maxchars-1 characters
		returns the number of columns read in (at most maxstrings)
	setsource (sourcenum, source)
		for its argument sourcenum, returns a binary
		representation of it in its argument array, source
*/

#include <stdio.h>		/* standard I/O library			      */
#include <ctype.h>
#include <signal.h>		/* used to catch interrupts		      */
#define MAXFACT 	 10	/* the maximum number of factors              */
#define MAXLEV		500	/* maximum number of levels per factor	      */
#define MAXSTRING 	 16	/* the maximum length of an input string      */
#define RANDOM	 	  0	/* zeroeth column is RANDOM factor	      */
typedef int boolean;		/* no boolean type in C                       */
#define true  		  1
#define false 		  0
/*				GLOBALS					      */
int	nfactors;		/* total number of factors                    */
unsigned nsources = 1;		/* number of sources                          */
int	nlevels[MAXFACT];	/* number of levels of each factor            */
boolean	between[MAXFACT];	/* true if factor is between                  */
char	*factorname[MAXFACT+1]   /* default names for factors		      */
	= {"RANDOM", "A", "B", "C", "D", "E", "F", "G", "H", "I"};
char	*levelname[MAXFACT][MAXLEV];	/* level names			      */
float	*datax;			/* will hold all the data                     */
char	*nreplics;		/* number of replications in each cell        */
double	*bracket;		/* will hold bracket values                   */
FILE	*datafile;		/* input file                                 */
char	tmpname[20];		/* temporary file where data are stored       */
boolean	errorflag = false;	/* set to true in case of fatal error	      */
/*				MAIN					      */
main (argc, argv) int argc; char *argv[];
	{
	extern	onintr ();
	signal (SIGINT, onintr);
	checkstdin (argv[0]);
	getlevels (argc, argv);
	readdata ();
	cellmeans ();
	anova ();
	}
onintr () { unlink (tmpname); exit (1); }

/*				GETLEVELS				      */
/* getlevels finds the number of levels of each factor.
   For each line, it reads in the levels of each factor.
   It assumes that the number of levels equals the maximum levelnumber.
   The data is read from the stdin but is copied for further use in datafile.
*/
getlevels (argc, argv) int argc; char **argv;
    {
    register int factor;			/* looping variable	      */
    register int level;				/* looping variable	      */
    char	line[BUFSIZ];			/* each data line read in here*/
    char	*ptr;
    char	column[MAXFACT+2][MAXSTRING];	/* data line separated in cols*/
    int	ncols;					/* number of columns in line  */
    boolean	newlevel;			/* T if new level name	      */
    char	*copy ();
    int	lineno = 0;				/* counts input lines         */

    /* if there are arguments, STORE their names */
    for (factor = 0; factor < argc-1 && factor <= MAXFACT; factor++)
	    factorname[factor] = copy (argv[factor+1]);

    /* OPEN datafile to store copy of data */
    sprintf (tmpname, "/tmp/anova.%d", getpid ());
    if ((datafile = fopen (tmpname, "w")) == NULL)
	errorexit ("Unable to open temporary file");

    /* SET nfactors and INITIALIZE levels */
firstline:
    if (!fgets (line, BUFSIZ, stdin))
	errorexit ("No data");
    ptr = line;
    while (isspace (*ptr)) ptr++;
    if (*ptr == NULL) goto firstline;
    nfactors = sstrings (line, (char *) column, MAXFACT+2, MAXSTRING) - 1;
    if (nfactors > MAXFACT) errorexit ("Too many columns in data file");
    if (argc < nfactors+2) factorname[nfactors] = "DATA";

    /* SET nlevels[factor] and levelname[factor] */
    do  {
	ptr = line;
	while (isspace (*ptr)) ptr++;
	if (*ptr == NULL) continue;
	fputs (line, datafile);
	lineno++;
	ncols = sstrings (line, (char *) column, MAXFACT+1, MAXSTRING);
	if (ncols != nfactors+1)
	    {
	    if (ncols == 0) continue;
	    fprintf (stderr, "Error on line %d\n", lineno);
	    errorexit ("Ragged input");
	    }

	/* see if there are any new levels */
	for (factor = 0; factor < nfactors; factor++)
	    {
	    newlevel = true;
	    for (level = 0; level < nlevels[factor]; level++)
		if (!strcmp (levelname[factor][level], column[factor]))
			{newlevel = false; break;}
	    if (newlevel)
		{
		if (nlevels[factor] == MAXLEV) errorexit ("Too many levels");
		levelname[factor][nlevels[factor]] = copy (column[factor]);
		nlevels[factor]++;
		}
	    }

	/* CHECK to make sure that data is numerical */
	if (!number (column[nfactors])) /* data column */
	    {
	    fprintf (stderr, "Error on line %d\n", lineno);
	    errorexit ("Non-numerical data");
	    }
	} while (fgets (line, BUFSIZ, stdin));
    } /* ends getlevels */

/*				COPY					      */
char *
copy (string) char *string;
	{
	char *copy, *calloc ();
	if ((copy = calloc (1, (unsigned) strlen (string) + 1)) == NULL)
		errorexit ("No more space for storing strings");
	strcpy (copy, string);
	return (copy);
	}

/*				READDATA				      */
/* readdata reads in the data from datafile and stores it in the data array.
   Space is allocated for the data array and the number of replics per cell.
   For each line, it reads the levels of each factor and finds the location
   where the data is to be stored in data by calling offset with the level
   numbers stored in the array called level.  Any space not used in data
   (because of nested design, for example) has nreplics == 0.
   Finally, it removes the temporary data file.
*/
readdata ()
    {
    register int address;		/* where data will be added	      */
    register int factor;		/* looping variable		      */
    extern	double atof ();		/* ascii to floating point	      */
    int	level[MAXFACT];			/* level of each factor		      */
    char	line[BUFSIZ];		/* each data input line read in here  */
    char	column[MAXFACT+1][MAXSTRING]; /* data line in columns	      */
    unsigned ncells = 1;		/* product of all levels	      */
    char	*calloc ();

    /* ALLOCATE space for data assuming worst case */
    for (factor = 0; factor < nfactors; factor++)
	    ncells *= nlevels[factor];
    if ((datax = (float *) calloc (ncells, sizeof (float))) == NULL)
	    errorexit ("Can't allocate space for data");
    if ((nreplics = (char *) calloc (ncells, sizeof (char))) == NULL)
	    errorexit ("Can't allocate space for data");

    /* STORE data in data array */
    datafile = freopen (tmpname, "r", datafile);
    while (fgets (line, BUFSIZ, datafile))
	{
	sstrings (line, (char *) column, MAXFACT+1, MAXSTRING);
	for (factor = 0; factor < nfactors; factor++)
	    {
	    level[factor] = 0;
	    while (strcmp (column[factor], levelname[factor][level[factor]]))
		level[factor]++;
	    }
	address = offset (level);
	nreplics[address]++;
	datax[address] += atof (column[nfactors]);
	}

    /* AVERAGE data over all replications */
    for (address = 0; address < ncells; address++)
	if (nreplics[address] > 1) datax[address] /= nreplics[address];
    unlink (tmpname); /* done with datafile */
    } /* ends readdata */

/*				OFFSET					      */
/* offset returns a unique index for each combination of levels of factors
   it gets passed in level[factor]. level[factor] >= 0 for all factors.
*/
int
offset (level) int level[MAXFACT];
	{
	register int factor;		/* looping variable		      */
	int	index;			/* level of each factor read in here  */
	int	coeff = 1;		/* index multiplied by coeff	      */

	index = level[nfactors-1];
	for (factor = nfactors-2; factor >= 0; factor--)
		{
		coeff *= nlevels[factor+1];
		index += coeff * (level[factor]);
		}
	return (index);
	} /* ends offset */

/*				CELLMEANS				      */
/* cellmeans does the heavy computation involved in computing sums of squares
   as well as printing cell means.  The input to cellmeans is a set
   of arrays holding design information, and the data array which has to be
   in a particular format as defined by the program, offset.
   cellmeans also determines the type of each factor.
   The algorithm procedes as follows:
	for each source (from 0 to 2**nfactors), the sums and sums of squares
	for each level of that source are computed by looping through all
	the non-sources.  From this information, the mean and sd are
	printed for each level, and a term is added into bracket[sourcenum],
	an array of sums of squares of sums.  This array will be used by anova.
*/
cellmeans ()
    {
    register int factor;		/* looping variable                   */
    int	level[MAXFACT];			/* level of each factor               */
    int	sourcenum;			/* source number                      */
    boolean	source[MAXFACT];	/* boolean powerset of factors        */
    int	nterms;				/* number of terms in source          */
    double	datum;			/* each datum read into here          */
    int	address;			/* result from ofset                  */
    double	sum;			/* sum for each level of each factor  */
    double	sumsq;			/* for each level of each factor      */
    int	count;				/* count used along with sum          */
    int	sumcount[MAXFACT];		/* sum of counts of all levels of fact*/
    int	withprod = 1;			/* product of nlevels[WITHIN]         */
    boolean	sources, nonsources;	/* while loop variables               */
    double	sqrt ();		/* square root function		      */
    char	*calloc ();

    /* INITIALIZE sumcount. Used to check integrity of design */
    for (factor = 0; factor < nfactors; factor++) sumcount[factor] = 0;
    
    /* ALLOCATE space for sums of squares of sums */
    nsources = 1 << nfactors;
    if ((bracket = (double *) calloc (nsources, sizeof (double))) == NULL)
	errorexit ("Can't allocate space for computations");

    /* begin huge loop */
    for (sourcenum = 0; sourcenum < nsources; sourcenum++)
	{
	nterms = setsource (sourcenum, source);

	/* PRINT names of sources */
	if (!source[RANDOM])
	    {
	    printf ("SOURCE: ");
	    if (sourcenum == 0) printf ("grand mean");
	    else for (factor = 1; factor < nfactors; factor++)
		if (source[factor]) printf ("%s ", factorname[factor]);
	    printf ("\n");

	    /* PRINT header for cellmeans table */
	    for (factor = 1; factor < nfactors; factor++)
		printf ("%-5.5s  ", factorname[factor]);
	    printf ("   N       MEAN         SD\n");
	    }

	/* COMPUTE sums and brackets */
	for (factor = 0; factor < nfactors; factor++) level[factor] = 0;
	sources = true;		/* starts up sources loop */
	while (sources)
	    {
	    sum =   0.0;
	    sumsq = 0.0;
	    count = 0;
	    nonsources = true;	/* starts up nonsources loop */
	    while (nonsources)
		{
		address = offset (level);
		if (nreplics[address])
		    {
		    datum = datax[address];
		    sum += datum;
		    sumsq += datum*datum;
		    count++;
		    }

		/* UPDATE levels of nonsource factors */
		nonsources = nextlevel (level, source, false);
		} /* ends nonsources */

	    /* PRINT cell means */
	    if (!source[RANDOM])
		{
		for (factor = 1; factor < nfactors; factor++)
		    if (source[factor])
			{
			printf ("%-5.5s  ", levelname[factor][level[factor]]);
			if (nterms == 1) sumcount[factor] += count;
			}
		    else printf ("       ");
		printmean (count, sum, sumsq);
		}

	    /* UPDATE bracket or type of factor */
	    if (count) bracket[sourcenum] += sum*sum/count;
	    else if (nterms == 2 && source[RANDOM])
		for (factor = 1; factor < nfactors; factor++)
		    if (source[factor]) {between[factor] = true; break;}

	    /* UPDATE levels of source factors */
	    sources = nextlevel (level, source, true);
	    } /* ends sources */
	if (!source[RANDOM]) printf ("\n");
	} /* ends for each sourcenum */

    /* CHECK integrity of the design */
    for (factor = 0; factor < nfactors; factor++)
	if (!between[factor]) withprod *= nlevels[factor];
    for (factor = 1; factor < nfactors; factor++)
	if (sumcount[factor] != withprod)
	    {
	    fprintf (stderr, "%s is unbalanced.\n", factorname[factor]);
	    errorflag = true;
	    }

    /* FREE stored data */
    cfree ((char *) datax);
    cfree ((char *) nreplics);
    } /* ends cellmeans */

/*				NEXTLEVEL				      */
boolean
nextlevel (level, source, sourceflag)
	int level[MAXFACT];
	boolean source[MAXFACT], sourceflag;
	{
	register int factor;

	/* if sourceflag is true, increment a source factor,
	   otherwise, increment a non-source factor */
	for (factor = nfactors-1; factor >= 0; factor--)
		if (sourceflag == source[factor])
			if (++level[factor] < nlevels[factor]) break;
			else level[factor] = 0;
	return (factor >= 0);
	} /* ends nextlevel */
/*				PRINTMEAN				      */
printmean (count, sum, sumsq) double sum, sumsq;
    {
    if (count)
	{
	printf ("%4d %10.4f ", count, sum/count);
	if (count > 1) /* ok to compute sd */
	    printf ("%10.4f", sqrt ((sumsq-sum*sum/count)/(count-1)));
	}
    else
	{
	printf ("   Empty cells are not allowed!");
	errorflag = true;
	}
    printf ("\n");
    } /* ends printmean */

/*				SETSOURCE				      */
/* setsource takes sourcenum and turns it into a binary representation in
   source[factor]. It returns the number of factors in the source array.
*/
int
setsource (sourcenum, source) int sourcenum; boolean *source;
	{
	register int factor;		/* looping variable		      */
	int nterms = 0;			/* number of terms in source	      */

	for (factor = 0; factor < nfactors; factor++)
		{
		if (sourcenum % 2)
			{
			source[factor] = true;
			nterms++;
			}
		else source[factor] = false;
		sourcenum /= 2;
		}
	return (nterms);
	} /* ends setsource */

/*				ANOVA					      */
/* anova assumes that the array bracket has been allocated and set up with
   bracket values as described in Keppel.  anova's task is to compute from
   these bracket values the ss for each source, and then, with the array
   between (which holds the type of each factor (WITHIN or BETWEEN)) gets
   the error term for each source.  anova prints for each testable
   source, a mini-F-table with its particular error term.
*/
anova ()
    {
    register int factor;		/* looping variable		      */
    int	sourcenum;			/* source number		      */
    boolean	source[MAXFACT];	/* flag for each factor in source     */
    int	nterms;				/* number of terms in source	      */
    register int subsource;
    boolean	tmpsource[MAXFACT];	/* flag for each factor in subsource  */
    int	nsubs;				/* number of terms in subsource       */
    boolean	error[MAXFACT];		/* will hold factors in error term    */
    int	nerror;				/* number of terms in error source    */
    int	betprod = 1;			/* product nlevels of between factors */
    int	withprod = 1;			/* product nlevels within subj facts  */
    double	sseffect, sserror;	/* sum of squares		      */
    int	dfeffect, dferror;		/* degrees of freedom		      */
    double	mseffect, mserror;	/* mean square			      */
    double	F, p;			/* F ratio and probability	      */
    double	pof ();			/* probability of F ratio	      */


    /* COMPUTE product of nlevels of between/within subjects factors */
    for (factor = 0; factor < nfactors; factor++)
	if (nlevels[factor] <= 1)
	    {
	    fprintf (stderr, "Too few levels of %s\n",
		    factorname[factor]);
	    errorflag = true;
	    }
	else if (between[factor]) betprod *= nlevels[factor];
	else withprod *= nlevels[factor];

    /* PRINT design summary */
    printf ("FACTOR: ");
    for (factor = 0; factor <= nfactors; factor++)
	printf ("%10.10s ", factorname[factor]);
    printf ("\n");
    printf ("LEVELS: ");
    for (factor=0; factor < nfactors; factor++)
	printf ("%10d ", nlevels[factor]);
    printf ("%10d\n", withprod);
    printf ("TYPE  :     RANDOM ");
    for (factor = 1; factor < nfactors; factor++)
	if (between[factor]) printf ("   BETWEEN ");
	else printf ("    WITHIN ");
    printf ("      DATA\n");
    
    /* EXIT if errorflag is set */
    if (errorflag) errorexit ("No F table due to previous fatal error");

    /* PRINT table header */
    printf ("\n");
    printf ("SOURCE                SS     df             MS         F      p\n");
    printf ("===============================================================\n");

    /* if sourcenum is odd, then RANDOM will be in its source,
       so we only want even sourcenums for the main effects */
    for (sourcenum = 0; sourcenum < nsources; sourcenum += 2)
	{
	nterms = setsource (sourcenum, source);

	/* PRINT name of source */
	if (sourcenum == 0) printf ("mean");
	else for (factor = 1; factor < nfactors; factor++)
	    if (source[factor])
		if (nterms == 1)
		    {
		    printf ("%-7.7s", factorname[factor]);
		    break;
		    }
		else	printf ("%c", factorname[factor][0]);
	printf ("\t");

	/* COMPUTE sseffect by adding brackets, alternating signs */
	sseffect = 0.0;
	for (subsource = 0; subsource <= sourcenum; subsource += 2)
	    {
	    nsubs = setsource (subsource, tmpsource);
	    if (subset (tmpsource, source))
		sseffect += ((nterms-nsubs)%2 ? -1.0 : 1.0) * bracket[subsource];
	    }
	if (sseffect < 0.0) sseffect = 0.0;

	/* COMPUTE df for effect */
	dfeffect = 1;
	for (factor = 1; factor < nfactors; factor++)
	    if (source[factor]) dfeffect *= nlevels[factor] - 1;

	/* COMPUTE value of error term */
	/* the error term for a source factor is WxS/B,
	   where W is the set of all within subjects factors
	   IN THE SOURCE, and B is the set of ALL between
	   subject factors in the WHOLE design. S is the
	   subjects or RANDOM factor.  A bracket term is used
	   int the error term if it includes all between subject
	   factors, and if the only other factors it includes are
	   within subject factors or RANDOM.
	*/

	/* set up error source */
	error[RANDOM] = true;
	nerror = 1;
	for (factor = 1; factor < nfactors; factor++)
	    if (between[factor] || (source[factor]))
		{
		error[factor] = true;
		nerror++;
		}
	    else error[factor] = false;

	/* COMPUTE sserror by adding up appropriate brackets */
	sserror = 0.0;
	for (subsource = 0; subsource < nsources; subsource++)
	    {
	    nsubs = setsource (subsource, tmpsource);
	    if (subset (tmpsource, error))
		if (subset (between, tmpsource))
		    sserror += ((nerror-nsubs)%2 ? -1.0 : 1.0) *
				    bracket[subsource];
	    }
	if (sserror < 0.0) sserror = 0.0;

	/* COMPUTE df for error term */
	dferror = nlevels[RANDOM] - betprod;
	for (factor = 1; factor < nfactors; factor++)
	    if (source[factor] && !between[factor])
		dferror *= nlevels[factor] - 1;

	/* COMPUTE mean squares and F */
	mseffect = sseffect/dfeffect;
	mserror = sserror/dferror;
	if (mserror == 0.0)
	    {
	    F = 0.0;
	    p = 0.0;
	    }
	else
	    {
	    F = mseffect/mserror;
	    p = pof (F, dfeffect, dferror);
	    }

	/* PRINT effect part */
	printf ("%16.4f %6d %14.4f %9.3f %6.3f ",
		sseffect, dfeffect, mseffect, F, p);
	if (p <= 0.05) printf ("*");
	if (p <= 0.01) printf ("*");
	if (p <= 0.001) printf ("*");
	printf ("\n");

	/* PRINT error part */
	for (factor = 1; factor < nfactors; factor++)
		if (error[factor] && !between[factor])
			printf ("%c", factorname[factor][0]);
	printf ("%c/", factorname[RANDOM][0]);
	for (factor = 1; factor < nfactors; factor++)
		if (error[factor] && between[factor])
			printf ("%c", factorname[factor][0]);
	printf ("\t");
	printf ("%16.4f %6d %14.4f\n\n", sserror, dferror, mserror);
	} /* ends for each sourcenum */
    } /* ends anova */
boolean
subset (set1, set2) boolean *set1, *set2;
	{
	register int i;
	for (i = 0; i < nfactors; i++)
		if (set1[i] > set2[i]) return (false);
	return (true);
	}

/*				ERROREXIT				      */
errorexit (message) char *message;
	{
	fprintf (stderr, "anova: %s.\n", message);
	fprintf (stderr, "Try 'man anova' for an explanation.\n");
	unlink (tmpname);
	exit (1);
	} /* ends errorexit */

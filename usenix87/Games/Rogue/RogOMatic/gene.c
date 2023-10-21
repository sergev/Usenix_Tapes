/*
 * gene.c: Rog-O-Matic XIV (CMU) Mon Jan 28 20:51:32 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Initialize and summarize the gene pool
 */

# include <stdio.h>
# include "types.h"
# include "install.h"

int knob[MAXKNOB];
char genelock[100];
char genelog[100];
char genepool[100];

main (argc, argv)
int   argc;
char *argv[];
{ int m=10, init=0, seed=0, version=RV53A, full=0;

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { when 'i':	init++;
	when 'f':	full++;
	when 'm':	m = atoi(*argv+1); SKIPARG;
			printf ("Gene pool size %d.\n", m);
	when 's':	seed = atoi(*argv+1); SKIPARG;
			printf ("Random seed %d.\n", m);
	when 'v':	version = atoi(*argv+1); SKIPARG;
			printf ("Rogue version %d.\n", version);
	otherwise:	quit (1, "Usage: gene [-i] [-msv<value>]\n");
      }
    }
  }

  /* Assign the gene log and pool file names */
  sprintf (genelock, "%s/GeneLock%d", RGMDIR, version);
  sprintf (genelog, "%s/GeneLog%d", RGMDIR, version);
  sprintf (genepool, "%s/GenePool%d", RGMDIR, version);

  critical ();				/* Disable interrupts */
  if (lock_file (genelock, MAXLOCK))
  { if (init) 
    { srand (seed);			/* Set the random number generator */
      openlog (genelog);		/* Open the gene log file */
      initpool (MAXKNOB, m);		/* Random starting point */
      writegenes (genepool);		/* Write out the gene pool */
      closelog ();			/* Close the log file */
    }
    else if (! readgenes (genepool))	/* Read the gene pool */
      quit (1, "Cannot read file '%s'\n", genepool);

   unlock_file (genelock);
  }
  else
    quit (1, "Cannot access file '%s'\n", genepool);

  uncritical ();			/* Re-enable interrupts */
  analyzepool (full);			/* Print a summary */
}

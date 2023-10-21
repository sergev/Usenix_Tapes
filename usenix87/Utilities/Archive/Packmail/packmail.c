/* 
 * packmail.c:  Package a set of files for mailing.
 *
 * Usage: packmail [-nq] [-{host}<value>] files
 *
 * HISTORY:
 * 29-Jan-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Added header file option
 *
 * 25-Jun-83  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Changed first fit bin packing to first fit decreasing.
 *
 * 19-Jun-83  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 */

# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>

# define MAXFILE	512
# define SKIPARG	while (*++(*argv)); --(*argv)
# define max(A,B)	((A)>(B)?(A):(B))

/* Magic numbers for fixed and marginal overhead in bytes */
# define FOVRHD		77
# define HOVRHD		4
# define MOVRHD		37

struct filestruct { int size, bin; char *name; } pfile[MAXFILE];
int nf = 0;

/* 
 * cmpsize: Compare two files by decreasing size. For qsort.
 */

int cmpsize (a, b)
register struct filestruct *a, *b;
{ return (b->size - a->size);
}

/* 
 * cmpbin: Compare two files, by increasing bin and increasing
 * lexicographical name.
 */

int cmpbin (a, b)
register struct filestruct *a, *b;
{ register int result;
  if (result=(a->bin - b->bin)) return (result);
  return (strcmp (a->name, b->name));
}

/* 
 * main: Do the bin packing and file copying.
 */

main (argc, argv)
int argc;
char *argv[];
{ int maxsize=31000, count, bol, total=0, check=0, quick=0;
  register int i, assigned, b, maxb=1, ch, headsize = 0, overhead = 0;
  char *outnam = "pack.out", *headnam = NULL, *title = NULL, *filnam;
  char pname[128], fname[128];
  FILE *infile = NULL, *outfil = NULL, *headfil = NULL;

  /* get the arguments */
  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      {	case 'n': check++; break;
	case 'q': quick++; break;
        case 'h': headnam = *argv+1; SKIPARG; break;
        case 'o': outnam = *argv+1; SKIPARG; break;
        case 's': maxsize = atoi (*argv+1); SKIPARG; break;
        case 't': title = *argv+1; SKIPARG; break;
        default:  fprintf (stderr,
			"Usage: packmail [-nq] [-{host}<value>] files\n");
		  exit (1);
      }
    }
  }

  /* Set The title defaults to the output nane */
  if (!title) title = outnam;

  /*
   * Open the headerfile (if there is one)
   */

  if (headnam && *headnam)
  { headsize = HOVRHD + filesize (headnam) + cnt_extra (headnam, 2);
    total += headsize;
    if ((headfil = fopen (headnam, "r")) == NULL)
    { perror (headnam); exit (1); }
  }

  /* 
   * Locate all files and determine their sizes. Assign each to bin 0,
   * and set 'total' to the total size of all files. The size includes
   * any overhead required by the output packaging, except in quick
   * mode the extra characters prepended to each line are not counted.
   */

  for (i = 0; i < argc; i++, nf++)
  { pfile[nf].size = filesize (pfile[nf].name = argv[i]);
    pfile[nf].size += MOVRHD + 2 * strlen (pfile[nf].name);
    if (!quick) pfile[nf].size += cnt_extra (pfile[nf].name, 1);
    total += pfile[nf].size;
    pfile[nf].bin = 0;
  }

# ifdef DEBUG
  printf ("Header: %7d '%s'\n\n", headsize, headnam);

  for (i=0; i<nf; i++)
    printf ("%6d: %7d '%s'\n", i, pfile[i].size, pfile[i].name);
# endif

  /* 
   * If the total size is larger than maxsize, do bin packing and
   * assign each file to a bin from 1 to n.
   */

  overhead = FOVRHD + 2 * strlen (title) + headsize;

  if (total > maxsize)
  { /* Sort into decreasing order of size. */
    qsort (pfile, nf, sizeof (*pfile), cmpsize);

    /* Loop through bins, assigning each file that still fits */
    b=0; assigned=0;
    while (assigned < nf)
    { count=overhead; b++; maxb=b;
      for (i=0; i<nf; i++)
      { if (pfile[i].bin==0)
        { if (count==overhead || pfile[i].size+count <= maxsize)
	  { pfile[i].bin = b;
	    count += pfile[i].size;
	    assigned++;
	  }
	}
      }
      /* Bin 'b' is now full, indicate its size */
      printf ("File %s.%02d size %d.\n", outnam, b, count);
    }

    /* Now sort files into increasing order of <bin,name> */
    qsort (pfile, nf, sizeof (*pfile), cmpbin);
  }

# ifdef DEBUG
  printf ("\nAfter sorting:\n\n");
  for (i=0; i<nf; i++)
    printf ("[%02d] %7d '%s'\n", pfile[i].bin, pfile[i].size, pfile[i].name);
# endif

    if (check)
    { fprintf (stderr, "Packaging would required %d file%s.\n",
               maxb, (maxb != 1) ? "s" : "");
      exit (0);
    }

  /* 
   * Now loop through each file and copy it into the correct bin. Since
   * the files are sorted by bin, we only open each output file once.
   * Bin 0 is used if all files fit into one bin, otherwise we use bins 1
   * to 'n'. 'b' is the current bin, -1 indicates no output file yet.
   */

  for (b= -1, i=0; i<nf; i++)
  { filnam = pfile[i].name;

    /* Open the next input file for reading */    
    if ((infile = fopen (filnam, "r")) == NULL)
    { perror (filnam); exit (1); }

    /* If file.bin is not the current bin, open a new output file */
    if (b != pfile[i].bin)
    { /* Close the old file */
      if (outfil)
      { fprintf (outfil, "echo 'Part %02d of %s complete.'\n",
		 max (b,1), title);
        fprintf (outfil, "exit\n");
        fclose (outfil);
      }

      /*
       * Build the output name, bin 0 is just the output file name, and
       * bin k, k>0, is outputname.kk.
       */      

      if ((b = pfile[i].bin) > 0)
 	sprintf (fname, "%s.%02d", outnam, b);
      else
	strcpy (fname, outnam);

      if ((outfil = fopen (fname, "w")) == NULL)
      { perror (pname); exit (1); }

      fprintf (outfil, "#!/bin/sh\n");
      
      if (headnam)
      { fprintf (outfil, "#\n");
        
	/* Now copy the header file out with '#' in front */
	rewind (headfil);
	bol = 1;
	while ((ch=getc (headfil)) != EOF)
	{ if (bol) { fprintf (outfil, "# "); bol = 0; }
	  putc (ch, outfil);
	  if (ch == '\n') bol = 1;
	}

	if (!bol) putc ('\n', outfil);

        fprintf (outfil, "#\n");
      }
      
      fprintf (outfil,
               "echo 'Start of %s, part %02d of %02d:'\n",
               title, max (b,1), maxb);
    }

    /* 
     * Now copy the input file to the current bin; create a shell script
     * which will restore the original file with the original name (but
     * with default permission bits, group, and owner).
     */

    printf ("%s: added %s\n", fname, filnam);
    fprintf (outfil, "echo 'x - %s'\n", filnam);
    fprintf (outfil, "sed 's/^X//' > %s << '/'\n", filnam);

    /* Now copy the file out with 'X' in front */
    bol = 1;
    while ((ch=getc (infile)) != EOF)
    { if (bol) { fprintf (outfil, "X"); bol = 0; }
      putc (ch, outfil);
      if (ch == '\n') bol = 1;
    }
    
    if (!bol) putc ('\n', outfil);

    /* Add the trailing EOF marker */
    fprintf (outfil, "/\n");

    /* Close the input file */
    fclose (infile);
  }

  /* Close the remaining bin's file */
  if (outfil)
  { fprintf (outfil, "echo 'Part %02d of %s complete.'\n", 
	     max (b,1), title);
    fprintf (outfil, "exit\n");
    fclose (outfil);
  }

  /* Tell the user how many files the packaging took */
  fprintf (stderr, "Packaging required %d file%s.\n",
           maxb, (maxb > 1) ? "s" : "");
}

/****************************************************************
 * filesize: Determine the number of bytes in a file using stat
 ****************************************************************/

filesize (fname)
char *fname;
{ struct stat sbuf;

  if (stat (fname, &sbuf) != 0) { perror (fname); exit (1); }
  return (sbuf.st_size);
}

/* 
 * cnt_extra: Count the number of extra characters needed to 
 * pack file 'fname'.  That is 'add' characters added to each
 * line, plus 1 if the last line is missing a trailing newline,.
 */

cnt_extra (fname, add)
char *fname;
register int add;
{ register int count = 0;
  register FILE *cfil;
  register int c, lastc = '\0';

  if (cfil = fopen (fname, "r"))
  { while ((c = getc (cfil)) != EOF)
    { lastc = c; if (c == '\n') count += add; }
    if (lastc != '\n') count += (add+1);
    fclose (cfil);
  }
  
  return (count);
}

/*
**  SHAR
**  Make a shell archive of a list of files.
**
**  Options:
**	-b		Strip leading directories off names before packing
**	-n #		Set number of this archive in a series
**	-o foo		Output goes to file foo, not stdout
**	-e #		Set ending archive number of series
**	-t "See README"	Set instructions for when all archives are done
*/
#include "shar.h"
RCS("$Header: shar.c,v 1.18 87/03/18 14:10:21 rs Exp $")


/*
**  This prolog is output before the archive.
*/
static char	 *Prolog[] = {
  "! /bin/sh",
  " This is a shell archive.  Remove anything before this line, then unpack",
  " it by saving it into a file and typing \"sh file\".  To overwrite existing",
  " files, type \"sh file -c\".  You can also feed this as standard input via",
  " unshar, or by typing \"sh <file\", e.g..  If this archive is complete, you",
  " will see the following message at the end:",
  NULL
};


/*
**  Package up one file or directory.
*/
static void
shar(file, Basename)
    char		*file;
    int			 Basename;
{
    register char	*s;
    register char	*Name;
    register int	 Bads;
    register int	 Lastchar;
    register off_t	 Size;
    char		 buf[BUFSIZ];

    /* Just in case. */
    if (EQ(file, ".") || EQ(file, ".."))
	return;

    Size = Fsize(file);
    Name =  Basename && (Name = RDX(file, '/')) ? Name + 1 : file;

    /* Making a directory? */
    if (Ftype(file) == F_DIR) {
	printf("if test ! -d %s ; then\n", Name);
	printf("    echo shar: Creating directory \\\"%s\\\"\n", Name);
	printf("    mkdir %s\n", Name);
	printf("fi\n");
    }
    else {
	if (freopen(file, "r", stdin) == NULL) {
	    fprintf(stderr, "Can't open %s, %s\n", file, Ermsg(errno));
	    exit(1);
	}

	/* Emit the per-file prolog. */
	printf("if test -f %s -a \"${1}\" != \"-c\" ; then \n", Name);
	printf("  echo shar: Will not over-write existing file \\\"%s\\\"\n",
	       Name);
	printf("else\n");
	printf("echo shar: Extracting \\\"%s\\\" \\(%ld character%s\\)\n",
	       Name, (long)Size, Size > 1 ? "s" : "");
	printf("sed \"s/^X//\" >%s <<'END_OF_%s'\n", Name, Name);

	/* Output the file contents. */
	for (Bads = 0, Lastchar = '\n'; fgets(buf, BUFSIZ, stdin); ) {
	    printf("X");
	    if (buf[0]) {
		for (s = buf; *s; s++) {
		    if (BADCHAR(*s))
			Bads++;
		    (void)putchar(*s);
		}
		Lastchar = s[-1];
	    }
	    else
		Lastchar = 'X';
	}

	/* Tell about missing \n and control characters. */
	if (Lastchar != '\n')
	    (void)putchar('\n');
	printf("END_OF_%s\n", Name);
	if (Lastchar != '\n') {
	    printf("echo shar: Missing newline added to \\\"%s\\\"\n", Name);
	    fprintf(stderr, "Missing newline added to \"%s\"\n", Name);
	}
	if (Bads) {
	    printf(
	"echo shar: %d control character%s may be missing from \\\"%s\\\"\n",
		   Bads, Bads > 1 ? "s" : "", Name);
	    fprintf(stderr, "Found %d control char%s in \"%s\"\n",
		    Bads, Bads > 1 ? "s" : "", Name);
	}

	/* Output size check. */
	printf("if test %ld -ne `wc -c <%s`; then\n", (long)Size, Name);
	printf("    echo shar: \\\"%s\\\" unpacked with wrong size!\n", Name);
	printf("fi\n");

	/* Executable? */
	if (Fexecute(file))
	    printf("chmod +x %s\n", Name);

	printf("# end of overwriting check\nfi\n");
    }
}


main(ac, av)
    int			 ac;
    register char	*av[];
{
    register char	*Trailer;
    register char	*p;
    register int	 i;
    register int	 length;
    register int	 Oops;
    register int	 Knum;
    register int	 Kmax;
    time_t		 clock;
    int			 Basename;

    /* Parse JCL. */
    Basename = 0;
    Knum = 0;
    Kmax = 0;
    Trailer = NULL;
    for (Oops = 0; (i = getopt(ac, av, "be:n:o:t:")) != EOF; )
	switch (i) {
	    default:
		Oops++;
		break;
	    case 'b':
		Basename++;
		break;
	    case 'e':
		Kmax = atoi(optarg);
		break;
	    case 'n':
		Knum = atoi(optarg);
		break;
	    case 'o':
		if (freopen(optarg, "w", stdout) == NULL) {
		    fprintf(stderr, "Can't open %s for output, %s.\n",
			    optarg, Ermsg(errno));
		    Oops++;
		}
		break;
	    case 't':
		Trailer = optarg;
		break;
	}
    av += optind;
    if (*av == NULL) {
	fprintf(stderr, "No input files\n");
	Oops++;
    }
    if (Oops) {
	fprintf(stderr, "USAGE: shar [-b] [-o:] [-n:e:t:] file... >SHAR\n");
	exit(1);
    }

    /* Everything readable and reasonably-named? */
    for (Oops = 0, i = 0; p = av[i]; i++)
	if (freopen(p, "r", stdin) == NULL) {
	    fprintf(stderr, "Can't read %s, %s.\n", p, Ermsg(errno));
	    Oops++;
	}
	else
	    for (; *p; p++)
		if (!isascii(*p)
		 || (!isalnum(*p) && IDX(OK_CHARS, *p) == NULL)) {
		    fprintf(stderr, "Bad character '%c' in '%s'.\n", *p, av[i]);
		    Oops++;
		}
    if (Oops)
	exit(2);

    /* Prolog. */
    for (i = 0; p = Prolog[i]; i++)
	printf("#%s\n", p);
    if (Knum && Kmax)
	printf("#\t\t\"End of archive %d (of %d).\"\n", Knum, Kmax);
    else
        printf("#\t\t\"End of shell archive.\"\n");
    printf("# Contents: ");
    for (length = 12, i = 0; p = av[i++]; length += strlen(p) + 1)
	if (length + strlen(p) + 1 < WIDTH)
	    printf(" %s", p);
	else {
	    printf("\n#   %s", p);
	    length = 4;
	}
    printf("\n");
    clock = time((time_t *)NULL);
    printf("# Wrapped by %s@%s on %s", User(), Host(), ctime(&clock));
    printf("PATH=/bin:/usr/bin:/usr/ucb ; export PATH\n");

    /* Do it. */
    while (*av)
	shar(*av++, Basename);

    /* Epilog. */
    if (Knum && Kmax) {
	printf("echo shar: End of archive %d \\(of %d\\).\n", Knum, Kmax);
	printf("cp /dev/null ark%disdone\n", Knum);
	printf("MISSING=\"\"\n");
	printf("for I in");
	for (i = 0; i < Kmax; i++)
	    printf(" %d", i + 1);
	printf(" ; do\n");
	printf("    if test ! -f ark${I}isdone ; then\n");
	printf("\tMISSING=\"${MISSING} ${I}\"\n");
	printf("    fi\n");
	printf("done\n");
	printf("if test \"${MISSING}\" = \"\" ; then\n");
	if (Kmax == 2)
	    printf("    echo You have unpacked both archives.\n");
	else
	    printf("    echo You have unpacked all %d archives.\n", Kmax);
	if (Trailer && *Trailer)
	    printf("    echo \"%s\"\n", Trailer);
	printf("    rm -f ark[1-9]isdone%s\n",
	       Kmax >= 9 ? " ark[1-9][0-9]isdone" : "");
	printf("else\n");
	printf("    echo You still need to unpack the following archives:\n");
	printf("    echo \"        \" ${MISSING}\n");
	printf("fi\n");
	printf("##  End of shell archive.\n");
    }
    else
        printf("echo shar: End of shell archive.\n");

    printf("exit 0\n");

    exit(0);
}

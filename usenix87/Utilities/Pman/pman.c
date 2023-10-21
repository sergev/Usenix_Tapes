/*
	contents:
		pman main program to print sections of formatted manuals
	make:
		pman: pman.o getopt.o (putopt.o)
			cc -o pman pman.o getopt.o (putopt.o)
	compiler options:
		-DMONITOR="file"    saves monitor info in file (local only)
		-DCATMANDIR="dir"   looks for manual entries here
	author:
		Gary Perlman     Wang Institute      May 25 1985
		(based on a script by Bob Marcus)
	assumptions:
		1. section headings are on the left margin
		2. section headings do not contain any overstriking
		   as is possible with some versions of nroff
	possible extensions:
	calling pattern: the real workhorse is checklines
		main (argc, argv)
			initial (argc, argv)
				putopt (argc, argv, optstring, file)
				getopt (argc, argv, optstring)
				addsection (optarg)
			checklines (stdin, "-")
				getname (from file text)
			pman (name)
				findfile (section, name)
					printman (name, section, fullpath)
						printheader (name, section, fullpath)
						checklines (ioptr, name)
							getname (from file name)
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/dir.h>

/*
	directory functions/macros for portability between 4.2 BSD
	on older UNIX systems.  syntax ends up being like 4.2 BSD.
*/
#ifdef MAXNAMLEN            /* probably should be BSD4_2 or the like */

#define	namedir(entry)      (entry->d_name)
#define	MAXNAME             256

#else

#define	DIR	                FILE
#define	MAXNAME             (DIRSIZ+2)
#define	opendir(path)       fopen (path, "r")
#define closedir(dirp)      fclose (dirp)
char	*strncpy ();
static	char DiRtNaMe[MAXNAME]; /* name of current dir saved here */
#define	namedir(entry)      strncpy (DiRtNaMe, entry->d_name, DIRSIZ)

struct direct *
readdir (dirp)
DIR 	*dirp;
	{
	static	struct	direct	entry;
	if (dirp == NULL) return (NULL);
	for (;;)
		{
		if (fread (&entry, sizeof (struct direct), 1, dirp) == 0) return (NULL);
		if (entry.d_ino) return (&entry);
		}
	}

#endif

#define	EOS           '\0'
#define	MAXSECTIONS   8      /* sections 1 - 8 in manual */
#define	INDENT        5      /* number of spaces man text is indented */
#define	issection(s)  (s[1] == EOS && *s >= '1' && (*s-'0') <= MAXSECTIONS)
#define	newsection(s) (isupper (s[0]) && isupper (s[1]))

/* PARAMETERS */
#ifndef
#define	CATMANDIR "/usr/catman"
#endif
char	*Catmandir = CATMANDIR;       /* cat files in Catmandir[1-8] */
int 	Section = 0;                  /* manual section number */

/* OPTIONS: print the corresponding section in the UNIX manual */
int 	Header;          /* print a header */
int 	Compact;         /* compact output format */
int 	All;             /* print all occurances of name */
int 	Blanklines;      /* print blank lines */
int 	Name;            /* NAME */
int 	Options;         /* OPTIONS */
int 	Keywords;        /* KEYWORDS NAME SEE ALSO */
int 	Syntax;          /* SYNTAX SYNOPSIS USAGE */
int 	Description;     /* DESCRIPTION */
int 	Variables;       /* VARIABLES FILES ENVIRON */
int 	Warnings;        /* BUGS LIMITATIONS DIAGNOSTICS WARNINGS RESTRICTION */
    	                 /* STATUS */
int 	Examples;        /* EXAMPLES */
int 	Xrefs;           /* SEE ALSO  AUTHOR */

initial (argc, argv) char **argv;
	{
	extern	char	*optarg;
	extern	int 	optind;
	int 	errflg = 0;
	int 	C;
	char	*optstring = "abcdehknosvwx+:";
	int 	popt = 0;        /* have any printing options been selected? */
#define	USAGE "Usage: %s [-abcdehknosvwx] [-+ name] [section] [-] names\n"
#ifdef MONITOR
	putopt (argc, argv, optstring, MONITOR);
#endif
	while ((C = getopt (argc, argv, optstring)) != EOF)
		switch (C)
			{
			default: errflg++; break;
			case 'a': All++;           Header++;  break;
			case 'b': Blanklines++;               break;
			case 'c': Compact++;                  break;
			case 'd': Description++;   popt++;    break;
			case 'e': Examples++;      popt++;    break;
			case 'h': Header++;                   break;
			case 'k': Keywords++;      popt++;    break;
			case 'n': Name++;          popt++;    break;
			case 'o': Options++;       popt++;    break;
			case 's': Syntax++;        popt++;    break;
			case 'v': Variables++;     popt++;    break;
			case 'w': Warnings++;      popt++;    break;
			case 'x': Xrefs++;         popt++;    break;
			case '+':
				if (addsection (optarg)) popt++;
				else
					{
					fprintf (stderr, "%s: Can't add '%s'.  ", argv[0], optarg);
					fprintf (stderr, "(Too many sections)\n");
					errflg++;
					}
				break;
			}
	if (issection (argv[optind]))
		{
		Section = argv[optind][0] - '0';
		optind++;
		}
	if (optind == argc)
		{
		fprintf (stderr, "%s: no names supplied\n", argv[0]);
		errflg++;
		}
	if (errflg)
		{
		fprintf (stderr, USAGE, argv[0]);
		exit (1);
		}
	if (popt == 0) /* no printing options selected */
		Name = Syntax = Examples = 1;
	}

main (argc, argv) char **argv;
	{
	extern	int 	optind;
	char	*name;
	initial (argc, argv);
	while (optind < argc)
		{
		name = argv[optind++];
		if (!strcmp (name, "-")) /* process stdin */
			checklines (stdin, name);
		else if (pman (name) == 0)
			{
			fprintf (stderr, "%s: no manual entry found for %s", argv[0], name);
			if (Section)
				fprintf (stderr, " in section %d", Section);
			putc ('\n', stderr);
			}
		}
	exit (0);
	}

printman (name, section, fullpath)
char	*name;
int 	section;
char	*fullpath;
	{
	FILE	*ioptr;
	if (!fullpath || (ioptr = fopen (fullpath, "r")) == NULL)
		return (0);
	if (Header)
		printheader (name, section, fullpath);
	checklines (ioptr, name);
	(void) fclose (ioptr);
	return (1);
	}

/* searches dir, prints, returns success */
findfile (section, name)
int 	section;
char	*name;
	{
	char	fullpath[BUFSIZ];
	char	dir[BUFSIZ];
	DIR 	*dirp;
	int 	success = 0;
	(void) sprintf (dir, "%s%d", Catmandir, section);
	*fullpath = EOS;
	if (dirp = opendir (dir))
		{
		struct	direct	*dp;
		int 	len = strlen (name);
		while (dp = readdir (dirp))
			{
			if (begins (name, namedir(dp))
			&& (namedir(dp)[len] == '.' || namedir(dp)[len] == EOS))
				{
				(void) sprintf (fullpath, "%s/%s", dir, namedir(dp));
				if (printman (name, section, fullpath))
					{
					success++;
					if (!All) break;
					}
				}
			}
		closedir (dirp);
		}
	return (success);
	}

/* print what is requested from name, if possible, and return success */
pman (name)
char	*name;
	{
	int 	section;
	int 	success = 0;
	if (Section)
		return (findfile (Section, name));
	/* check all sections, stop at first hit */
	for (section = 1; section < MAXSECTIONS; section++)
		if (findfile (section, name))
			{
			success++;
			if (!All) break;
			}
	return (success);
	}

/*
	Data structure definition pairing parts of a manual with options
	There are defined parts left while the name is undefined
	The very last part has the printflag set to NULL
*/
typedef struct
	{
	int 	*pflag;    /* if true, then print this section */
	char	*name;     /* the name of the manual part to be printed */
	} PART;
#define	partname(i)  (Part[i].name)
#define	partprint(i) (*(Part[i].pflag))
#define	partsleft(i) (Part[i].name)
#define	lastpart(i)  (Part[i].pflag == NULL)
int 	True = 1;    /* dummy pflag */
PART	Part[] =     /* end with null elements */
	{
	{ &Name,          "NAME"          },
	{ &Keywords,      "NAME"          },
	{ &Keywords,      "KEYWORD"       },
	{ &Keywords,      "SEE ALSO"      },
	{ &Options,       "OPTIONS"       },
	{ &Syntax,        "SYNTAX"        },
	{ &Syntax,        "SYNOPSIS"      },
	{ &Syntax,        "USAGE"         },
	{ &Description,   "DESCRIPTION"   },
	{ &Description,   "NOTE"          },
	{ &Description,   "COMMENT"       },
	{ &Description,   "OPTION"        },
	{ &Description,   "COMMAND"       },
	{ &Variables,     "FILE"          },
	{ &Variables,     "VARIABLE"      },
	{ &Variables,     "ENVIRON"       },
	{ &Examples,      "EXAMPLE"       },
	{ &Examples,      "HINT"          },
	{ &Xrefs,         "SEE ALSO"      },
	{ &Xrefs,         "REFERENCE"     },
	{ &Xrefs,         "ALGORITHM"     },
	{ &Xrefs,         "AUTHOR"        },
	{ &Warnings,      "WARNING"       },
	{ &Warnings,      "STATUS"        },
	{ &Warnings,      "NOTE"          },
	{ &Warnings,      "CAVEAT"        },
	{ &Warnings,      "DIAGNOSTIC"    },
	{ &Warnings,      "BUG"           },
	{ &Warnings,      "RESTRICTION"   },
	{ &Warnings,      "LIMIT"         },
	{ &True,           NULL           }, /* last one must be null */
	{ &True,           NULL           }, /* extras left for + option */
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ &True,           NULL           },
	{ NULL,            NULL           } /* very last nust be NULL NULL */
	};

/*
	checklines reads the lines from its file until exhausted,
	printing the sections requested.  It matches the name part
	in the Part structure and then if that part has been asked
	for, it prints until the next part of the manual entry.
	checklines skips many unwanted lines:
		page numbering lines, blank lines, copyright lines,
		and miscallaneous page header/footer lines
	note: most of this program's time is spent here (esp. in fgets)
*/
checklines (ioptr, name)
FILE	*ioptr;
char	*name;
	{
	char	line[BUFSIZ];
	char	NAME[BUFSIZ];        /* upper case version of name */
	register	int	part;        /* index through all the different parts */
	int 	printing = 0;        /* are we printing the lines in this part? */
	int 	prevblank = 0;       /* was previous line a blank? */
	register char	*ptr;
	getname (name, NAME, ioptr);
	while (fgets (ptr = line, BUFSIZ, ioptr))
		{
		/* set ptr to first non-space character */
		if (isspace (*ptr))           /* most common case first */
			{
			while (isspace (*ptr)) ptr++;
			if (*ptr == EOS)          /* a visually empty line */
				{
				if (Blanklines && printing && !prevblank)
					prevblank = putchar ('\n');
				/* else skip over blank lines */
				continue;
				}
			}
		else if (isdigit (*ptr))      /* page number in column 0 */
			continue;
		else if (begins ("Printed ", ptr))
			continue;                 /* skip over page footer */
		/* assert (!isspace (*ptr)); */
		if (begins (NAME, ptr))  /* just a page break header */
			{
			char	*eptr = ptr;
			if (ptr == line)          /* NAME(#) at line start */
				continue;     
			while (*eptr && !isspace (*eptr)) eptr++; /* go to first space */
			while (isspace (*eptr)) eptr++;
			if (*eptr == EOS)         /* NAME(#) at line end */
				continue;
			}
		else if (begins ("(c) Digital", ptr))
			continue;                 /* skip over DEC copyright lines */
		/* we now have a printable line */
		if (newsection (line)) /* see if lines will be printed */
			{
			printing = 0;
			for (part = 0; partsleft (part); part++)
				if (begins (partname(part), line))
					if (printing = partprint(part))
						break;
					/* else continue to check for -+ option parts */
			}
		if (printing)
			{
			if (Compact)
				{
				if (newsection (line))
					{
					while (*ptr) ptr++;
					while (ptr > line && isspace (*(ptr-1))) ptr--;
					*ptr = EOS;
					printf ("%-.7s", line);
					}
				else
					{
					putchar ('\t');
					if (ptr-line > INDENT) /* only skip indent */
						ptr = line+INDENT; /* might foul up tabstops */
					fputs (ptr, stdout);
					}
				}
			else fputs (line, stdout);
			prevblank = 0; /* previous line not a blank line */
			}
		}
	}

/* add a new section name to the search list and return success */
addsection (name)
char	*name;
	{
	int 	part;
	for (part = 0; partsleft (part); part++); /* get last part */
	if (!lastpart (part))
		{
		partname (part) = name;
		return (1);
		}
	else return (0);
	}

begins (s1, s2) /* true if s1 is the beginning of s2 */
register	char	*s1;
register	char	*s2;
	{
	while (*s1)
		if (*s1++ != *s2++) return (0);
	return (1);
	}

underline (s) /* prints s underlined */
char	*s;
	{
	while (*s)
		{
		if (isprint (*s) && *s != ' ')
			{
			putchar ('_');
			putchar ('\b');
			}
		putchar (*s++);
		}
	}

/* print the name and section (e.g., 1local, 3g) of name */
printheader (name, section, fullpath)
char	*name;       /* base name of man entry */
int 	section;     /* the section number of the entry */
char	*fullpath;   /* the full path name of the entry */
	{
	char	header[BUFSIZ];
	register	char *sptr;  /* eventually, a sub-section pointer */
	for (sptr = fullpath; *sptr; sptr++);
	while (*sptr != '.' && *sptr != '/' && sptr > fullpath) sptr--;
	if (*sptr) sptr++; /* skip over . */
	if (*sptr) sptr++; /* skip over section digit */
	(void) sprintf (header, "SOURCE%c\t%s %d%s\n",
		(Compact ? ' ' : '\n'), name, section, sptr);
	underline (header);
	}

/* get NAME used to search for lines with -man format headers */
getname (name, NAME, ioptr)
char	*name;     /* the base name of the entry */
char	*NAME;     /* eventually, the upper case name with ( suffix */
FILE	*ioptr;    /* man file, needed if input is from stdin */
	{
	char	line[BUFSIZ];
	register	char *ptr;
	extern	char	*strcpy ();
	if (!strcmp (name, "-")) /* input will be from stdin */
		{
		while (fgets (ptr = line, BUFSIZ, ioptr))
			{
			while (isspace (*ptr)) ptr++;
			if (*ptr) /* we have the title */
				{
				name = ptr;
				while (*ptr && !isspace (*ptr) && *ptr != '(') ptr++;
				*ptr = EOS;
				break;
				}
			}
		}
	(void) strcpy (NAME, name); /* copy, map to UC, and add open paren */
	for (ptr = NAME; *ptr; ptr++)
		if (islower (*ptr)) *ptr = toupper (*ptr);
	*ptr++ = '(';
	*ptr = EOS;
	}

/* slice -- split files at lines that match a pattern */
#include <stdio.h>
#include <ctype.h>

#include "opts.h"				/* defines nextstr() etc */

char *progname = "slice";		/* for error messages */
char *pattern = (char *) NULL;	/* reg expr used to split file */
char **format;					/* ptr for format strings */
int  n_format;					/* number of format strings */
char *defaultfmt[] = {DFLTOUTNAME};	/* default format string */
int  filenumber = 0;
int  every_n_lines = 0;			/* split every n lines */
bool exclude = FALSE;			/* exclude matched line from o/p files */
bool split_after = FALSE;		/* split after matched line */

usage(status)
     int status;	/* exit if status != 0 */
{
     fprintf(stderr,"Usage: %s [-f filename] [-a] [-x] [-i<n>] [-m|-s|-n<n>] [-e expression | expression] [format...]\n", progname);
     if (status)
	  exit(status);
}

main(argc, argv)
     char *argv[];
{
     /* split files at points that match a given pattern */
     /* initialise things */
     bool donefiles = FALSE;
     char *buffer;
	 char *infile = (char *) NULL;

     int getnum();		/* does more checking than atoi */
     char *rmpath();    /* removes leading pathname from a filename */

     /* now remove possible leading pathname
      * (e.g. /usr/bin/slice is to report it's errors as slice
      */
     progname = rmpath(argv[0]);


	while (--argc) {
	  if (**++argv == '-') {
		switch(*++*argv) {
			case 'a': {				/* split after pattern */
				split_after = TRUE;
				break;
			}
			case 'e': {				/* pattern (expression) */
				++argv; argc--;
				if (argc==0 || !**argv) {
					error("Pattern after -e missing or null\n");
					usage(1);
				}
				pattern = *argv;
				break;
			}
			case 'm': {				/* mailbox pattern */
				pattern = "^From ";
				break; 
			}
			case 's': {				/* shell pattern */
				pattern = "^#! *\/bin\/sh";
				break; 
			}
			case 'n': {				/* -n n_lines -- split every n lines */
				nextstr(buffer,argc,argv,usage(2));
				every_n_lines = getnum(buffer);
				if (every_n_lines <= 0) {
					error("-n: number must be at least 1\n");
					exit(EXIT_SYNTAX);
				}
				break;
			} 
			case 'f': {
				++argv; argc--;
				if (argc==0 || !**argv) {
					error("Filename after -f missing or null\n");
					usage(1);
				}
				infile = *argv;
				break;
			}				
		    case 'i': {	/* -i initial_number */
				nextstr(buffer,argc,argv,usage(2));
				filenumber = getnum(buffer);
				if (filenumber < 0) {
			    	error("-i must be followed by a positive number\n");
				    exit(EXIT_SYNTAX);
				 }
				filenumber--;	/* needs to be one less to start with */
				break;
		    }
			case 'x': { /* exclude matched lines */
				exclude = TRUE;
				break;
			}
		    default: {
				error("Unknown flag -%c\n", **argv);
				usage(1);
		    }
		}			/* end switch */
	  } else {	
		if (!pattern) pattern = *argv;	/* first non-flag is pattern */
		else break;						/* break while loop */
	  }			/* end if */
     }		/* end while */

	 if (!argc) {
		format = defaultfmt;
		n_format = 1; }
	 else {
		format = argv;
		n_format = argc;
	 }

#ifdef DEBUG
	printf("argc=%d\n",argc);
	printf("format='%s'\n",*format);
	printf("pattern='%s'\n",pattern);
#endif

	 if (!infile) split(stdin, DFLTNAME, pattern);
	 else        fsplit(infile, pattern);

     exit(0);
}

fsplit(name, pat)
     char *name;
     char *pat;
{
     FILE *fd;

     if (!name || !*name) {
	  error("Can't split a file with an empty name\n");
	  usage(2);
     }

     if ( (fd = fopen(name, "r")) == NULL) {
	  error("Can't open %s\n", name);
	  return;
     }

     (void) split(fd, name, pat);

     if (fclose(fd) == EOF) {	/* something's gone wrong */
	  error("Can't close %s -- giving up\n", name);
	  exit(EXIT_RUNERR);
     }
}

char buffer[BUFLEN];

int
split(input, name, pattern)
     FILE *input;		/* fd of input file */
     char *name;		/* input filename */
     char *pattern;		/* pattern used to split file */
{
     /* do the real work here. Oh dear, I don't know how... */
     /* we are always called with an open file. */

#ifndef USG
     extern char *re_comp();     /* compile string into automaton */
     extern int   re_exec();	/* try to match string */
     int reg_status = 0;	/* regular expression status */
     char *errmessage;
#define REMATCH 1
#define RENOMATCH 0
#define REFAULT -1
#else
     extern char *regcmp();	/* compile string into automaton */
     extern char *regex();	/* match string with automaton */
     char *reg_status;		/* regular expression status */
     char *rex;
#endif

     FILE *output = NULL;
     char fnambuf[MAXFILENAMELEN + 2];  /* +1 for null, +1 for overflow */
     int line = 0;

	 if (split_after && exclude) {
	  error("Can't specify both -a and -x\n");
	  usage(2);
	 }

	 if (every_n_lines && exclude) {
	  error("Can't specify both -n and -x\n");
	  usage(2);
	 }

	 if (every_n_lines && split_after) {
	  error("Can't specify both -n and -a\n");
	  usage(2);
	 }

	 if (every_n_lines && pattern) {
	  error("Can't specify both -n and pattern\n");
	  usage(2);
	 }

     if (!every_n_lines && (!pattern || !*pattern)) {
	  error("Can't match an empty pattern\n");
	  usage(2);
     }

#ifndef USG
     if (!every_n_lines && (errmessage = re_comp(pattern)) != NULL) {
	  error("Error in pattern <%s>: %s\n", pattern, errmessage);
	  exit(EXIT_RUNERR);
     }
     /* errmessage is NULL here */
#else
     if (!every_n_lines && (rex = regcmp(pattern,(char *)0)) == NULL) {
	  error("Erron in pattern <%s>\n", pattern);
	  exit(EXIT_RUNERR);
     }
     /* rex is pointer to compiled expression.... */
#endif

     /* the -2 to fgets is because of the null and \n appended */
     while (fgets(buffer, BUFLEN - 2, input) != NULL) {
	  if (!output ||	/* first line */
	     (every_n_lines > 0 && (++line == every_n_lines)) || /* nth line */
	     (!every_n_lines &&
#ifndef USG
	     ((reg_status = re_exec(buffer)) == REMATCH)) ) { /* matches pat */
#else
	     ((reg_status = regex(rex, buffer)) != NULL)) ) { /* matches pat */
#endif
	       /* don't look at 1st line of file, to avoid an infinite */
	       /* recursion... */

			if (output && split_after) {
				fputs(buffer, output);
			}

			if (n_format && mkname(fnambuf, name)) {;
				/* check for output file = input file */
				if (strcmp(fnambuf,name)==0) {
					error("Output file same as input file\n");
					exit(EXIT_RUNERR);
				}
				/* start a new file */
				if (output && output != stdout) {
					if (fclose(output) == EOF) {
						error("Can't close output file\n");
						exit(EXIT_RUNERR);
					}
					output = NULL;
				}
				line = 0;
				if (fnambuf[0]=='+' && fnambuf[1]==NULL) {
					output = stdout;
				} else {
					if ((output = fopen(fnambuf, "a")) == NULL) {
						error("Can't open output file %s\n", fnambuf);
						exit(EXIT_RUNERR);
					}
				}
				/* if matched lines are excluded, skip the fputs */
#ifndef USG
				if (exclude && reg_status == REMATCH) continue;
#else
				if (exclude && reg_status != NULL) continue;
#endif

				/* if file is to be split after pattern, put already done */
#ifndef USG
				if (split_after && reg_status == REMATCH) continue;
#else
				if (split_after && reg_status != NULL) continue;
#endif
			} else {
				error("Insufficient formats -- last file contains remainder\n");
				}
#ifndef USG
	  } else if (reg_status == REFAULT) {
	       /* the re_exec failed */
	       error("Internal error trying to match <%s> to <%s>\n",
			      pattern, buffer);
	       exit(EXIT_INTERN);
#endif
	  }
	  fputs(buffer, output);
      }
      return (filenumber == -1);	/* exit status for main */
}

bool
mkname(fnambuf, name)
	 char *fnambuf;
	 char *name;
{
     int i, s = -1, d = -1;
	 static bool new_format = TRUE;
	 static bool perpetual = FALSE;
	 static bool d_before_s = FALSE;

	 if (new_format) {
		 if (!n_format) {
			error("Internal error: mkname called but formats have run out\n");
			exit(EXIT_INTERN);
		 }
	     i = bfsearch(*format, "%",0);
	     s = bfsearch(*format, "%s",0);
	     if (i>=0 && i==s) d = bfsearch(*format, "%",++i);
	     else 			   d = i;
		 if (d<0) perpetual = FALSE;
		 else     perpetual = TRUE;
		 if (d<s || s<0) d_before_s = TRUE;
		 else            d_before_s = FALSE;
		 new_format = FALSE;
	 }

	 if (perpetual) ++filenumber;

     if (d_before_s)
          sprintf(fnambuf, *format, filenumber, rmpath(name));
     else 
          sprintf(fnambuf, *format, rmpath(name), filenumber);
      
	 if (!perpetual) {
		new_format = TRUE;
		--n_format;
		if (n_format) {
			++format; 
			filenumber=0;
		}
	}
}

error(fmt, a1, a2, a3, a4)
     char *fmt;
{
     fputs(progname, stderr);
     fputs(": ", stderr);
     fprintf(stderr, fmt, a1, a2, a3, a4);
}

/* getnum(s) returns the value of the unsigned int in s.  If there's any
 * trailing garbage, or the number isn't +ve, we return -1
 */
int
getnum(s)
     char *s;
{
     register char *p;

     for (p = s; *p; p++) {
	  if (!isdigit(*p)) {
	       return -1;
	  }
     }
     return atoi(s);
}


/* Remove the leading pathname from a filename */

char *
rmpath(fullname)
    char *fullname;
{
    register char *p;
    char *q = (char *) NULL;

    for (p = fullname; p && *p; p++) {
         if (*p == '/')
  	    q = ++p;
    }
    if (q && *q) {
         return(q);
    }
    return(fullname);
}


/* Find substring within string */
/* Brute force algorithm */

int 
bfsearch(string,key,start)

  char  string[],
	key[];
  int   start;
{
	int i=start,j=0;

	if (string[0]==NULL || key[0]==NULL) return(-1);

	do {
	  if (string[i] == key[j])
	    {i++; j++;}
	  else
	    {i=i-j+1; j=0;};
	}
	while (string[i]!=NULL && key[j]!=NULL);

	if (key[j]==NULL) return(i-j);
	  else return(-1);
}

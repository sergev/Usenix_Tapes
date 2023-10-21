/* mailsplit -- split files at lines that match a pattern */
#include <stdio.h>
#include <ctype.h>

#include "opts.h"  /* defines nextstr() etc */

char *progname = "filesplit";	/* for error messages */
char *pattern = DFLTPAT;
char *outformat = DFLTOUTNAME;
int filenumber = -1;
int every_n_lines = 0;	/* split every n lines if set -- overrides pattern */

usage(status)
     int status;	/* exit if status != 0 */
{
     fprintf(stderr,"Usage: %s [-i n] [-o fmt] [-p pat] [-n n] [file...]\n", progname);
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

     int getnum();	/* does more checking than atoi */

     progname = argv[0];

     /* now remove possible leading pathname
      * (e.g. /usr/bin/mailsplit is to report it's errors as mailsplit
      */
     {
	  register char *p;
	  char *q = (char *) NULL;

	  for (p = progname; p && *p; p++) {
	       if (*p == '/')
		    q = p;
	  }
	  if (q && *q) {
	       progname = q;
	  }
     }


     while (--argc) {
	  if (**++argv == '-') {
	       switch(*++*argv) {
		    case 'p': {		/* -p pattern */
			 nextstr(pattern,argc,argv,usage(2));
			 break;
		    }
		    case 'o': {	/* -o pattern_for_output_filenames */
			 nextstr(outformat,argc,argv,usage(2));
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
		    case 'n': {	/* -n n_lines --- split every n lines */
			 nextstr(buffer,argc,argv,usage(2));
			 every_n_lines = getnum(buffer);
			 if (every_n_lines <= 0) {
			      error("-n: number must be at least 1\n");
			      exit(EXIT_SYNTAX);
			 }
			 break;
		    }

		    default: {
			 fprintf(stderr, "Unknown flag -%c\n", **argv);
			 usage(1);
		    }
	       }
	  } else {	/* not a "-" flag */
	       fsplit(*argv, pattern);
	       donefiles++;
	  }
     }

     if (!donefiles) {
	  split(stdin, DFLTNAME, pattern);
     }

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
     FILE *input;
     char *name;
     char *pattern;
{
     /* do the real work here. Oh dear, I don't know how... */
     /* we are always called with an open file. */

     extern char *re_comp();	/* compile string into automaton */
     extern int re_exec();	/* try to match string */
#define REMATCH 1
#define RENOMATCH 0
#define REFAULT -1

     char *errmessage;
     FILE *output = NULL;
     char fnambuf[MAXFILENAMELEN + 2];  /* +1 for null, +1 for overflow */
     int reg_status = 0;	/* regular expression status */
     int line = 0;

     if (index(outformat, '%') == NULL) {
	  error("Output filename format (\"%s\") must contain %%\n",outformat);
	  usage(2);
     }
     if (!pattern || (!*pattern && !every_n_lines)) {
	  error("Can't match an empty pattern\n");
	  usage(2);
     }

     if (!every_n_lines && (errmessage = re_comp(pattern)) != NULL) {
	  error("Error in pattern <%s>: %s\n", pattern, errmessage);
	  exit(EXIT_RUNERR);
     }
     /* errmessage is NULL here */


     /* the -2 to fgets is because of the null and \n appended */
     while (fgets(buffer, BUFLEN - 2, input) != NULL) {
	  if (!output ||	/* first line */
	     (every_n_lines > 0 && (++line == every_n_lines)) || /* nth line */
	     (!every_n_lines &&
	     ((reg_status = re_exec(buffer)) == REMATCH)) ) { /* matches pat */
	       /* don't look at 1st line of file, to avoid an infinite */
	       /* recursion... */

	       /* start a new file */
	       if (output) {
		    if (fclose(output) == EOF) {
			 error("Can't close output file \"%s\"\n", fnambuf);
			 exit(EXIT_RUNERR);
		    }
		    output = NULL;
	       }
	       line = 0;
	       sprintf(fnambuf, outformat, ++filenumber, name);
	       if ((output = fopen(fnambuf, "w")) == NULL) {
		    error("Can't open output file %s\n", fnambuf);
		    exit(EXIT_RUNERR);
	       }
	  } else if (reg_status == REFAULT) {
	       /* the re_exec failed */
	       error("Internal error trying to match <%s> to <%s>\n",
			      pattern, buffer);
	       exit(EXIT_INTERN);
	  }
	  fputs(buffer, output);
     }
     return (filenumber == -1);	/* exit status for main */
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

/*	
 *	unshar	-- undo a shell archive file
 *	(C) Copyright June 4 1987 by Craig Norborg
 *	Permission given to use this code in any form as long as it is
 *	not sold or marketed in any form without the written permission
 *	of its author.  Removal of this copyright notice is expressly
 *	forbidden as well as any alteration of it.
 */
/*
 *	Here is a small unshar program written to be as portable as possible.
 *	It was written under Aztec C on an Amiga and tested on a VAX 11/780,
 *	pdp11/70 and a Sequent Balance 21000.  Any bug reports or enhancements
 *	should be reported to the author.  Some further enhancements may
 *	include the correct handling of sub-directories and the handling of
 *	btoa/atob type shars.  If you find a type of shar that this code
 *	does not work on, please send it to me, doc@j.cc.purdue.edu.
 */

#include <stdio.h>
#include <ctype.h>

#define BUFSIZE 512		/* Max length of an input line */
#define STRLEN	25		/* Max length of a file name or delimiter */
#define CAT	"cat"		/* The name of the 'cat' program */
#define SED	"sed"		/* The name of the 'sed' program */
#define WCOUNT	"if test"	/* Leader for 'wc' testing */

/*
 * This is a small routine that given a 'cat' or 'sed' string, will pull out
 * the end of file string and the file name 
 */
void
getendfile(line, end, file)
char   *line,			/* The 'sed' or 'cat' string */
       *end,			/* Place to store the end of file marker */
       *file;			/* Place for the filename */
{
	char   *tmp, *rindex();
	register int i = 0;

	/*
	 * This section of code finds the end of file string.  It assumes
	 * that the eof string is the string of characters immediately
	 * following the last '<' and that it has either a '\' preceding it
	 * or is surrounded by single quotes. 
	 */
	tmp = (char *) rindex(line, '<');	/* Find the last '<' on the
											 * line */
	while (isspace(*++tmp))
		;	/* Do nothing */
	if ('\'' == *tmp) {						/* Is it a quoted string? */
		while ('\'' != *++tmp)
			end[i++] = *tmp;
		tmp++;
	} else if ('\\' == *tmp) {				/* Is it a backslashed string? */
		while (!isspace(*++tmp))
			end[i++] = *tmp;
	} else {								/* Uh-oh! */
		(void) printf("Can't find eof string\n");
		exit(2);
	}
	end[i] = '\0';		/* Null off the end of the string */

	/*
	 * This section of code finds the name of the file.  It assumes that
	 * the name of the file is the string immediately following the last
	 * '>' in the line 
	 */
	i = 0;
	tmp = (char *) rindex(line, '>');
	while (isspace(*++tmp))
		;	/* Do Nothing */
	while (!isspace(*tmp)) {
		file[i++] = *tmp;
		tmp++;
	}
	file[i] = '\0';		/* Null off the end of the string */

#ifdef DEBUG
	(void) printf("EOF = %s, FILE = %s\n", end, file);
#endif DEBUG
}

int
main(argc, argv)
int     argc;
char  **argv;
{
	FILE   *fp, *dfp, *fopen();	/* input file pointer and dest file
					 * pointer */
	char    buf[BUFSIZE],	/* line buffer */
	        prefix[STRLEN],	/* SED leader if any */
	        endstring[STRLEN],	/* EOF marker */
	        filename[STRLEN];	/* file name */
	int     infile = 0,	/* var to tell if we're in the middle of a
				 * file or not */
	        wc = 0;		/* variable to keep a word count */

	if (1 == argc) {	/* check usage */
		(void) printf("usage: unshar <file>");
	}
	if (NULL == (fp = fopen(argv[1], "r"))) {
		(void) printf("Error opening input file\n");
		exit(1);
	}
	while (NULL != fgets(buf, BUFSIZE, fp)) {	/* while there are lines
							 * to get */
#ifdef DEBUG
		puts(buf);
#endif DEBUG

		if (0 == infile) {	/* if we are not in the middle of a
					 * file */
			if ('#' == buf[0])	/* comment? */
				continue;

			/* Is this a CAT type shar? */
			if (0 == strncmp(buf, CAT, strlen(CAT))) {
				prefix[0] = '\0';
				getendfile(buf, endstring, filename);
				if (NULL == (dfp = fopen(filename, "w"))) {
					(void) printf("Error opening output file %s\n", filename);
					exit(1);
				}
				(void) printf("Extracting %s ... ", filename);
				(void) fflush(stdout);
				infile = 1;
				wc = 0;
				continue;
			}
			/* Is it a SED type shar? */
			if (0 == strncmp(buf, SED, strlen(SED))) {
				register int i = 0, j = 0;

				while ('^' != buf[i++])
					;
				while ('/' != buf[i]) {
					prefix[j++] = buf[i++];
				}
				prefix[j] = '\0';
				getendfile(&buf[i], endstring, filename);
				if (NULL == (dfp = fopen(filename, "w"))) {
					(void) printf("Error opening output file %s\n", filename);
					exit(1);
				}
				(void) printf("Extracting %s ... ", filename);
				(void) fflush(stdout);
				infile = 1;
				wc = 0;
				continue;
			}
			/* Did we want to do a word count on that last file? */
			if (0 == strncmp(buf, WCOUNT, strlen(WCOUNT))) {
				register int i = 0;

				while(!isdigit(buf[i]))
					i++;
				if (wc != atoi(&buf[i]))
					(void) printf("Error unsharing %s (wc should have been %d, but was %d)\n", filename, atoi(&buf[i]), wc);
				continue;
			}
		} else {	/* We are in the middle of a file */

			/* Are we at the end of this one? */
			if (0 == strncmp(buf, endstring, strlen(endstring))) {
				(void) printf("Done\n");
				(void) fclose(dfp);
				infile = 0;
				continue;
			}
			/* No, then does it have a prefix? */
			if ('\0' == prefix[0]) {
				fputs(buf, dfp);
				wc = wc + strlen(buf);
			} else {

				/*
				 * If it does have a prefix, is there one on
				 * this line? 
				 */
				if (0 != strncmp(buf, prefix, strlen(prefix))) {
					fputs(buf, dfp);
				} else {
					fputs(&buf[strlen(prefix)], dfp);
					wc = wc + strlen(buf) - strlen(prefix);
				}
			}
		}
	}
	(void) printf("All Done!\n");
	(void) fclose(fp);
}

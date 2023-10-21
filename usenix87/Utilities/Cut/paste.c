/*
 * A version of paste. This is compatable with AT&T paste command SVR2.
 *
 * John Weald
 *
 */
#include <stdio.h>

#define MAXLINE   1024			/* Max. allowed line length	*/
#define MAXFILES  12			/* Max. number of input files   */

extern void exit();

main(argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;

	int c;				/* For getopt()			*/
	char conchars[MAXFILES];	/* The concatination characters */
	int nconchars = 1;		/* The number of conchars[]	*/
	int serial = 0;			/* True if old type paste "-s"	*/

	conchars[0] = '\t';

	while ((c = getopt(argc, argv, "sd:")) != EOF)
	{
		switch(c)
		{
		   case 's':
			/* Concatinate the same file serially */
			serial++;
			break;

		   case 'd':
			/* Use other than a single tab */
			nconchars = setconcat(conchars, optarg);
			break;

		   default:
			/* Does not return */
			prusage();
		}
	}


	if (serial)
		spaste(&argv[optind], conchars, nconchars);
	else
		paste(&argv[optind], conchars, nconchars);
	exit(0);
/* NOTREACHED */
}



/*
 * paste()
 *
 * Do the actual paste.
 */
paste(files, con, ncons)
char *files[];		/* Null terminated list of input files		*/
char con[];		/* The concatination characters			*/
char ncons;		/* The number of above				*/
{
	char ibuf[MAXLINE+1];		/* The  input buffer		*/
	char obuf[MAXLINE];		/* The  output buffer		*/
	register char *iptr = ibuf;
	register char *optr = obuf;
	FILE *fps[MAXFILES];		/* One for each open file 	*/
	int f;				/* Number of files opened	*/
	int allfiles;			/* Ditto			*/
	int inc;			/* True if concat. char. == '\0'*/
	int ocount;			/* Output buffer char. count	*/
	char c;				/* The current concat. char	*/
	int i;

	/*
	 * Open all the input files, any filename of '-' means
	 * the standard input. No file name means standard input.
	 */
	for (f = 0; files[f] != (char *)NULL; f++)
	{
		if (*files[f] == '-')
			fps[f] = stdin;
		else if ((fps[f] = fopen(files[f], "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "Failed to open file %s\n", files[f]);
			exit(1);
		}
		if (f >= MAXFILES)
		{
			fprintf(stderr, 
			   "Too many files. Maximum allowed is %d\n", MAXFILES);
			exit(1);
		}
	} 
	if (files[0] == (char *)NULL)
	{
		fps[0] = stdin;
		f++;
	}

	/*
	 * Read all lines until no more lines in any file.
	 */
	allfiles = f;
	while (f)
	{
		optr = obuf;
		ocount = 0;
		/*
		 * Join lines from all files.
		 *
		 * The concatination character may be '\0' which
		 * means no character. The variable inc is an indication
		 * of the concatination character being '\0', we need to
		 * if there is a concatination character to move up the
		 * output buffer.
		 *
		 * The concatination characters are used in a round robin
		 * list.
		 */

		for (inc = 0, i = 0; i < allfiles; i++)
		{
			iptr = ibuf;
			optr += inc;
			/* To save repeated evaluation */
			c = con[i % ncons];
			inc = c == '\0' ? 0 : 1;

			if (fps[i] == (FILE *)NULL)
			{
				/* No more lines in this file. */

				*optr = c;
				continue;
			}
			if (fgets(ibuf, sizeof(ibuf), fps[i]) == (char *)NULL)
			{
				/* Reached EOF - finished with the file */
				(void)fclose(fps[i]);
				fps[i] = (FILE *)NULL;
				*optr = c;
				f--;
				continue;
			}

			/*
			 * Replace the newline with the concatination character.
			 * There is no need to look for end-of-string since
			 * we know that 
			 * a) if ibuf is full to the max, then we will
			 *    overflow obuf before we hit the end of ibuf.
			 * b) if ibuf is not full, then it must contain a
			 *    a newline character, but may or may not  
			 *    fit into obuf.
			 */
			for (;  *iptr != '\n'; ocount++)
			{
				/* Need space for trailing null */
				if (ocount >= sizeof(obuf) - 1)
				{
					fprintf(stderr, "Output line too long, maximum length is %d.\n", MAXLINE);
					exit(1);
				}
				*optr++ = *iptr++;
			}
			*optr = c;

		}
		if (f)
		{
			*optr = '\0';
			puts(obuf);
		}
	}
}


/*
 * setconcat()
 *
 * Parse the concatination characters and place them in the array c.
 * Return the number of concatination characters.
 *
 * Specials are:
 *	\n	- Newline
 *	\t	- Tab (default)
 *	\	- Backslash
 *	\0	- No concatination character
 */
static int
setconcat(c, in)
char *c;
char *in;
{
	int i;		/* The number seen so far			*/

	for (i = 0; *in != '\0'; in++, c++, i++)
	{
		if (i > MAXFILES)
		{
			fprintf(stderr, "Too many concatination characters, maximum allowed is %d\n", MAXFILES);
			exit(1);
		}
		if (*in != '\\')
		{
			*c = *in;
			continue;
		}
		in++;
		switch (*in)
		{
		   case 'n':
			*c = '\n';
			break;
		   case 't':
			*c = '\t';
			break;
		   case '0':
			*c = '\0';
			break;
		   default:
			/* Includes '\\' */
			*c = *in;
			break;
		}
	}
	return(i);
}

static 
prusage()
{
	fprintf(stderr, "USAGE: paste [-s] [-d<list>] files\n");
	exit(1);
}

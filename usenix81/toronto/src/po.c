/* po - filter to add page offset and possibly cut mark to text */
/* John Hogg, U of Toronto, 26 January 1981 */
/* A rewrite of a program by Henry Spencer, 1978 */

#include <stdio.h>

#define linemax 512
#define TRUE 1
#define FALSE 0

char linebuf[linemax];

int offset1;
int cutbar;
int offset2;


main(argc, argv)
	int argc;
	char **argv;
	{
	FILE *fp;
	char *scan;

	/* Defaults, for nroff default 65-char text line. */
	offset1 = 10;
	cutbar = TRUE;
	offset2 = 10;   /* One-inch margin gives centering on 8-1/2. */

	if(argc > 1 && (*++argv)[0] == '-')
		{
		--argc;
		if((*argv)[1] == 's')
			{
			if((*argv)[2] != '\0')
				usage(); /* and exit */
			/* Stupid system documentation uses nonstandard */
			/*  line length. */
			offset1 = 10;
			cutbar = TRUE;
			offset2 = 12;
			}
		else
			{
			scan = *argv + 1;
			offset1 = atoi(scan);
			while(*scan != '\0' && *scan != '.')
				{
				if(*scan < '0' || *scan++ > '9')
					usage();
				}
			if(*scan == '\0')
				{
				cutbar = FALSE;
				offset2 = 0;
				}
			else
				{
				cutbar = TRUE;
				offset2 = atoi(++scan);
				while(*scan)
					if(*scan < '0' || *scan++ > '9')
						usage();

				}
			}
		argv++;
		}

	if(argc == 1)
		doit(stdin);
	else
		{
		while(--argc > 0)
			{
			if((fp = fopen(*argv++, "r")) == NULL)
				{
				fprintf(stderr, "Cannot open %s\n", *(argv-1));
				exit(-2);
				}
			doit(fp);
			fclose(fp);
			}
		}
	}

doit(fp)
	FILE *fp;
	{
	char *cp;

	cp = linebuf-1;
	while((*++cp = getc(fp)) != EOF)
		{
		if(*cp == '\n' || *cp == '\r')
			{
			*++cp = '\0';
			putline();
			cp = linebuf-1;
			}
		}
	if(cp != linebuf) /* last line not terminated with \n or \r */
		{
		*cp = '\0';
		putline();
		}
	fflush(fp);
	}

putline()
	{
	int count;

	for(count = 0; count < offset1; count++)
		putc(' ', stdout);
	if(cutbar)
		putc('|', stdout);
	for(count = 0; count < offset2; count++)
		putc(' ', stdout);

	fprintf(stdout, "%s", linebuf);
	}

usage()
	{
	fprintf(stderr, "Usage: po [-x[.y]] [-s] [file ...]\n");
	exit(-1);
	}

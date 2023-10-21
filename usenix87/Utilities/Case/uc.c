/* uc.c: change to upper (or lower) case
   Joel West 11/9/85 <ihnp4!gould9!joel, gould9!joel@NOSC.arpa>
*/

#include <stdio.h>
#include <ctype.h>

int optl=0;

main(argc,argv)
int argc;
char **argv;
{	char *p,c;
	int i, up2low, fileargs;

	up2low = 'a'-'A';

	i=1;

	for (i=1; i<argc; i++)
	{   if (*argv[i] != '-')
		break;
	    p = argv[i];
	    if (*++p == 'l')
		optl++;
	    else
	    {	fprintf(stderr, "usage: %s [-l] [file1 file2 ...]\n", argv[0]);
		exit (1);
	    }
	}

	fileargs = i<argc;	/* there are file arguments */
	if (! fileargs)		/* minimum one trip for stdin */
	    i = argc-1;
	for ( ; i<argc; i++)	
	{   if (fileargs)
	    {   if (freopen(argv[i], "r", stdin) == NULL)
		{   perror(argv[i]);
		    continue;
		}
	    }
	    while ((c=getchar()) != EOF)
	    {	if (optl)
		{   if (isupper(c))
			c += up2low;
		}
		else
		{   if (islower(c))
			c -= up2low;
		}
		putchar(c);
	    }
	}
}

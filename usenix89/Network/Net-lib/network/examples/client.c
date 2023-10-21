#ifndef lint
static char     rcsid[] = "$Header$";
#endif

#include <stdio.h>
#include <ctype.h>

typedef enum { false = 0, true = 1} bool; 

char           *progname;

int             p_val;
char           *h_string;
main (argc, argv)
	int             argc;
	char           *argv[];
{
	extern int      opterr;
	extern char    *optarg;
	int             c;
	opterr = 1;
	progname = argv[0];

	p_val = 2500;
	h_string = (char *) 0;

	while ((c = getopt (argc, argv, "p:h:")) != EOF)
		switch (c)
		{
		case 'p':
			p_val = atoi (optarg);
			break;
		case 'h':
			h_string = optarg;
			break;
		case '?':
			usage ();
			break;
		}

	do_prog ();
	exit (0);
}
usage ()
{
	fprintf (stderr, "Usage: %s [ -p port ] [ -h host ]\n", progname);
	exit (1);
}
do_prog ()
{
	int             p,
		  	n,
	                c;
	char            buff[255];
	if (h_string)
		p = phone (useport (p_val), h_string);
	else
		p = phone (useport (p_val), hostname());

	initfd ();

	addfd (0);

	addfd (p);

	while (1)
	{
		c = block ();

		if (c == 0)
		{
			if (gets (buff) == NULL)
				goto leave;
			write (p, buff, strlen (buff));
		}
		else
		{
			n = read (c, buff, 255);
			if (n)
				printf ("Strange... the daemon talked back\n");
			else
			{
				printf ("Daemon died\n");
				exit (0);
			}
		}
	}
leave:
	printf ("Quiting...\n");
}

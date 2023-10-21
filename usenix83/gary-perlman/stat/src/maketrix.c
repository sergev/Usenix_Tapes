#include <stdio.h>
main (argc, argv) char **argv;
	{
	int	ncols;
	int	nstrings = 0;
	char	string[BUFSIZ];
	switch (argc)
	    {
	    case 1:
		ncols = 2;
		break;
	    case 2:
		if (!number (argv[1]) || ((ncols = atoi (argv[1])) < 1))
		    eprintf ("%s: %s is not a valid number of columns",
		    	argv[0], argv[1]);
		break;
	    default:
		eprintf ("%s: Only argument is number of columns", argv[0]);
	    }
	checkstdin (argv[0]);
	while (scanf ("%s", string) == 1)
		{
		fputs (string, stdout);
		if (++nstrings == ncols)
			{
			putchar ('\n');
			nstrings = 0;
			}
		else putchar ('\t');
		}
	if (nstrings != 0)
	    eprintf ("%s: Warning, last line does not have %d columns",
		argv[0], ncols);
	}

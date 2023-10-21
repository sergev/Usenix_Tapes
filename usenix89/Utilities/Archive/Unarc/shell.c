/*
**  Stand-alone driver for shell.
*/
#include "shar.h"
RCS("$Header: shell.c,v 1.4 87/03/24 16:19:56 rs Exp $")


extern void	 SetVar();


main(ac, av)
    register int	 ac;
    register char	*av[];
{
    char		*vec[MAX_WORDS];
    char		 buff[MAX_VAR_VALUE];

    if (Interactive = ac == 1) {
	fprintf(stderr, "Testing shell interpreter...\n");
	Input = stdin;
	File = "stdin";
    }
    else {
	if ((Input = fopen(File = av[1], "r")) == NULL)
	    SynErr("UNREADABLE INPUT");
	/* Build the positional parameters. */
	for (ac = 1; av[ac]; ac++) {
	    (void)sprintf(buff, "%d", ac - 1);
	    SetVar(buff, av[ac]);
	}
    }

    /* Read, parse, and execute. */
    while (GetLine(TRUE))
	if (Argify(vec))
	    (void)Exec(vec);

    /* That's it. */
    exit(0);
}

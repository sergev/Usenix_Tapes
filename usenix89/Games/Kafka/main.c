/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * The kafka main function. Usage: kafka [sourcefile] [-v].
 */

#include <stdio.h>

#define USAGE { printf("Usage: kafka [sourcefile] [-v].\n"); exit(1); }

int vflag = 0;
char *sourcefile = NULL;

main(ac, av)
char **av;
{
	int ch;

	for (ch = 1; ch < ac; ch++) {
		if (!strcmp(av[ch], "-v")) {
			vflag++;
			continue;
		}
		if (!strcmp(av[ch], "-t")) {
			/* Do textfile option. (Unimplemented...) */
			ch++;
			continue;
		}
		if (!strcmp(av[ch], "-s")) {
			/* Do specfile option. (Unimplemented...) */
			ch++;
			continue;
		}
		/* Sourcefile. */
		if (sourcefile)
			USAGE;
		sourcefile = av[ch];
	}
	if (!sourcefile) 
		USAGE;

	/* Set up the files, */
	init();

	/* read in the information, */
	yyparse();

	/* and write it out. */
	dstrans();

	/* That's all... */
	exit(0);
}

/* The error handler. This could use work. */

yyerror()
{
	extern int lineno, errp;

	fprintf(stderr, "Syntax error in line %d.\n", lineno);
	fflush(stderr);
	errp = 1;
}


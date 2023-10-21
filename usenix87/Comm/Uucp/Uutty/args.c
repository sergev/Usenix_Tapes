#include "uutty.h"
/*
** Process the command-line arguments.
*/
args(ac,av)
char **av;
{	int  a;

	for (a=1; a<ac; a++) {
		D3("arg %d=\"%s\"",a,av[a]);
		switch (av[a][0]) {
		case '-':
		case '+':
			D6("opt %d=\"%s\"",a,av[a]);
			option(av[a]);
			continue;
		default:
			switch(files++) {
			case 0:		/* Name of serial port */
				device = av[a];
				break;
			default:
				E("Too many args; \"%s\" ignored.",av[a]);
			}
		}
	}
}

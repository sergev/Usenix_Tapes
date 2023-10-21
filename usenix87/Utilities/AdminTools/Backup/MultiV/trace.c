#ifdef DEBUG	/* Only used if DEBUG is defined */
#include <stdio.h>
	int	tron;	/* global = trace on */
	char	tr[160];

strace(file, line, step)
	char	*file, *step;
	int	line;
{
	static int	indent = 0;
	register	i;

	if (*step == '-')
		indent--; 
	if (tron) {
		fprintf(stderr, "%14s: %5d: ", file, line);
		for (i = indent; i--; )
			fprintf(stderr, ".  ");
		fprintf(stderr, "%s\n", step);
	}
	if (*step == '+')
		indent++; 
}
#endif

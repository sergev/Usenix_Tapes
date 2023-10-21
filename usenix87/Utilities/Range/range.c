/*
 * range a to b [by c]
 *	generate for-loop values for Shell
 */

#include <stdio.h>
#include <ctype.h>

long	atol();
void	usage();
long	number();
int	isnumeric();

int
main(argc, argv)
char **argv;
{
	register long a, b, c;

	if (argc < 4 || strcmp(argv[2], "to"))
		usage();
	a = number(argv[1]);
	b = number(argv[3]);
	c = a <= b? 1: -1;
	if (argc > 5) {
		if (strcmp(argv[4], "by"))
			usage();
		c = number(argv[5]);
	}
	if (c < 0)
		for (; a >= b; a += c)
			printf("%ld\n", a);
	else if (c == 0)
		printf("%ld\n", a);
	else
		for (; a <= b; a += c)
			printf("%ld\n", a);
	exit(0);
}

void
usage()
{
	fprintf(stderr, "Usage: range n1 to n2 [by n3]\n");
	exit(1);
}

long
number(s)
char *s;
{
	if (!isnumeric(s)) {
		fprintf(stderr, "range: ill-formed number `%s'\n", s);
		exit(1);
	}
	return(atol(s));
}

int
isnumeric(s)
register char *s;
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '+' || *s == '-')
		s++;
	do {
		if (!isascii(*s) || !isdigit(*s))
			return(0);
	} while (*++s);
	return(1);
}

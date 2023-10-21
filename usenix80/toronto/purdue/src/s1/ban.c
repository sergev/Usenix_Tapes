#
#define ERR 'X'

#include "/usr2/tgi/src/c/CS5X7.font"

char line[256];

main(argc, argv)
char **argv;
{
	register int n;

	if (argc < 2) {
		write(2, "usage: ", 7);
		while (**argv)
			write(2, (*argv)++, 1);
		write(2, " string ...\n", 12);
		exit(-1);
	}

	while (--argc) {
		argv++;
		puts("");
		for (n = 0; n < DEPTH; n++) {
			bloklin(*argv, line, n);
			puts(line);
		}
		puts("\n");
	}
	flush();
}

bloklin(s, buf, row)
char *s, *buf;
int row;
{
	register int w;
	register char *p, c;
	int n;

	p = buf;
	for (n = 0; s[n]; n++) {
		c = s[n] & 0177;
		for (w = 0; w < WIDTH; w++) {
#ifdef CHAR
			*p++ = bit(c, row, w)? CHAR : ' ';
#endif
#ifndef CHAR
			*p++ = bit(c, row, w)? c : ' ';
#endif
		}
		*p++ = ' ';
	}
	*p++ = 0;
}

bit(c, d, w)
char c;
int d, w;
{
	register int shift, offset;
	register char x;

	shift = d * WIDTH + w;
	x = c;
#ifdef UCASE
	if (x > END)
		x =& 0137;
#endif
	if (START <= x && x <= END)
		x =- START;
	else
		x = ERR - START;
	offset = shift / 16 + x * ((DEPTH * WIDTH + 15) / 16);
	shift =% 16;
	return(tbl[offset] >> (15 - shift) & 01);
}

puts(as)
char *as;
{
	register char *p;

	for (p = as; *p; p++);
	write(1, as, p - as);
	write(1, "\n", 1);
}

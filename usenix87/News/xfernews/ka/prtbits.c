/*
 * Display the contents of a file in binary.
 * The bits are printed with the *low* order bit first.
 */

#include <stdio.h>

FILE *in ;

main(argc, argv)
	char **argv ;
	{
	long offset ;
	register c ;
	register i ;

	in = stdin ;
	if (argc > 1)
		in = fopen(argv[1], "r") ;
	if (in == NULL) {
		fprintf(stderr, "Can't open %s\n", argv[1]) ;
		exit(1) ;
	}
	offset = 0 ;
	while ((c = getc(in)) != EOF) {
		if ((offset & 07) != 0)
			putc(' ', stdout) ;
		else if (offset != 0)
			putc('\n', stdout) ;
		for (i = 8 ; --i >= 0 ; ) {
			putc('0' + (c & 01), stdout) ;
			c >>= 1 ;
		}
		offset++ ;
	}
	putc('\n', stdout) ;
}

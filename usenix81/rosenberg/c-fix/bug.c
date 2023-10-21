/*
 * this seems to generate
 * a coding error
 * on some pdp-11 C compilers
 */

int zero = 0;
int selfinv;
int i;
int j;

main()
{
	selfinv = ~(((unsigned)~0) >> 1) ;
#ifdef Whitesmith
	selfinv = 0100000 ;
#endif
	i = selfinv + 0101;	/* negative */
	j = selfinv - 0100;	/* positive */

	printf("selfinv = 0%o\n", selfinv);
	printf("zero = %d\n", zero);
	printf("i = 0%o = %d.\n", i, i);
	printf("j = 0%o = %d.\n", j, j);

	printf(
		"(j-i < 0) == (j-i < zero)\t%s\n",
		((j-i < 0) == (j-i < zero)) ? "true" : "false"
	);
}

/*
 *	csw - read and set the switch register
 */

main(argc, argv)
char *argv[];
{
	if (argc > 1)
		setcsw(atoo(argv[1]));
	printf("SW: %o\n", getcsw());
}


atoo(p)
register char *p;
{
	register int n;

	for (n = 0; *p >= '0' && *p <= '7'; p++) {
		n <<= 3;
		n += *p - '0';
	}
	return(n);
}

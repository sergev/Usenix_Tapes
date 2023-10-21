/*
 * This is just a copy of echo.c which first seeks std output
 * To position -1 before writing out args. The Purdue tty driver
 * sys/dev/tty.c checks for SU and tty file being seeked to pos
 * -1.  If true, then characters are dumped to the tty's input
 * clist (just like he typed them in and they echo also) instead
 * of the output clist.
 * # force ls -l >/dev/tty8   would do an ls -l on tty8 just
 * like somebody typed in the command there (like RSTS force).
 * --ghg
 */
main(argc, argv)
char **argv;
{
	register n;
	register char *p;

	if ((n = argc) < 2)
		exit(-1);
	p = *++argv;
	while (--n) {
		while (*p)
			p++;
		*p = ' ';
	}
	*p++ = '\n';
	seek(1, 0, 0);
	seek(1, -1, 1);
	write(1, *argv, p - *argv);
	exit(0);
}

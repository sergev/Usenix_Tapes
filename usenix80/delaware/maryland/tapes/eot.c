#
/*
 * EOT
 * finds an end-of-tape consisting of two consecutive file marks.
 * A parameter can limit the number of file marks to be passed.
 * A different physical drive can be specified by
 * a parameter with a leading dash.
 * The last digit of the calling program name can also specify the drive.
 * If one of the default drives is used
 * the tape is backed up to just after the first of the two file marks.
 * cc -s eot.c errprnt.c ; mv a.out /usr/bin/eot
 */
#define INTR 2
#define QUIT 3
char *argv0 0;  /* remember calling program name */
int count -1;
int file -1;
int passed;	/* logically already passed file marks */
int len;
char *tname;
char *dname "/dev/nrw_rmt0"; /* Default drive for advancing */
char *bname "/dev/bu_rmt0"; /* Default drive for backing up */
char buff[44544];

stopintr()
{
	pmsg(" interrupt\n");
}

stopquit()
{
	pmsg(" quit\n");
}

pmsg(etype)
char *etype;
{
	printf(argv0);
	if(passed)
		printf(" passes %d file mark%s on %s before", passed,
			(passed == 1 ? "\0" : "s"), tname);
	printf(etype);
	flush();
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	register char **ap;
	register int c;
	register char *cp;

	ap = argv;
	argv0 = *ap;
	close(2); dup(1);
	tname = dname;
	if(signal(INTR, 1) != 1)
		signal(INTR, stopintr);
	if(signal(QUIT, 1) != 1)
		signal(QUIT, stopquit);
	while(argc-- > 0) {
		if((c = **ap) == '-') {
			if((c = *++*ap) == 0) {
				dname[12] = '4';
				tname = dname;
			} else if(*++*ap == 0 && '0' <= c && c <= '7') {
				dname[12] = c;
				tname = dname;
			} else
				tname = --*ap;
		} else if(c == '0') {
			count = 0;
			cp = *ap;
			while('0' <= (c = *++cp) && c <= '7')
				count = c - '0' + count * 8;
			if(count == 0) return;
		} else if((c = atoi(*ap)) != 0)
			count = c;
		else {
			cp = *ap;
			while(*++cp);
			c = *--cp;
			if('0' <= c && c <= '7') {
				dname[12] = c;
				tname = dname;
			} else if(*ap != argv0)
			 errprnt("[ count ] [ -[unit] ]\ninstead of %s", *ap);
		}
		ap++;
	}
	c = count;
	while(c--) {
		if((file = open(tname, 0)) < 0) {
			printf("%s cannot skip on %s", argv0, tname);
			if(++c != count) {
				printf(" with %d file", c);
				if(--c) putchar('s');
				printf(" to go out of %d", count);
			}
			putchar('\n');
			flush();
			exit(1);
		}
		passed++;
		if((len = read(file, buff, sizeof buff)) == -1)
			errprnt("read error with %d file marks to go", ++c);
		if(len == 0) {
			close(file);
			printf("%s found eot after skipping %d file",
				argv0, count - c - 1);
			if((count - c) != 2) putchar('s');
			putchar('\n');
			flush();
			if(tname != dname)
			   errprnt("cannot backup after using %s", tname);
			bname[11] = dname[12];
			if(open(bname, 0) < 0)
				errprnt("error on %s while backing up",
					bname);
			exit(0);
		}
		if(close(file) == -1)
			errprnt("close error on %s", tname);
	}
	errprnt("passed %d file marks", count);
}

#
/*
 * BU
 * backs up tape drive the number of files given by parameter.
 * 1 is used if parameter is not given.
 * A different physical drive can be specified by
 * a parameter with a leading dash.
 * The last digit of the calling program name can also specify the drive.
 * cc -s bu.c errprnt.c ; mv a.out /usr/bin/bu
 */
#define INTR 2
#define QUIT 3
char *argv0 0;  /* remember calling program name */
int count 1;
int file -1;
int passed;	/* logically already passed file marks */
char *bname;
char *dname "/dev/bu_rmt0"; /* Default drive is low density */

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
		printf(" backs up %d file%s on %s before", passed,
			(passed == 1 ? "\0" : "s"), bname);
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
	bname = dname;
	if(signal(INTR, 1) != 1)
		signal(INTR, stopintr);
	if(signal(QUIT, 1) != 1)
		signal(QUIT, stopquit);
	while(argc-- > 0) {
		if((c = **ap) == '-') {
			if((c = *++*ap) == 0) {
				dname[11] = '4';
				bname = dname;
			} else if(*++*ap == 0 && '0' <= c && c <= '7') {
				dname[11] = c;
				bname = dname;
			} else
				bname = --*ap;
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
				dname[11] = c;
				bname = dname;
			} else if(*ap != argv0)
			 errprnt("[ count ] [ -[unit] ]\ninstead of %s", *ap);
		}
		ap++;
	}
	c = count;
	while(c--) {
		passed++;
		if((file = open(bname, 0)) < 0) {
			printf("%s cannot back up %s with ", argv0, bname);
			printf("%d file", 1 + c);
			if(c) putchar('s');
			printf(" to go out of %d\n", count);
			flush();
			exit(1);
		}
		if(close(file) == -1)
			errprnt("close error on %s", bname);
	}
}

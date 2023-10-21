/*
 * REWIND
 * rewinds a magtape back to the beginning of tape
 * trying both the new and old versions of the rewinding tape drives.
 * A different physical drive can be specified by
 * an octal digit, a dash, or a device name.
 * The last digit of the calling program name can also specify the drive.
 * cc -s rewind.c ; mv a.out /usr/bin/rewind
 */
char *argv0 0;  /* remember calling program name */
char *dname "/dev/rw_rmt0"; /* Default drive is low density */
char *bname "/dev/rmt0"; /* second try at rewinding */

main(argc, argv)
int argc;
char *argv[];
{
	register char *tname;
	register int c;
	register char *cp;

	cp = *argv;
	argv0 = cp;
	tname = dname;

	while(*++cp);
	c = *--cp;
	if('0' <= c && c <= '7')
		dname[11] = c;
	if(argc > 1) {
		cp = *++argv;
		if((c = *cp) == '-')
			dname[11] = '4';
		else if(*++cp == 0)
			dname[11] = c;
		else
			tname = --cp;
	}
	/* try opening twice in case of previous i/o errors */
	if(open(tname, 0) < 0 && open(tname, 0) < 0) {
		if(tname != dname)
			printf("%s cannot open %s\n", argv0, tname);
		else {
			bname[8] = dname[11];
			if(open(bname, 0) < 0)
				printf("%s cannot open %s or %s\n",
					argv0, tname, bname);
		}
	}
	flush();
}

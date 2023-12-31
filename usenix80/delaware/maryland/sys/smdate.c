/*
 * smdate mmddhhmm[yy] file ...
 * changes modified date of file using digit
 * string in date format after printing date used.
 * cc -s -O smdate.c tpsm.o; mv a.out smdate
 * Written using date.c by Robert L. Kirby UofMd 12-March-1980.
 */
int	timbuf[2];
char	*cbp;

char *tzname[2];
int	dmsize[];
char	cbuf[];
char	*cbp;

struct {
	char	name[8];
	char    tty;
	char	fill1;
	int	wtime[2];
	int	fill2;
} wtmp[2];

main(argc, argv)
int argc, **argv;
{
	register char *tzn, **ap;
	extern int timezone, *localtime();

	if(--argc < 1) {
		write(1, "smdate mmddhhmm[yy] [file ...]\n", 31);
		exit(1);
	}
	ap = argv;
	cbp = *++ap;
	if(gtime()) {
		write(1, "bad conversion\n", 15);
		exit(1);
	}
	if (*cbp != 's') {
	/* convert to Greenwich time, on assumption of Standard time. */
		dpadd(timbuf, timezone);
	/* Now fix up to local daylight time. */
		if (localtime(timbuf)[8])
			dpadd(timbuf, -1*60*60);
	}
	cbp = cbuf;
	ctime(timbuf);
	write(1, cbuf, 20);
	tzn = tzname[localtime(timbuf)[8]];
	if (tzn)
		write(1, tzn, 3);
	write(1, cbuf+19, 6);
	while(--argc > 0)
		if(smdate(*++ap, timbuf) < 0)
			perror(*ap);
	exit(0);
}

gtime()
{
	register int i;
	register int y, t;
	int d, h, m;
	extern int *localtime();
	int nt[2];

	t = gpair();
	if(t<1 || t>12)
		goto bad;
	d = gpair();
	if(d<1 || d>31)
		goto bad;
	h = gpair();
	if(h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if(m<0 || m>59)
		goto bad;
	y = gpair();
	if (y<0) {
		time(nt);
		y = localtime(nt)[5];
	}
	if (*cbp == 'p')
		h =+ 12;
	if (h<0 || h>23)
		goto bad;
	timbuf[0] = 0;
	timbuf[1] = 0;
	y =+ 1900;
	for(i=1970; i<y; i++)
		gdadd(dysize(i));
	/* Leap year */
	if (dysize(y)==366 && t >= 3)
		gdadd(1);
	while(--t)
		gdadd(dmsize[t-1]);
	gdadd(d-1);
	gmdadd(24, h);
	gmdadd(60, m);
	gmdadd(60, 0);
	return(0);

bad:
	return(1);
}

gdadd(n)
{
	register char *t;

	t = timbuf[1]+n;
	if(t < timbuf[1])
		timbuf[0]++;
	timbuf[1] = t;
}

gmdadd(m, n)
{
	register int t1;

	timbuf[0] =* m;
	t1 = timbuf[1];
	while(--m)
		gdadd(t1);
	gdadd(n);
}

gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

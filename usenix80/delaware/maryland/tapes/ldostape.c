/*
 * LDOSTAPE
 * lists contents of DOS PIP format magtapes.
 * ldostape [ -N ] [ -special ] [ -cN ] [ -u[N] ] [ -g[N] ]
 * cc -s -O ldostape.c errprntc.c -lp -lp ; mv a.out ldostape
 */
char *argv0 0;
int count 0;
int infile;
int total;
char outs[12]; /* area to build ascii name of file */
char ufilf;
char gfilf;
char user;
char group;
char *tname;
char *dname "/dev/nrw_rmt0"; /* Default drive is low density */
char *datestr "%2d-%s-%d\n";
int header[256];
char *mon[] {
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};
int nlpday[ ] { /* number of days in each month */
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};

struct  integ {
	char    lobyte;
	char    hibyte;
};

main(argc, argv)
int argc;
char *argv[];
{
	register char **args;
	register int cnt;
	extern int errno;

	args = argv;
	argv0 = *args++;
	close(2); dup(1);
	tname = dname;
	while(**args == '-' && --argc > 0) {
		if(*++*args == 'c') {
			count = atoi(++*args);
		} else if('0' <= **args && **args < '8' && args[0][1] == 0) {
			dname[12] = **args;
			tname = dname;
		} else if(**args == 'u') {
			if(*++*args == 0) ufilf = 0;
			else {
				ufilf++;
				user = atoi(*args);
			}
		} else if(**args == 'g') {
			if(*++*args == 0) gfilf = 0;
			else {
				gfilf++;
				group = atoi(*args);
			}
		} else {
			tname = *args;
		}
		args++;
	}
	if((infile = open(tname, 0)) < 0)
		errprnt("cannot open %s", tname);
	printf("\nNAME  .EXT    UIC  Protection Date\n");
	while((cnt = read(infile, header, 512)) != 0) {
		if(cnt == -1)
			errprnt("%d physical read error on %s header",
				errno, tname);
		if(cnt < 14)
			errprnt("%d %s read error on %s header",
				cnt, tname);
		if((ufilf == 0 || header[3].hibyte == user) &&
		   (gfilf == 0 || header[3].lobyte == group)) {
			asciiname(header, outs);
			printf(outs);
			printf(" [%03o,%03o] <%03o> ",
				header[3].hibyte & 0377,
				header[3].lobyte & 0377,
				header[4]);
			jdate(header[5]);
			total++;
		}
		if(close(infile) == -1)
			errprnt("close error on %s", tname);
		if(--count == 0) break;
		if((infile = open(tname, 0)) < 0)
			errprnt("cannot reopen %s", tname);
	}
	printf("\n%s found %d files", argv0, total);
	if(count != 0) printf(" before EOT");
	printf(" on %s\n", tname);
	cexit(0);
}

int d50(rad50)
int *rad50;
{
	register int *r50;
	extern ldivr;

	r50 = rad50;
	*r50 = ldiv(0, *r50, 050);
	return(ldivr);
}

asciiname(rad50, ascii)
int *rad50;
char *ascii;
{
	register char *apt;
	register int *r50pt;

	r50pt = rad50; apt = &ascii[2];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	r50pt++;
	apt = &apt[5];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	r50pt++;
	apt = &apt[6];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = '.';
}

int r50toa(cc)
int cc;
{
	register c;

	c = cc;
	if(c < 0) c = -c;
	if(1 <= c && c <= 27) return(c - 1 + 'a');
	if(c == 0) return ' ';
	if(036 <= c && c < 050) return(c - 036 + '0');
	if(c == 033) return('-');
	if(c == 034) return('.');
	return('?'); /* undefined character */
}

jdate (juldays)
int juldays;
{
	register days, year, i;

	year = juldays / 1000;
	days = juldays - (year * 1000);
	year = 1970 + year;
	nlpday[1] = 28;

	if(days == 0) {
		days = 1;
		printf(datestr, days, mon[0], year);
		return;
	}
	if((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0)) {
		if(days == 366)  {
			days = 31;
			printf (datestr, days, mon[11], year);
			return;
		}
		for(i = 0; days > nlpday[i]; ++i)  {
			if ((i + 1) == 1)
				++nlpday[i + 1];
			days = days - nlpday[i];
		}
		printf(datestr, days, mon[i], year);
		return;
	} else {
		if(days == 365) {
			days = 31;
			printf(datestr, days, mon[11], year);
			return;
		}
		for (i = 0; days > nlpday[i]; ++i)
			days = days - nlpday[i];
		printf (datestr, days, mon[i], year);
	}
}

/*
 | Hebrew dates since 2nd century A.D.
 | by Amos Shapir 1978 (rev. 1985)
 | The HEBREW option prints hebrew strings in l.c.
 | flags: -j - input date is Julian (default before 1582 (5342))
 |	  -h - translate hebrew date to general
 */
#include "hdate.h"

int jflg, hflg;
main(argc, argv)
	char **argv;
{
	register m, y, n;
	long time();
	struct hdate *gdate(), *h;
#ifdef HEBREW
	static hs[50];
#endif

	while(argc>=2 && argv[1][0]=='-') {
		switch(argv[1][1]) {
		case 'j':
			jflg++; break;
		case 'h':
			hflg++; break;
		default:
			usage();
		}
		argv++;
		argc--;
	}
	if(argc == 1) {
		/* date changes at 1800 local time */
		n = (time(0)+(6*60-MINWEST)*60)/(24*60*60)+1;
		m = 1;
		y = 1970;
		hflg = 0;
	} else if(argc == 4) {
		y = atoi(argv[3]);
		if(y < 100 && !jflg && !hflg)
			y += 1900;
		if(hflg && y < 5342 || !hflg && y < 1582)
			jflg++;
		m = atoi(argv[2]);
		n = atoi(argv[1]);
	} else
		usage();
	if(hflg) {
		h = gdate(n, m, y);
		printf("%d %s %d", h->hd_day+1, mname[h->hd_mon], h->hd_year);
		if(jflg)
			printf(" (J)");
		printf("\n");
		return 0;
	}
	h = hdate(n, m, y);
#ifdef HEBREW
	strcat(hs, hnum(h->hd_day+1));
	strcat(hs, " ");
	strcat(hs, hmname[h->hd_mon]);
	strcat(hs, " ");
	strcat(hs, hnum(h->hd_year));
	printf("%s\n", rev(hs));
#else
	printf("%d %s %d\n", h->hd_day+1, hmname[h->hd_mon], h->hd_year);
#endif
}
usage()
{
	printf("Usage: hdate [-j] [-h] [day mon year]\n");
	exit(1);
}

/*
 | compute general date structure from hebrew date
 */
struct hdate *
gdate(d, m, y)
	register m, y, d;
{
	static struct hdate h;
	register s;

	y -= 3744;
	s = dysiz(y);
	d += s;
	s = dysiz(y+1)-s;		/* length of year */
	d += (59*(m-1)+1)/2;	/* regular months */
	/* special cases */
	if(s%10>4 && m>2)
		d++;
	if(s%10<4 && m>3)
		d--;
	if(s>365 && m>6)
		d += 30;
	d -= 6002;
	if(!jflg) {	/* compute century */
		y = (d+36525)*4/146097-1;
		d -= y/4*146097+(y&3)*36524;
		y *= 100;
	} else {
		d += 2;
		y = 0;
	}
	/* compute year */
	s = (d+366)*4/1461-1;
	d -= s/4*1461+(s&3)*365;
	y += s;
	/* compute month */
	m = (d+245)*12/367-7;
	d -= m*367/12-30;
	if(++m >= 12) {
		m -= 12;
		y++;
	}
	h.hd_day = d;
	h.hd_mon = m;
	h.hd_year = y;
	return(&h);
}

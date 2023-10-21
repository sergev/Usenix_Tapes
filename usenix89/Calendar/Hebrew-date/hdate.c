/*
 | Hebrew dates since 2nd century A.D.
 | by Amos Shapir 1978 (rev. 1985)
 | The HEBREW option prints hebrew strings in l.c.
 | flags: -j - input date is Julian (default before 1582 (5342))
 |	  -h - translate hebrew date to general
 */

char  *mname[] = {
#ifdef HEBREW
	"zyxi", "gyeo", "kqle",
	"haz", "yah", "`cx",
	"piqo", "`ix", "qieo",
	"znef", "`a", "`lel",
	"`cx `", "`cx a"
#else
	"Tishrei", "Heshvan", "Kislev",
	"Tevet","Shvat", "Adar",
	"Nisan", "Iyar", "Sivan",
	"Tamuz", "Av", "Elul",
	"Adar a", "Adar b"
#endif
};
char  *gmname[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};
struct hd {
	int hd_day;
	int hd_mon;
	int hd_year;
};

#ifdef HEBREW
puth(n)
	register n;
{
	char as[16];
	register char *s;

	s = as;
	if(n >= 1000) {
		puth(n/1000);
		n %= 1000;
	}
	while(n >= 400) {
		*s++ = 'z';
		n -= 400;
	}
	if(n >= 100) {
		*s++ = 'v'+n/100;
		n %= 100;
	}
	if(n >= 10) {
		if(n == 15 || n == 16)
			n -= 9;
		*s++ = "hiklnpqrtv"[n/10];
		n %= 10;
	}
	if(n > 0)
		*s++ = '_'+n;
	/* gershayim */
	if(s == &as[1])
		*s++ = '\'';
	else {
		*s = s[-1];
		s[-1] = '"';
		/* sofyot */
		if(*s=='k' || *s=='n' || *s=='p' || *s=='t' || *s=='v')
			(*s)--;
		++s;
	}
	*s++ = '\0';
	printf(as);
}
#endif

int jflg, hflg;
main(c, v)
	char **v;
{
	register m, y, n;
	long d, time();
	struct tm *localtime(), *t;
	struct hd *hdate(), *gdate(), *h;

	while(c>=2 && v[1][0]=='-') {
		switch(v[1][1]) {
		case 'j':
			jflg++; break;
		case 'h':
			hflg++; break;
		default:
			usage();
		}
		v++;
		c--;
	}
	if(c == 1) {
		/* date changes at 1800 Jerusalem time (1540 GMT) */
		n = (time(0)+500*60)/(24*60*60)+1;
		m = 1;
		y = 1970;
		hflg = 0;
	} else if(c == 4) {
		y = atoi(v[3]);
		if(y < 100 && !jflg && !hflg)
			y += 1900;
		if(hflg && y < 5342 || !hflg && y < 1582)
			jflg++;
		m = atoi(v[2]);
		n = atoi(v[1]);
	} else
		usage();
	if(hflg) {
		h = gdate(n, m, y);
		printf("%d %s %d", h->hd_day+1, gmname[h->hd_mon], h->hd_year);
		if(jflg)
			printf(" (J)");
		printf("\n");
		return 0;
	}
	h = hdate(n, m, y);
#ifdef HEBREW
	puth(h->hd_day+1);
	printf(" %s ", mname[h->hd_mon]);
	puth(h->hd_year);
	printf("\n");
#else
	printf("%d %s %d\n", h->hd_day+1, mname[h->hd_mon], h->hd_year);
#endif
}
usage()
{
	printf("Usage: hdate [-j] [-h] [day mon year]\n");
	exit(1);
}

/*
 | compute date structure from no. of days since 1 Tishrei 3744
 */
/* constants, in 1/18th of minute */
#define HOUR 1080
#define DAY  (24*HOUR)
#define WEEK (7*DAY)
#define M(d,h,p) (d*DAY+h*HOUR+p)
#define MONTH M(29,12,793)
struct hd *
hdate(d, m, y)
	register m, y, d;
{
	static struct hd h;
	register s;

	if((m -= 2) <= 0) {
		m += 12;
		y--;
	}
	/* no. of days, Julian calendar */
	d += 365*y+y/4+367*m/12+5968;
	/* Gregorian calendar */
	if(!jflg)
		d -= y/100-y/400-2;

	/* compute the year */
	m = (double)d*DAY/MONTH;	/* needs ~34 bits */
	y = (m+38)*19/235-3;
	s = dysiz(y);
	d -= s;
	s = dysiz(y+1)-s;
	y += 3744;

	if(d < 0) {	/* computed year was overestimated */
		d += s;
		y--;
	} else if(d >= s) {	/* computed year was underestimated */
		d -= s;
		y++;
	}

	/* compute day and month */
	if(d >= s-236) {	/* last 8 months are regular */
		d -= s-236;
		m = d*2/59;
		d -= (m*59+1)/2;
		m += 4;
		if(s>365 && m<=5)	/* Adar of Meuberet */
			m += 8;
	} else {
		/* 1st 4 months have 117-119 days */
		s = 114+s%10;
		m = d*4/s;
		d -= (m*s+3)/4;
	}

	h.hd_day = d;
	h.hd_mon = m;
	h.hd_year = y;
	return(&h);
}

/*
 | compute general date structure from hebrew date
 */
struct hd *
gdate(d, m, y)
	register m, y, d;
{
	static struct hd h;
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

/* no. of months in y years */
msiz(y)
	register y;
{
	return y*12+(y*7+1)/19;
}

/* no. of days in y years */
dysiz(y)
	register y;
{
	register m, nm, dw, s;
	double nmt;

	m = msiz(y);
	nmt = (double)m*MONTH+M(0,1,779); /* molad at 197 cycles + M(2,5,204) */
	nm = nmt-(int)(nmt/WEEK)*WEEK;	/* actually nmt%WEEK */
	s = (int)(nmt/DAY)-2;
	dw = nm/DAY;
	nm %= DAY;
	/* special cases of Molad Zaken */
	if(nm >= 18*HOUR || msiz(y+1)-m==12 && dw==3 && nm>=M(0,9,204) ||
	 m-msiz(y-1)==13 && dw==2 && nm>=M(0,15,589))
		s++,dw++;
	/* ADU */
	if(dw == 1 || dw == 4 || dw == 6)
		s++;
	return s;
}

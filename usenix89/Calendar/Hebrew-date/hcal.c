/*
 | calendar with hebrew dates
 | by Amos Shapir 1984 (rev. 1985)
 | The HEBREW option prints hebrew strings in l.c.
 | The REV option reverses them before printing.
 | flag: -j - input date is Julian (default before 1582)
 */

int jflg;
main(argc, argv)
	char **argv;
{
	register m, y;

	if(argc>=2 && argv[1][0]=='-' && argv[1][1]=='j') {
		jflg++;
		argv++;
		argc--;
	}
	if(argc == 2)
		y = atoi(argv[1]);
	else if(argc == 3) {
		m = atoi(argv[1]);
		y = atoi(argv[2]);
	} else {
		printf("Usage: hcal [-j] [mon] year\n");
		return (1);
	}
	if(y < 100 && !jflg)
		y += 1900;
	if(y < 1582)
		jflg++;
	if(argc == 2)
		for(m=1; m<=12; m++)
			hcal(m, y);
	else
		hcal(m, y);
	return (0);
}

char	*mname[]= {
#ifdef HEBREW
	"JANUARY", "FEBRUARY", "MARCH", "APRIL",
	"MAY", "JUNE", "JULY", "AUGUST",
	"SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER",
#else
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December",
#endif
};
char  *hmname[] = {
#ifdef HEBREW
	"zyxi", "gyeo", "kqle",
	"haz", "yah", "`cx",
	"piqo", "`ix", "qieo",
	"znef", "`a", "`lel",
	"`cx `", "`cx a"
#else
	"Tishrey", "Heshvan", "Kislev",
	"Tevet","Shvat", "Adar",
	"Nisan", "Iyar", "Sivan",
	"Tamuz", "Av", "Elul",
	"Adar a", "Adar b"
#endif
};

/* bit array of holidays */
long hagim[] = {
	0x408406, 0, 0, 0, 0,
	0, 0x208000, 0, 0x40, 0,
	0, 0, 0, 0
},
moadim[] = {
	0x3F0000, 0, 0x7E000000, 0x406, 0x8000,
	0xC000, 0x1F0000, 0x40000, 0, 0x20000,
	0x200, 0, 0, 0xC000
};

struct hdate {
	int hd_day;
	int hd_mon;
	int hd_year;
	int hd_dw;
	int hd_flg;
};

/* print cal of month m of Gregorian year y */
hcal(m, y)
{
	register i, w, ms, hmsk, mmsk, hi;
	struct hdate df, dl, *hdate();
#ifdef HEBREW
	static hs[50];
	char *hnum(), *rev();
#endif

	ms = 30+(0x15aa>>m&1);
	if(m==2) {
		ms = 28;
		if((y&3)==0 && (jflg || !(y%100==0 && (y/100&3)!=0)))
			ms++;
	}
	df = *hdate(1, m, y);
	dl = *hdate(ms, m, y);
	if(y >= 1948)	/* 5 Iyar */
		moadim[7] |= 0x20;
	if(dl.hd_flg < 0)	/* Hanukah in a short year */
		moadim[3] |= 0x8;
	printf("%s %4d", mname[m-1], y);
#ifdef HEBREW
	strcpy(hs, hmname[df.hd_mon]);
	if(df.hd_year != dl.hd_year) {
		strcat(hs, " ");
		strcat(hs, hnum(df.hd_year));
	}
	if(df.hd_mon != dl.hd_mon) {
		strcat(hs, " - ");
		strcat(hs, hmname[dl.hd_mon]);
	}
	strcat(hs, " ");
	strcat(hs, hnum(dl.hd_year));
	printf("%*s", 41-(strlen(mname[m-1])+5+strlen(hs)), "");
	printf("%s\n SUN   MON   TUE   WED   THU   FRI   SAT\n", rev(hs));
#else
	printf(" / %s", hmname[df.hd_mon]);
	if(df.hd_year != dl.hd_year)
		printf(" %d", df.hd_year);
	if(df.hd_mon != dl.hd_mon)
		printf(" - %s", hmname[dl.hd_mon]);
	printf(" %d\n", dl.hd_year);
	printf(" Sun   Mon   Tue   Wed   Thu   Fri   Sat\n");
#endif
	w = df.hd_dw;
	for(i=0; i<w; i++)
		printf("      ");
	hi = df.hd_day+1;
	hmsk = hagim[df.hd_mon]>>hi;
	mmsk = moadim[df.hd_mon]>>hi;
	for(i=1; i<=ms; i++) {
#ifdef HEBREW
		printf("%2d%c%s", i, hmsk&1 ? '*' : mmsk&1 ? '+' : '/', rev(hnum(hi)));
#else
		printf("%2d%c%2d", i, hmsk&1 ? '*' : mmsk&1 ? '+' : '/', hi);
#endif
		if(++w == 7) {
			printf("\n");
			w = 0;
		} else
			printf(" ");
		/* hebrew month changes */
		if(i==1 && ms==31 && dl.hd_day==0 && df.hd_day!=0) {
			df = *hdate(2, m, y);
			hi = 0;
			hmsk = hagim[df.hd_mon];
			mmsk = moadim[df.hd_mon];
		} else if(i == ms-dl.hd_day-1) {
			hi = 0;
			hmsk = hagim[dl.hd_mon];
			mmsk = moadim[dl.hd_mon];
		}
		hi++;
		hmsk >>= 1;
		mmsk >>= 1;
	}
	if(ms-w <= 28)
		printf("\n");
	printf("\n\n");
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
struct hdate *
hdate(d, m, y)
	register m, y, d;
{
	static struct hdate h;
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
	h.hd_dw = (d+1)%7;

	/* compute the year */
	m = (double)d*DAY/MONTH;	/* needs ~34 bits */
	y = (m+38)*19/235-3;
	s = dysiz(y);
	d -= s;
	s = dysiz(y+1)-s;
	y += 3744;

	h.hd_flg = s%10-4;
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
#ifdef HEBREW
char *
hnum(n)
	register n;
{
	static char hn[16];
	register char *p = hn;

	if(n >= 1000) {
		(void)hnum(n/1000);	/* result in hn */
		while(*p && *p!=' ')
			p++;
		n %= 1000;
	}
	while(n >= 400) {
		*p++ = 'z';
		n -= 400;
	}
	if(n >= 100) {
		*p++ = 'v'+n/100;
		n %= 100;
	}
	if(n >= 10) {
		if(n == 15 || n == 16)
			n -= 9;
		*p++ = "hiklnpqrtv"[n/10];
		n %= 10;
	}
	if(n > 0)
		*p++ = '_'+n;
	if(p-hn < 2)
		*p++ = ' ';
	*p++ = 0;
	return hn;
}
char *
rev(as)
	char *as;
{
#ifdef REV
	register char *p, *s;
	register t;

	s = as;
	for(p=s;*p;p++);
	while(p > s) {
		t = *--p;
		*p = *s;
		*s++ = t;
	}
#endif
	return as;
}
#endif

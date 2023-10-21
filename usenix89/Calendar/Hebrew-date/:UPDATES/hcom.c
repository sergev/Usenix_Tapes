/* routines common to hdate and hcal */

#include "hdate.h"

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
	extern jflg;

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
#ifdef BIGLONG
	m = d*DAY/MONTH;	/* needs ~34 bits */
#else
	m = muldiv(d, DAY, 0, MONTH);
#endif
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
#ifdef BIGLONG
	long nmt;	/* needs ~34 bits */
#endif

	m = msiz(y);
#ifdef BIGLONG
	nmt = m*MONTH+M(0,1,779); /* molad at 197 cycles + M(2,5,204) */
	nm = nmt%WEEK;
	s = nmt/DAY-2;
#else
	nm = mulmod(m, MONTH, M(0,1,779), WEEK);
	s = muldiv(m, MONTH, M(0,1,779), DAY)-2;
#endif
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

#ifndef BIGLONG
/* (a*b+c)/m where a*b might overflow */
muldiv (a, b, c, m) 
int a, b, c, m;
{
#ifdef vax
	asm ("emul	4(ap),8(ap),12(ap),r0");
	asm ("ediv	16(ap),r0,r0,r2");
#else
	/* if double is large enough */
	double floor();
	return (long)floor(((double)a*b+c)/m);
#endif
}

/* (a*b+c)%m where a*b might overflow */
mulmod (a, b, c, m) 
int a, b, c, m;
{
#ifdef vax
	asm ("emul	4(ap),8(ap),12(ap),r0");
	asm ("ediv	16(ap),r0,r2,r0");
#else
	/* if double is large enough */
	double floor(), x;
	x = (double)a*b+c;
	return (long)(x-floor(x/m)*m);
#endif
}
#endif

#ifdef HEBREW
char *
hnum(n)
	register n;
{
	static char hn[16];
	register char *p = hn;

	if(n >= 1000) {
		(void)hnum(n/1000);	/* result in hn */
		while(*p)
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
	*p++ = 0;
	return hn;
}
#ifdef REV
char *
rev(as)
	char *as;
{
	register char *p, *s;
	register t;

	s = as;
	for(p=s;*p;p++);
	while(p > s) {
		t = *--p;
		*p = *s;
		*s++ = t;
	}
	return as;
}
#endif
#endif

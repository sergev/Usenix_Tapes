/*
 | calendar with hebrew dates
 | by Amos Shapir 1984 (rev. 1985)
 | The HEBREW option prints hebrew strings in l.c.
 | The REV option reverses them before printing.
 | flag: -j - input date is Julian (default before 1582)
 */
#include "hdate.h"
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

/* print cal of month m of Gregorian year y */
hcal(m, y)
{
	register i, w, ms, hmsk, mmsk, hi;
	struct hdate df, dl;
#ifdef HEBREW
	static hs[50];
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
		printf("%2d%c%2s", i, hmsk&1 ? '*' : mmsk&1 ? '+' : '/', rev(hnum(hi)));
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

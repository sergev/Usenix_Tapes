
/*	FILE:	DATES.C				Jan. 1983
 *
 *	AUTHOR: OSCAR NIERSTRASZ
 *
 *	Usage: dates [-t <days>] <file> ... 
 *
 *	Checks files of events and warns you if they
 *	are coming up soon.
 *	Flags and files may appear in any order.
 *
 *	-t sets the number of days
 *
 *	Assumes files have either structure:
 *	mmdd
 *		event
 *		...
 *	mmdd-mmdd
 *		event
 *		...
 */

#include <stdio.h>
#include <sys/types.h>
#define	TRUE	1
#define	FALSE	0
#define	MAX	500

#define	EOS	'\0'
#define	RANGE	'-'
#define	COMMENT	'#'

#define DLEN	30	/* date buffer length */

#define	NEXT	1	/* NEXT: use next year's date if this	*/
#define	THIS	0	/* year's has passed; THIS: don't	*/

#define	TDIF	6	/* 6 hours earlier than Greenwich	*/
#define	DAYS	7	/* default warning period		*/
#define	num(n)	(n - '0')
#define	dysize(yr)	(365 + (((yr%4)==0)?1:0))
#define	dmsize(yr, m)	(dm[m] + (((yr % 4) == 0) && m >= 2))
#define	month_name(n)	((n < 1 || n > 12) ? name[0] : name[n])
#define	day_name(n)	weekday[((n)%7+7)%7]

long	*time();
void	getday(),
	rtime(),
	tcheck();
int	days = DAYS, putline = FALSE;

struct	ymdhms{
	int	year;
	int	month;
	int	day;
	int	hour;
	int	min;
	int	sec;
	int	wday;
	} t;	/* Today's date */

static	int	dm[13] = {
	0,
	31,
	59,
	90,
	120,
	151,
	181,
	212,
	243,
	273,
	304,
	334,
	365
};

static	char	*name[] = {
		"illegal month",
		"Jan",
		"Feb",
		"March",
		"April",
		"May",
		"June",
		"July",
		"Aug",
		"Sept",
		"Oct",
		"Nov",
		"Dec"
	};

static	char	*weekday[] = {
		"Wed",
		"Thu",
		"Fri",
		"Sat",
		"Sun",
		"Mon",
		"Tue"
	};

char *nom(s)
    char *s; {
	char	*c;

	c = s;
	while(*c != EOS) {
		if(*c == ',') {
			*c = EOS;
			c++;
			while(*c == ' ')
				c++;
			return(c);}
		c++;}
	return(c);}

main(argc, argv)
     int argc;
     char *argv[]; {
	FILE	*fp;
	int	i=1, ll;
	char	s[MAX];

	if(argc == 1) {
		fprintf(stderr, "Usage: dates [-t <days>] <file> ...\n");
		exit(1);}
	rtime();
	while(i<argc) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			    case 't':
				i++;
				if(i == argc)
					fprintf(stderr, "dates: missing arg\n");
				else	days = atoi(argv[i]);
				break;
			    default:
				fprintf(stderr, "dates: unknown flag %s\n", argv[i]);}}
		else {
			if((fp = fopen(argv[i], "r")) == NULL)
				fprintf(stderr, "dates: can't open %s\n", argv[i]);
			else {
				while((ll = getline(s, fp)) != EOF) {
					if(ll == MAX)
					    fprintf(stderr, "dates: line too long: %s\n", s);
					if(s[0] != COMMENT) {
						if(data(s)) {
							if (putline)
							    printf("%s\n", s);}
						else {
							tcheck(s);}}}
				fclose(fp);}}
		i++;}}

void rtime() {	/*	sets today's date in t.		*/
	int	i, days;
	long	tbuf;

	time(&tbuf);		/*	seconds	*/
	t.sec = tbuf % 60;
	tbuf = tbuf / 60;	/*	minutes	*/
	t.min = tbuf % 60;
	tbuf = tbuf / 60 - TDIF; /*	hours	*/
	t.hour = tbuf % 24;
	tbuf = 1 + tbuf / 24;	/*	days	*/
	t.wday = tbuf % 7;
	t.year = 1970;
	days = dysize(t.year);
	while(tbuf > days) {
		(t.year)++;
		tbuf -= days;
		days = dysize(t.year);}
	for(i=1; i<=12; i++) {
		days = dmsize(t.year, i);
		if(tbuf < days)
		    break;}
	t.month = i;
	t.day = tbuf - dmsize(t.year, i-1);}

void tcheck(s)
    char *s; {
	char	*sn, *f, date1[DLEN], date2[DLEN];
	int	dif1, dif2;

	getday(s, &dif1, date1, NEXT);
	if(s[4] == EOS) {
		if(dif1 <= days) {
			putline = TRUE;
			printf("%s :\n", date1);}
		else	putline = FALSE;}
	else if(s[4] == RANGE) {
		getday(s+5, &dif2, date2, NEXT);
		/* if date1 has passed but date2 hasn't,	*/
		/* force getday to use this year for date1	*/
		if(dif2 < dif1)
		    getday(s, &dif1, date1, THIS);
		if((dif1 <= days) && (dif2 >= 0)) {
			putline = TRUE;
			printf("%s to %s :\n", date1, date2);}
		else putline = FALSE;}}

void getday(s, pdif, date, next)
    char *s, *date;
    int	*pdif, next;{
	int	m, d, dif;

	m = num(s[0])*10 + num(s[1]);
	d = num(s[2])*10 + num(s[3]);
	*pdif = dif = dmsize(t.year, m-1) + d
		- (dmsize(t.year, t.month-1) + t.day);
	if(next && dif < 0)
	    *pdif = dif = dysize(1+t.year) + dmsize(1+t.year, m)
		+ d - (dmsize(t.year, t.month) + t.day);
	if(dif == -1)
	    sprintf(date, "Yesterday");
	else if(dif == 0)
	    sprintf(date, "Today");
	else if(dif == 1)
	    sprintf(date, "Tomorrow");
	else
	    sprintf(date, "%s %s %d", day_name(dif+t.wday),
		month_name(m), d);}

int data(s)
    char *s; {
	/* Returns TRUE if line is data		*/
	/* FALSE if date line			*/
	/* Acceptable formats are "mmdd"	*/
	/* and "mmdd-mmdd"			*/
	int	i;

	for(i=0; i<4; i++)
	    if(s[i] < '0' || '9' < s[i])
		return(TRUE);
	if(s[4] == EOS)
	    return(FALSE);
	/* date may be a range */
	if(s[4] == RANGE) {
		/* Can't have "nested" ranges */
		if(s[9] != EOS)
		    return(TRUE);
		return(data(s+5));}
	return(TRUE);}

int getline(s,file)
    char *s;
    FILE *file; {
	/*	get a line of input up to MAX characters	*/
	/*	return length of input or EOF or MAX		*/
	int	i=0;
	char	c;

	while((c=fgetc(file)) != '\n') {
		if(c == EOF) {
			rewind(file);
			return(EOF);}
		s[i] = c;
		i++;
		if(i == MAX) {
			s[i-1] = '\0';
			return(MAX);}}
	s[i] = '\0';
	return(i);}

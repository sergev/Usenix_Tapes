#

/*
 *  Routine to return an exit status of one or zero on a comparison
 *  of the current time to a value.
 */

int *tbuf ;
int tvec[2] ;

#define TIME	3000
#define DAY		3001
#define MONTH	3002
#define YEAR	3003
#define DOW		3004
#define DOY		3005
#define DST		3006

#define EQUAL		4000
#define NOTEQUAL	4001
#define LESS		4002
#define GREAT		4003
#define LESSEQUAL	4004
#define GREATEQUAL	4005


struct {
	char *p ;
	int v ;
	} symbol[] {
	"time",		3000,
	"t",		3000,
	"hms",		3000,
	"day",		3001,
	"d",		3001,
	"month",	3002,
	"m",		3002,
	"year",		3003,
	"y",		3003,
	"dow",		3004,
	"doy",		3005,
	"dst",		3006,

	"==",		4000,
	"=",		4000,
	"is",		4000,
	"eq",		4000,
	"!=",		4001,
	"ne",		4001,
	"<",		4002,
	"lt",		4002,
	">",		4003,
	"gt",		4003,
	"<=",		4004,
	"=<",		4004,
	"le",		4004,
	">=",		4005,
	"=>",		4005,
	"ge",		4005,

	"january",	5000,
	"jan",		5000,
	"february",	5001,
	"feb",		5001,
	"march",	5002,
	"mar",		5002,
	"april",	5003,
	"apr",		5003,
	"may",		5004,
	"june",		5005,
	"jun",		5005,
	"july",		5006,
	"jul",		5006,
	"august",	5007,
	"aug",		5007,
	"september",5008,
	"sept",		5008,
	"sep",		5008,
	"october",	5009,
	"oct",		5009,
	"november",	5010,
	"nov",		5010,
	"december",	5011,
	"dec",		5011,

	"sunday",	6000,
	"sun",		6000,
	"monday",	6001,
	"mon",		6001,
	"tuesday",	6002,
	"tues",		6002,
	"wednesday",6003,
	"wed",		6003,
	"thursday",	6004,
	"thurs",	6004,
	"friday",	6005,
	"fri",		6005,
	"saturday",	6006,
	"sat",		6006,
	0,			0
	} ;

int param[3] ;

int ptime[3] ;

main(argc,argv)
int argc ;
char *argv[] ;
{
	int rel ;

	time(tvec) ;
	tbuf = localtime(tvec) ;

	if (argc != 4) {
		printf("Usage:  xtime name relation value\n") ;
		exit(1) ;
		}

	param[0] = getparm(argv[1]) ;
	param[1] = getparm(argv[2]) ;
	if ((param[2] = getparm(argv[3])) < 0) gettime(argv[3],ptime) ;

	rel = 0 ;

	switch( param[0] ) {
		case TIME :
			if (param[2] >= 0 ||
				ptime[2] < 0 || ptime[2] > 23 ||
				ptime[1] < 0 || ptime[1] > 59 ||
				ptime[0] < 0 || ptime[0] > 59 ) {
				printf("Error in time value.\n") ;
				exit(1) ;
				}
			if ((rel = tbuf[2] - ptime[2]) == 0) {
				if ((rel = tbuf[1] - ptime[1]) == 0) {
					rel = tbuf[0] - ptime[0] ;
					}
				}
			break ;

		case DAY :
			if (param[2]<0 ||
				param[2]>31) {
				printf("Error in day.\n") ;
				exit(1) ;
				}
			rel = tbuf[3] - param[2] ;
			break ;

		case MONTH :
			if (param[2] >= 5000) param[2] =- 5000 ;
			if (param[2]<0 || param[2]>11) {
				printf("Error in month.\n") ;
				}
			rel = tbuf[4] - param[2] ;
			break ;

		case YEAR :
			if (param[2]>1900) param[2] =- 1900 ;
			if (param[2]<0 || param[2]>99) {
				printf("Error in year.\n") ;
				exit(1) ;
				}
			rel = tbuf[5] - param[2] ;
			break ;

		case DOW :
			if (param[2] >= 6000) param[2] =- 6000 ;
			if (param[2] < 0 || param[2] > 6) {
				printf("Error in day-of-week.\n") ;
				}
			rel = tbuf[6] - param[2] ;
			break ;

		case DOY :
			if (param[2] < 0 || param[2] > 366) {
				printf("Error in day-of-year.\n") ;
				exit(1) ;
				}
			rel = tbuf[7] - param[2] ;
			break ;

		case DST :
			rel = tbuf[8] & param[2] ;
			break ;

		default :
			printf("Error in time-name.\n") ;
			exit(1) ;
			break ;

		}

	switch ( param[1] ) {
		case EQUAL :
			if (rel) exit(1) ;
			else exit(0) ;

		case NOTEQUAL :
			if (rel) exit(0) ;
			else exit(1) ;

		case LESS :
			if (rel < 0) exit (0) ;
			else exit(1) ;

		case LESSEQUAL :
			if (rel <= 0) exit (0) ;
			else exit(1) ;

		case GREAT :
			if (rel > 0) exit (0) ;
			else exit(1) ;

		case GREATEQUAL :
			if (rel >= 0) exit (0) ;
			else exit(1) ;

		default :
			printf("Error in relation.\n") ;
			exit (1) ;

		}

	}





/*
 * Routine to get the value of a parameter.
 */

getparm(x)
char *x ;
{
	char *c ;
	int i ;

	c = x ;

	if (*c >= '0' && *c <= '9') {		/* Numeric */
		while (*c) {
			if (*c == ':') return (-1) ;
			c++ ;
			}
		if ((i = atoi(x)) < 0 || i >= 3000) {
			printf("Numeric value out of bounds.\n") ;
			exit(1) ;
			}
		return (i) ;
		}
	else {
		for (i = 0; symbol[i].p != 0; i++) {
			if ( strcmp(symbol[i].p,x) == 0) return (symbol[i].v) ;
			}
		printf("Symbol %s not defined.\n",x) ;
		exit(1) ;
		}

	}





/*
 * Routine to convert an ascii time value of the form hh:mm:ss to
 * a vector of integers.  ss,mm,hh.
 */

gettime(x,t)
char *x ;
int t[3] ;
{
	t[0] = t[1] = t[2] = 0 ;

	t[2] = atoi(x) ;
	while (*x && *x != ':') {
		x++ ;
		}
	if (*x == '\0') return ;

	x++ ;
	t[1] = atoi(x) ;
	while (*x && *x != ':') {
		x++ ;
		}
	if (*x == '\0') return ;

	t[0] = atoi(x+1) ;
	return ;

	}





/*
 * routine to compare two strings..
 */

strcmp(s1,s2)
char *s1 ;
char *s2 ;
{
	while (*s1 && *s1 == *s2) {
		s1++ ;
		s2++ ;
		}

	return (*s1 - *s2) ;
	}

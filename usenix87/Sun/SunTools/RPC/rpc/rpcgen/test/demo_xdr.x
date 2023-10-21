/*
 *  This file defines the protocol to support a demo rpc
 *  service and is designed to be processed by the `rpcgen'
 *  protocol compiler.  The Sun ctime(3) routines are used
 *  as examples.  See the rpcgen manual page for more detailed
 *  information.
 */

#ifdef REL3_0
#ifndef NULL
#       define NULL 0
#endif
#endif


/*
 *  argument / result data types
 */

#define DATELEN 26
typedef string date[DATELEN];

#define STRLEN 80
typedef string str[STRLEN];

typedef long clock;

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

struct timeval {
	u_long	tv_sec;
	long	tv_usec;
};

struct timez {
	int	tz_minuteswest;
	int	tz_dsttime;
};

struct tzargs {
	int	zone;
	int	dst;
};


/*
 *  There is one generic data type for the result.  This is implemented
 *  as a union switch.  The specific union element returned is specified
 *  by ret_status.
 */

enum ret_status {
	RET_DATE, RET_TM, RET_STR, RET_DAYS, RET_CLOCK, RET_TZ, RET_YEAR,
	RET_ERROR
};

struct ret_err {
	int	err_number;
	str	err_text;
};

union demo_res switch (ret_status which) {
	case RET_DATE:
		date	date;
	case RET_TM:
		tm	*tmp;
	case RET_STR:
		str	str;
	case RET_DAYS:
		int	days;
	case RET_ERROR:
		ret_err err;
};


/*
 *  This section declares the individual procedures which are supported
 *  by this protocol.  The program number and version numbers are
 *  arbitrary in this case.  Procedures correspond to the ctime(3)
 *  time routines.
 */

program DEMOPROG {
    version DEMOVERS {
	demo_res	/* date */	CTIME(clock)		= 1;
	demo_res	/* tm   */	LOCALTIME(clock)	= 2;
	demo_res	/* tm   */	GMTIME(clock)		= 3;
	demo_res	/* date */	ASCTIME(tm)		= 4;
	demo_res	/* str  */	TIMEZONE(tzargs)	= 5;
	demo_res	/* days */	DYSIZE(int)		= 6;
    } = 5;
} = 123456;

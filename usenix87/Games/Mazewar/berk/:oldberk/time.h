#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct timeval {
	int	tv_usec;
	int	tv_sec;	
};

struct itimerval {
	struct timeval it_interval;
	struct timeval it_value;
};

#include "../berk/btime.h"

/*	time.h	6.1	83/07/29	*/

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};


setitimer(which,value,ovalue)
int which;
struct timerval *value, *ovalue;
{
	int al;
	al = value->tv_sec;
	if(value->tv_usec!=0) ++al;
	switch(which) {
	case ITIMER_VIRTUAL:
		sprintf(stderr,"berkerr: virtual timer access\n");
		return -1;
	case ITIMER_PROF:
		sprintf(stderr,"berkerr: prof timer access\n");
		return -1;
	default:
		alarm(al);
		if

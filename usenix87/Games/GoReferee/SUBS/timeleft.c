
/*
**	SETIMELEFT -- set user time limit (seconds)
**	TIMELEFT -- return amount of user time not yet used (seconds)
*/
#ifndef	BSD
#ifndef	V8
oops()	{	BSD = ~V8; }	/* to generate an error message */
#endif	V*
#endif	BSD

#ifdef	BSD

#include	<sys/time.h>
#include	<sys/resource.h>

static	long	stoptime;
static	struct	rusage	ru;

setimeleft(secs)
{
	getrusage(RUSAGE_SELF, &ru);
	stoptime = ru.ru_utime.tv_sec + secs;
}

timeleft()
{
	getrusage(RUSAGE_SELF, &ru);
	return(stoptime - ru.ru_utime.tv_sec);
}

#endif
#ifdef	V8

#include	<sys/types.h>
#include	<sys/times.h>

static	time_t	stoptime;
static	struct	tms t;

setimeleft(secs)
{
	times(&t);
	stoptime = t.tms_utime + secs * 60;
}

timeleft()
{

	times(&t);
	return((stoptime - t.tms_utime) / 60);
}

#endif

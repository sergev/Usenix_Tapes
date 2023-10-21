#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#ifndef BSDREL >= 7
#include <sys/timeb.h>
#else
struct timeb
{
	time_t	time;
	unsigned short millitm;
	short	timezone;
	short	dstflag;
};
#endif

time_t getdate();
long getadate();


time_t
cgtdate(datestr)
char *datestr;
{
	time_t	i;
	char	junk[40],month[40],day[30],time[60],year[50];
	char	bfr[181];

	if ((i = getadate(datestr)) != -1L)
		return i;
	if ((i = getdate(datestr, (struct timeb *) NULL)) >= 0)
		return i;
	sscanf(datestr, "%s %s %s %s %s", junk, month, day, time, year);
	sprintf(bfr, "%s %s, %s %s", month, day, year, time);
	return getdate(bfr, (struct timeb *) NULL);
}

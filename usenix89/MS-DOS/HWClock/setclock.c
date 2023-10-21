#include <stdio.h>
#include <time.h>
#include "clock.h"

main (argc, argv)
int	argc;
char	*argv[];
{
	long time(), clicks;
	struct tm *localtime(), *tm;
	
	time(&clicks);
	tm = localtime(&clicks);
 	
	out(SEC, i2hwc(tm->tm_sec));
	out(MIN, i2hwc(tm->tm_min));
	out(HOUR, i2hwc(tm->tm_hour));
	out(MDAY, i2hwc(tm->tm_mday));
	out(MON, i2hwc(tm->tm_mon + 1));
	out(YEAR, i2hwc(tm->tm_year - 80));
	out(WDAY, i2hwc(tm->tm_wday));
}

i2hwc(getal)
int getal;
{
	char buf[3];
	int i;

	sprintf(buf, "%2d\0", getal);
	sscanf(buf, "%x", &i);
	return i;
}

#include <stdio.h>
#include <time.h>
#include "clock.h"

main (argc, argv)
int	argc;
char	*argv[];
{
	char *asctime(), c;
	struct  tm *tm;
	
	
	tm->tm_sec = hwc2i(in(SEC));
	tm->tm_min = hwc2i(in(MIN));
	tm->tm_hour = hwc2i(in(HOUR));
	tm->tm_mday = hwc2i(in(MDAY));
	tm->tm_mon = hwc2i(in(MON)) - 1;
	tm->tm_year = hwc2i(in(YEAR)) + 80;
	tm->tm_wday = hwc2i(in(WDAY));

	if (argc > 1) 
		printf("%02d%02d%02d%02d%02d\n",
		tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_year);
	else	
		printf("%s", asctime(tm));
}

hwc2i(getal)
int getal;
{
	char	buf[3];
	
	sprintf(buf, "%2x\0", getal);
	return atoi(buf);
}

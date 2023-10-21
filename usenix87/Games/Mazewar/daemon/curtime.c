#ifndef lint
static char rcsid[] = "$Header: curtime.c,v 1.1 84/08/25 17:04:51 lai Exp $";
#endif

#include <sys/types.h>	/*RMM*/
#include <sys/param.h>
#include <sys/time.h>

#ifndef	NULL
#define NULL	0
#endif

#define CTIME_SIZE	27

char *
curtime()
{
	extern char *ctime();
	static char HUH[] = "??? ??? ?? ??:??:?? ????";
	static char buf[CTIME_SIZE];
	char *date;
	time_t now;

	(void) strcpy(buf, HUH);

	if (time(&now) != -1) {
		date = ctime(&now);
		if (date != NULL) {
			(void) strcpy(buf, date);
			if (buf[strlen(buf) - 1] == '\n')
				buf[strlen(buf) - 1] = '\0';
		}
	}

	return (buf);
}

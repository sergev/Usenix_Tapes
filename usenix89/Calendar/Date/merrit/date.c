
/*
 * show time like "date(1)" but with am/pm rather than 24 hour
 *
 * Copyright 1984 Doug Merritt
 * Permission is hereby granted to freely redistribute this program.
 */
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

struct tm *localtime();
long time();

#define	to_ampm(n)	(n > 12? n-12 : n)
#define	am_pm(n)	(n > 12? "pm" : "am")

char	*month[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};
char	*week[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

main(ac, av)
	int	ac;
	char	**av;
{
	long	t;
	struct	tm *bt;
	char	cmd[100];

	/*
	 * Asked us to SET the date??? Punt...
	 */
	if (ac > 1) {
		sprintf(cmd, "/bin/date %s", av[1]);
		exit(system(cmd));
	}
	t = time(0L);
	bt = localtime(&t);
	printf("%s %s %d 19%02d %d:%02d:%02d %s\n",
		week[bt->tm_wday], month[bt->tm_mon], bt->tm_mday,
		bt->tm_year,
		to_ampm(bt->tm_hour), bt->tm_min, bt->tm_sec,
		am_pm(bt->tm_hour) );
}

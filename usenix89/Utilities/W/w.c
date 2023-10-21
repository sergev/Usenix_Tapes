char	*sccsid = "@(#)w.c  Mikel Manitius  AT&T-IS  86-03-14";
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <time.h>
#include <pwd.h>

/*
 * w - who (what) where when (why?)
 *
 * Mikel Manitius - 86-03-14 - (mikel@codas.att.com.uucp)
 *
 */

struct	utmp ut;
struct	stat st;
struct	tm   *tm;
struct	tm   *localtime();
struct	passwd	*pw;
struct	passwd	*getpwnam();

int	sflag = 0;

char	*pname;
char	*ch_time();
char	*sh_time();

extern	char *day[];
extern	char *mon[];

char *idle(device)
char	*device;
{
	int	now;
	int	idle;
	int	min;
	int	sec;
	char	*pp;

	pp = (char *) malloc(10);
	if(pp == NULL)
		return(NULL);
	time(&now);
	if(stat(device, &st) != 0) {
		perror(device);
		exit(-1);
		}
	idle = now - st.st_atime;
	min = idle / (60 * 60);
	sec = (idle - (min * 60 * 60)) / 60;
#ifdef SILENT
	if(idle < 59)
		sprintf(pp, "  %2d ", idle);
	else
#endif
		sprintf(pp, "%2d:%02d", min, sec);
	return(pp);
}

int	done = 0;

main(argc, argv)
int	argc;
char	*argv[];
{
	register	fd;
	int	i;
	int	j;

	pname = argv[0];
	for(i=1;i < argc;i++)
		switch(argv[i][0]) {
			case '-':
				for(j=1;argv[i][j] != '\0';j++)
					switch(argv[i][j]) {
						case 's':
							sflag++;
							break;
						default:
							usage();
							break;
						}
				break;
			default:
				usage();
				break;
			}

	fd = open(UTMP_FILE, 0);
	if(fd < 0) {
		perror(fd);
		exit(-1);
		}
	while(read(fd, (char *)&ut, sizeof(ut)))
		if(ut.ut_type == USER_PROCESS) {
			char	device[20];

			if(!done) {
				printf("login        tty         login@    idle    root\n");
				done = 1;
				}
			tm = localtime(&ut.ut_time);
			printf("%-8s    %-8s     %2d:%02d    ",
				ut.ut_name, ut.ut_line,
				tm->tm_hour, tm->tm_min);
			sprintf(device, "/dev/%s", ut.ut_line);
			printf("%-5s   %5d", idle(device),
				ut.ut_pid);
			if(access(device, 2) != 0)
				printf("    (messages off)\n");
			else if(access(device, 1) == 0)
				printf("    (hungry)\n");
			else
				doname(ut.ut_name);
		} else if(ut.ut_type == BOOT_TIME)
			uptime(ut.ut_time);
	close(fd);
}

uptime(at)
int	at;
{
	int	now;
	char	*up;

	time(&now);
	tm = localtime(&now);

	if(sflag) {
		up = sh_time(now - at);
		printf(" %s %s %d %02d:%02d:%02d, up %s\n",
			day[tm->tm_wday], mon[tm->tm_mon],
			tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_sec, up);
	} else {
		up = ch_time(now - at);
		printf(" %2d:%02d:%02d, up %s\n",
			tm->tm_hour, tm->tm_min, tm->tm_sec, up);
		}
}

doname(name)
char	*name;
{
	int	i;
	int	len;
	char	fmt[10];

	pw = getpwnam(name);
	if(pw == NULL) {
		putc('\n', stdout);
		return(-1);
		}
	len = strlen(pw->pw_gecos) - 1;
	for(i=0;i < len;i++)
		if(isdigit(pw->pw_gecos[i]) || pw->pw_gecos[i] == ',') {
			i--;
			break;
			}
	sprintf(fmt, "   %%.%ds\n", i+1);
	printf(fmt, pw->pw_gecos);
}

usage()
{
	fprintf(stderr, "usage: %s [ -s ]\n", pname);
	exit(-1);
}

#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "at.h"



long num;
int fouryr, year, day, hour, minute, par_pid;
char error[80], fullname[30];
FILE *err_fp, *id_fp;


main()
{
	FILE *fp;
	char buf[BUFLEN];

	chdir("/usr/spool/at");
	if ((err_fp = fopen(ERRFILE, "a+")) < 0)
		exit(-1);
	if ((id_fp = fopen(IDFILE, "r+")) < 0)
		exit(-1);
	
	system("/bin/sh -c 'cd /usr/spool/at; /bin/ls [0-9]* > temp'");
	now();
	if ((fp = fopen("temp", "r")) < 0) {
		fprintf(err_fp, "atrun: cannot open temporary file %s\n", TEMPFILE);
		exit(-1);
	}

	/* if the first character picked up is not a number, then it is
	   the beginning of the error message "[0-9]* not found". in that
	   case, quit, since there are no files waiting in the AT
	   directory. */

	while ((fscanf(fp, "%s", buf) != EOF) && (isdigit(*buf))) {
		process(buf);
	}
	unlink(TEMPFILE);
	rewind(err_fp);
	if (fscanf(err_fp, "%s", buf) == EOF)
		unlink(ERRFILE);
		
}

	/* 'str' contains the name of a file which we check against the current
	    time. if it ought to be run, setuid and setgid, then run it by
	    exec'ing /bin/sh with the filename as its first argument. if it
	    was run, remove it afterwards. */

process(str)
char *str;
{
	char o_name[30], buf[BUFLEN];
	time_t o_time;
	struct stat stat_buf;
	FILE *fopen();
	int fd, pid, o_uid, found;

	if (!ripe(str))
		return;

	/* get the directory entry of the file in order to find the owner
	   and group ID's. if unable to get the info, just skip it. */
	if (stat(str, &stat_buf) < 0)
		return;

	/* make sure that no one has tampered with the file except, possibly,
	   the owner. if someone else has, remove it and return - but do not
	   execute! likewise, if there is a file in /usr/spool/at that looks
	   like an at-file, but has no entry in /usr/spool/at/.ids, refuse
	   to run it. */

	sprintf(fullname, "/usr/spool/at/%s", str);
	rewind(id_fp);
	found = 0;
	while (fgets(buf, BUFLEN, id_fp) != NULL) {
		sscanf(buf, "%s %d %ld", o_name, &o_uid, &o_time);
		if (strcmp(fullname, o_name) == 0) {
			if (o_uid == stat_buf.st_uid) {
				found = 1;
				break;
			}
		}
	}
	if (found == 0) {
		cleanup(fullname, &stat_buf);
		return;
	}
	
	if ((pid = fork()) < 0)
		die("unable to fork");
	else if (pid != 0) {
		wait((int *) 0);
		cleanup(fullname, &stat_buf);
	}
	else {

		/* set uid and gid, then force file descriptors 0, 1 and 2 to point
		   to /dev/null so that any output from the 'sh' process (NOT
		   the processes it is running, of course) is sent there. */

		setuid((int) stat_buf.st_uid);
		setgid((int) stat_buf.st_gid);
		fd = open("/dev/null", O_RDWR);
		close(0); close(1); close(2);
		dup(fd); dup(fd); dup(fd);
		execl("/bin/sh", "/bin/sh", fullname, 0);
	}
}

	/* remove the file from /usr/spool/at, and get rid of its entry in
	   the file /usr/spool/at/.ids. */

cleanup(str, s_buf)
char *str;
struct stat *s_buf;
{	
	FILE *fp, *fopen();
	char o_name[30], *t, *mktemp(), buf[BUFLEN];
	int o_uid, o_time;

	rewind(id_fp);
	t = mktemp("atXXXXXX");
	fp = fopen(t, "w");
	while (fgets(buf, BUFLEN, id_fp) != NULL) {
		sscanf(buf, "%s %d %ld", o_name, &o_uid, &o_time);
		if (strcmp(str, o_name) != 0)
			fputs(buf, fp);
	}
	fclose(id_fp);
	fclose(fp);
	unlink(IDFILE);
	link(t, IDFILE);
	chown(IDFILE, 0, 0);
	chmod(IDFILE, 0600);	
	id_fp = fopen(IDFILE, "r+");
	unlink(t);
	unlink(str);
}


	/* return 1 if the filename indicates a time earlier than the present,
	   0 otherwise. */

#define  DECIDED(x, y) (x < y ? 1 : ((x == y) ? -1 : 0))

ripe(str)
char *str;
{
	int d, f_year, f_day, f_hour, f_minute;

	if (sscanf(str, "%2d%3d.%2d%2d", &f_year, &f_day, &f_hour, &f_minute) < 4) {
		sprintf(error, "bad file name %s", str);
		burp(error);
	}
	if ((d = DECIDED(f_year, year)) >= 0) 
		return(d);
	if ((d = DECIDED(f_day, day)) >= 0)
		return(d);
	if ((d = DECIDED(f_hour, hour)) >= 0)
		return(d);
	return(f_minute <= minute);	
}


	/* put error message in error file and quit. remove temporay
	   files if this process is the parent process called in
	   crontab, but not if it is just a child process called
	   in a fork() earlier. */

die(str)
char *str;
{
	fprintf(err_fp, "atrun: %s\n", str);
	if (getpid() == par_pid)
		unlink(TEMPFILE);
	exit(-1);
}

	/* put message in error file but don't quit. */

burp(str)
char *str;
{
	fprintf(err_fp, "atrun: %s\n", str);
}

now()
{

	num = time((long *) 0) - LOCAL;
	num -= (fouryr = num / S_FOUR_YEAR) * S_FOUR_YEAR;
	num -= (year = num / S_YEAR) * S_YEAR;
	year += 70 + (4 * fouryr);
	num -= (day = num / S_DAY) * S_DAY;
	num -= (hour = num / S_HOUR) * S_HOUR;
	num -= (minute = num / S_MINUTE) * S_MINUTE;

}


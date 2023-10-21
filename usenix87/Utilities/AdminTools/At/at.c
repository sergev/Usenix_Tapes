#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include "at.h"

	/* number of days in the months of the year */

int mon_days[12] = { 0, 31, 59, 90, 120, 150, 181, 212, 242, 273, 303, 334 };


	/* names of things */

char *mon_name[] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"decmeber"
};

char *day_name[] = {
	"sunday",
	"monday",
	"tuesday",
	"wednesday",
	"thursday",
	"friday",
	"saturday"
};


#define MONTH  1
#define WEEK   2

	/* index into the array of either months or days of the given 
	   second argument */

int index = 0;
int fouryr, year, day, hour, minute;
int pres_day, pres_wkday;
int at_days, at_wkday;
long now_time, at_time, till_tomorrow, leftover;
char t_char, error[100];



main(argc, argv)
int argc;
char *argv[];
{
	long to_seconds();
	char temp[10];
	char filename[20];
	int type;
	
	
	if (argc < 2)
	  err_msg("usage: at time [ month [ day ] || weekday [ week ] ] [ file ]");

	now();
	at_time = to_seconds(argv[1]);

	/* the possible cases are:

		1.	at 340  [ file ]
		2.	at 340 jan 23  [ file ]
		3.  at 340 sat  [ week ] [ file ]

	   take care of each in turn below: get the number of days in the future
	   each one is (in 'at_days') and if there is a filename, place
	   it in 'filename'.  */

	filename[0] = '\0';
	if (argc == 2) {
		at_days = 0;
	}
	else if (argc == 3) {
		if (access(argv[2], 0) == 0) {
			strcpy(filename, argv[2]);	
			at_days = 0;
		}
		else {
			if (analyze(argv[2]) != WEEK)
				err_msg("invalid second argument");
			at_days = then(argv[2], "");
		}
	}
	else {
		type = analyze(argv[2]);
		at_days = then(argv[2], argv[3]);
		if ((type == WEEK) && (strcmp(argv[3], "week") != 0))
			strcpy(filename, argv[3]);
		if (argc > 4)
			strcpy(filename, argv[4]);
	}

	if ((at_days == 0) && (at_time < S_DAY - till_tomorrow))
		at_days = 1;
	if (at_days == 0)
		at_time += now_time + till_tomorrow + 1 - S_DAY;
	else {
		at_time += now_time + till_tomorrow;
		if (at_days > 1)
			at_time += S_DAY * (at_days - 1);
	}

	calstr(at_time, temp);
	makefile(temp, filename);
	putid(temp);

}


putid(file)
char *file;
{
	FILE *fp, *fopen();
	char name[30];
	struct stat sbuf;

	sprintf(name, "/usr/spool/at/%s%c", file, t_char);
	if (stat(name, &sbuf) < 0)
		err_msg("cannot get status of file");

	if ((fp = fopen(IDFILE, "a")) == NULL) {
		sprintf(error, "cannot open file %s\n", IDFILE);
		err_msg(error);
	}

	fprintf(fp, "%s %d %ld\n", name, sbuf.st_uid, sbuf.st_ctime);
	fclose(fp);
	chown(IDFILE, 0, 0);
	chmod(IDFILE, 0600);
}


	/* create a file in the directory /usr/spool/at which is owned
	   by the current at user and contains the commands which he
	   wishes to execute via /bin/sh. already in the file are a
	   'cd' command into the current directory, and a bunch of
	   commands to set up the environment to be identical to the
	   present one. */


makefile(time, filename)
char *time, *filename;
{
	FILE *fp, *fp2;
	char t_file[20], buf[BUFLEN];

	if ((fp = fopen(TEMPFILE, "a")) == NULL)
		err_msg("can not create temporary file");
	if (*filename == '\0')
		fp2 = stdin;
	else
		if ((fp2 = fopen(filename, "r")) == NULL) {
			sprintf(error, "can't open %s", filename);
			err_msg(error);
		}

	while (fgets(buf, BUFLEN, fp2) != NULL)
		fputs(buf, fp);
	fclose(fp);
	fclose(fp2);

	t_char = 'A';
	sprintf(t_file, "/usr/spool/at/%s%c", time, t_char);
	while ((access(t_file, 0) == 0) || (errno != ENOENT)) {
		if (++t_char > 'z')
			err_msg("too many files");
		sprintf(t_file, "/usr/spool/at/%s%c", time, t_char);
	}

	if (link(TEMPFILE, t_file) < 0) {
		sprintf(error, "cannot create file %s", t_file);
		err_msg(error);
	}
	chown(t_file, getuid(), getgid());
	chmod(t_file, 0600);
	if (unlink(TEMPFILE) < 0)
		fprintf(stderr, "warning: cannot remove temporary file %s", TEMPFILE);
}
		


	/* turn the given time string, such as '356am', into the number of
	   seconds from (the previous) midnight. trailing letters can be
	   'A' for AM, 'P' for PM, 'N' for noon, 'M' for midnight. */

long
to_seconds(t_str)
char *t_str;
{
	long retval;
	char last;

	if (sscanf(t_str, "%ld", &retval) < 1)
		err_msg("invalid time of day");
	if (strlen(t_str) < 3)
		retval *= 100;
	if ((retval < 0) || (retval > 2400))
		err_msg("invalid time of day");

	last = toupper(t_str[strlen(t_str) - 1]);
	switch (last) {
	case 'A':
		/* do nothing - 24-hour time is default. */
		break;
	case 'P':
		retval += 1200;
		break;
	case 'N':
		/* again, do nothing, since 1200 is by default noon */
		break;
	case 'M':
		if (retval == 1200)
			retval = 2400;
		break;
	default:
		break;
	}

	return((retval / 100) * 3600 + (retval % 100) * 60);	
}

	/* figure out whether the given string is a month name or a day
	   name. set the global variable 'index' to the index of the
	   found item in its array, and return either MONTH or S_DAY
	   to the calling routine. if ambiguous, terminate program. */

analyze(str)
char *str;
{
	int i, j, k, max, max2, m_max;

	max = max2 = 0;

	/* first see how it compares to months */
	for (i = 0; i < 12; i++) {
		for (j = 0; mon_name[i][j] == str[j]; j++)
			;
		if (j > max) {
			max = j;
			index = i;
		}
		else if (j == max)
			max2 = max;
	}

	m_max = max;

	/* now do days */
	for (k = 0; k < 7; k++) {
		for (j = 0; day_name[k][j] == str[j]; j++)
			;
		if (j > max) {
			max = j;
			index = k;
		}
		else if (j == max)
			max2 = max;
	}

	if (max2 == max)
		err_msg("ambiguous date");
	else 
		return(m_max == max ? MONTH : WEEK);
}
		
	
		

	/* print an error message and quit */

err_msg(str)
char *str;
{
	fprintf(stderr, "at: %s\n", str);
	unlink(TEMPFILE);
	exit(-1);
}


	/* get the current time in seconds since Jan 1 1970, 
	   day-of-the-year, weekday and number of seconds until midnight tonight. */

now()
{
	char nowstr[10];

	now_time = time((long *) 0) - LOCAL;
	calstr(now_time, nowstr);

	pres_day = day;
	pres_wkday = ((366 * fouryr) + (365 * year) + pres_day + 6) % 7; 
	till_tomorrow = (23 - hour) * S_HOUR + (59 - minute) * S_MINUTE	+ 
		(60 - leftover);
}


	/* figure out how many S_DAYS into the future the given date is, and
	   return that number. pass argv[2] in arg2, which sohuld have either
	   a month name or a weekday name, and argv[3] in arg3, which should
	   be either a number (in the case of MONTH) or "week" (in the case of
	   S_DAY) or possibly the name of the file to be run later. */

then(arg2, arg3)
char *arg2, *arg3;
{
	int diff, day, wk_extra, leap_day;

	if (analyze(arg2) == MONTH) {
		if (sscanf(arg3, "%d", &day) < 1)
			err_msg("invalid day number following month name");
		day += mon_days[index];
		if (((year == 0) && (pres_day < 60) && (day >= 60)) ||
		    ((year == 3) && (day < pres_day) && (day >= 60)))
			leap_day = 1;
		else	
			leap_day = 0;
		if ((diff = day - pres_day) < 0)
			return((365 - diff) + leap_day);
		else
			return(diff + leap_day);
	}
	else {
		if (strcmp(arg3, "week") == 0)
			wk_extra = 7;
		else
			wk_extra = 0;
		if ((diff = index - pres_wkday) < 0)
			return((7 + diff) + wk_extra);
		else
			return(diff + wk_extra);
	}
} 


	/* given some number of seconds since 0:00 Jan 1, 1970 in the
	   parameter num, put in the parameter str the date in form
	   YYDDD.HHMM */

calstr(num, str)
long num;
char *str;
{
	
	num -= (fouryr = num / S_FOUR_YEAR) * S_FOUR_YEAR;
	num -= (year = num / S_YEAR) * S_YEAR;
	num -= (day = num / S_DAY) * S_DAY;
	num -= (hour = num / S_HOUR) * S_HOUR;
	num -= (minute = num / S_MINUTE) * S_MINUTE;
	leftover = num;

	sprintf(str, "%02d%03d.%02d%02d", 4*fouryr + year + 70, day, hour, minute);
}

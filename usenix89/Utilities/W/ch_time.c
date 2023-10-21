#define			NULL		(0)
#define			SECOND		1L
#define			MINUTE		60L
#define			HOUR		(60L * 60L)
#define			DAY		(24L * 60L * 60L)
#define			WEEK		(7L  * 24L * 60L * 60L)
#define			BUFSIZE		1024

/*
 * ch_time - format elapsed time into a character string.
 *
 * Mikel Manitius - 85-01-08 - (mikel@codas.att.com.uucp)
 *
 */

char	*malloc();

char	*day[] =
{
	"Sun", "Mon", "Tue",
	"Wed", "Thu", "Fri",
	"Sat", 0
};

char	*mon[] =
{
	"Jan", "Feb", "Mar", "Apr", "May",
	"Jun", "Jul", "Aug", "Sep", "Oct",
	"Nov", "Dec", 0
};


char *ch_time(sec)
int	sec;
{
	unsigned long	hrs = 0L;
	unsigned long	days = 0L;
	unsigned long	mins = 0L;
	unsigned long	weeks = 0L;
	char	*buff;

	buff = malloc(BUFSIZE);
	if(buff == NULL)
		return(NULL);
	weeks = sec / WEEK;
	sec -= WEEK * weeks;
	days = sec / DAY;
	sec -= DAY * days;
	hrs = sec / HOUR;
	sec -= HOUR * hrs;
	mins = sec / MINUTE;
	sec -= MINUTE * mins;

	if(weeks)
		sprintf(buff, "%d week%s, ", weeks, (weeks == 1) ? "" : "s");
	if(days)
		sprintf(buff, "%s%d day%s, ", (weeks) ? buff : "",
			days, (days == 1L) ? "" : "s");
	if(hrs || days || weeks)
		sprintf(buff, "%s%d hour%s, ", (days || weeks) ? buff : "",
			hrs, (hrs == 1L) ? "" : "s");
	if(mins || hrs || days || weeks)
		sprintf(buff, "%s%d minute%s, ",
			(hrs || days || weeks) ? buff : "",
			mins, (mins == 1L) ? "" : "s");
	sprintf(buff, "%s%s%d second%s.",
			(mins || hrs || days || weeks) ? buff : "",
			(mins || hrs || days || weeks) ? "and " : "",
			sec, (sec == 1L) ? "" : "s");
	return(buff);
}


char *sh_time(sec)
int	sec;
{
	unsigned long	hrs = 0L;
	unsigned long	days = 0L;
	unsigned long	mins = 0L;
	char	*buff;

	buff = malloc(BUFSIZE);
	if(buff == NULL)
		return(NULL);
	days = sec / DAY;
	sec -= DAY * days;
	hrs = sec / HOUR;
	sec -= HOUR * hrs;
	mins = sec / MINUTE;
	sec -= MINUTE * mins;

	sprintf(buff, "%d day%s + %02d:%02d:%02d",
		days,
		(days == 1) ? "" : "s",
		hrs, mins, sec);
	return(buff);
}

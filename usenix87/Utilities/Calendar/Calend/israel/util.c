/* Copyright 1983 - University of Maryland */

#include <signal.h>
#include <ctype.h>
#include "globals.h"

FILE	*popen();
char	*getlogin();
struct passwd	*getpwuid();
int	onintr();

#define MAIL "/usr/bin/mailx -s 'Reminder Service' %s"

char *mon_nm[]={
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
/*
 *
 *	The original settings (listed below) caused no end of trouble to .calrc
 *	files set up to be 'human readable'.  Every occurance of 'Sep' was
 *	changed to 'September' when calend did a '+month' operation. 
 *	bob@plus5. 
 *
 * char *mon_nm[]={
 *	"January","February","March","April","May","June",
 *	"July","August","September","October","November","December"
 */

};

char *relative[]={
    "the day before yesterday", "yesterday", "today", "tomorrow",
    "the day after tomorrow"
};

int mon_len[]={
    31,28,31,30,31,30,31,31,30,31,30,31
};

char *day_nm[]={
    "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};

get_plus_parts()
{
    int num = 0;
    char c;

    offmonth = 0;
    offday = 0;

    if (line[lineptr] != '+') return;

    lineptr++;

    while (isdigit(c = line[lineptr++])) num = num * 10 + c - '0';

    lineptr--;

    if (line[lineptr] == 'd' || line[lineptr] == 'D') {
	lineptr++;
	offday = num;
	return;
    }

    if (line[lineptr] != '/' && line[lineptr] != 'm' && line[lineptr] != 'M') {
	offday = num;
	return;
    }

    offmonth = num;
    num = 0;

    lineptr++;

    while (isdigit(c = line[lineptr++])) num = num * 10 + c - '0';
    lineptr--;

    if (line[lineptr] == 'd' || line[lineptr] == 'D') lineptr++;
    offday = num;
    return;
}

char *datestr(datenum,mtype,forward)
int datenum,mtype,forward; {
    register char dstri[BUFSIZ];
    int ddiff;

    if ((mtype == 3) && (abs(ddiff = reldiff(datenum,tdate,forward)) < 3))
    {
	sprintf(dstri,"%s", relative[ddiff+2]);
	return(dstri);
    }
    if (dtype) {
	if (mtype == 2) sprintf(dstri,"%d",datenum % 100);
	else if (mtype == 1)
	sprintf(dstri,"%s",mon_nm[(datenum / 100 - 1) % 12]);
	else sprintf(dstri,"%s %d",mon_nm[(datenum / 100 - 1) % 12],
				   datenum % 100);
	return(dstri);
    }
    if (mtype == 1) return("");
    return day_nm[datenum % 7];
}

int reldiff(d1,d2,firstmore)
int d1,d2,firstmore;
{
    int count = 0;

    if (firstmore) SWAP(d1,d2);
    while ((d1 != d2) && (count < 3)) {
	count++;
	d1 = add_date(d1,1);
    }
    if (! firstmore) count = 0 - count;
    return(count);
}

int add_date(date, offset)
int date, offset;
{
    int mo, da, sum;

    if (!dtype) {
	sum = date + offset;
	while (sum < 0) sum = sum + 7;
	sum = sum % 7;
	return (sum);
    }

    mo = date / 100;
    da = date % 100 + offset;

    mo = mo % 12;
    if (mo == 0) mo = 12;

    while (da < 1) {
	mo--;
	if (mo == 0) mo = 12;
	da = da + mon_len[mo - 1];
    }

    while (da > mon_len[mo - 1]) {
	da = da - mon_len[mo - 1];
	mo++;
	if (mo == 13) mo = 1;
    }

    return (mo * 100 + da);
}

getnextchar() {
	return(lastchar = line[lineptr++]);
}

create_message(str)
char *str;
{
    register char c, *cstr, nstr[20], m;
    int i,mt;

    while (c = *str++) {
	if (c == '%') {
	    if (((m = *str++) == 'm') || (m == 'M')) mt = 1;
	    else if ((m == 'd') || (m == 'D')) mt = 2;
	    else if ((m == 'r') || (m == 'R')) mt = 3;
	    else {
		str--;
		mt = 0;
	    }
	    switch (c = *str++) {
	    case '1' :
	    	cstr = datestr(dayt1,mt,0);
		break;
	    case '2' :
	    	cstr = datestr(dayt2,mt,1);
		break;
	    case 'c' :
	    case 'C' :
	    	cstr = datestr(tdate,mt,0);
		break;
	    case '%' :
	    	cstr = "%";
		break;
	    default :
	    	cstr = "%";
		str--;
		if (mt) str--;
		break;
	    }
	    while(*mptr++ = *cstr++);
	    mptr--;
	}
	else *mptr++ = c;
    }
    *mptr = '\0';
}

send_mail ()
{
	register char *name = getlogin();
	register FILE *mpipe;
	char linebuf[BUFSIZ];

	if (!name) name = getpwuid(getuid()) -> pw_name;
	sprintf(linebuf,MAIL,name);
	mpipe = popen(linebuf,"w");
	fprintf(mpipe,"%s\n",message);
	pclose(mpipe);
}

run_remind() {
	register char time[BUFSIZ];
	register char restmsg[BUFSIZ];
	int tmp;

	sscanf(message,"%s %[^\n]",time,restmsg);

	strcpy(message,restmsg);	/* get new message for other options */

	if((tmp = fork()) == 0) {
		execl(REMIND,"remind","-f",time,restmsg,0);
		_exit(-999);
	}
	if (tmp == -1) {
		fprintf(stderr,"Help, I'm unforkable!\n");
	}
}

execute_thing()
{
	int pid;

	if ((pid = fork()) == 0) {
		execl("/bin/csh", "csh", "-cf", message, 0);
		perror("Fork of /bin/csh");
		_exit(1);
	}
	else if (pid == -1)
		fprintf(stderr,"Couldn't fork to run '%s'.\n",message);
}

pr_message() {
	printf("%s\n",message);
}

copy_file(from,to)
char *from, *to;
{
    FILE *fromfile, *tofile;
    char fline[BUFSIZ];

    signal(SIGINT,SIG_IGN);

    if ((fromfile = fopen(from,"r")) == NULL) {
	perror(from);
	onintr();
	exit(errno);
    }

    if ((tofile = fopen(to,"w"))  == NULL) {
	perror(to);
	fclose(fromfile);
	signal(SIGINT,onintr);
	return;
    }
    
    while (fgets(fline, sizeof fline, fromfile))
	fputs(fline,tofile);

    fclose(fromfile);
    fclose(tofile);
    signal(SIGINT,onintr);
}

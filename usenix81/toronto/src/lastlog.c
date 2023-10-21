#include <stdio.h>
/*
 * lastlog - determine extent of a previous terminal session
*/

#define equal(a,b)	(!strcmp(a,b))
#define equaln(a,b,n)	(!strncmp(a,b,n))
#define cycle	for(;;)
#define YES	1
#define NO	0
#define READ	"r"
#define ABSOLUTE 0
#define RELATIVE 1
#define END	2

#define SUCCESS	0
#define FAILURE	-1

#define REBOOTTY "~"
#define OLDDATE	"|"

#define ENTRYLEN sizeof entry
#define NAMELEN	8

#include <sys/types.h>
#include <utmp.h>
#include <pwd.h>

FILE	* log;
char	*logfile "/usr/adm/wtmp";
char	loginid[NAMELEN];
int	skip, firstskip, lastskip;
char	goingup, goingdown;
char	srchgood;

time_t crashtime;

#define MAXNUMRANGES 100
int firstnum[MAXNUMRANGES], lastnum[MAXNUMRANGES];
int rangenum;
int numranges;

char	crash, reboot;

struct utmp entry;

int ac;
char **av;
int arg;
#define DEFARGC	3
char	*defargv[DEFARGC];

main(argc, argv)
int argc;
char *argv[];
	{
	int status;

	initialize(argc, argv);
	cycle
		{
		status = nextarg();
		if(status == FAILURE)
			exit(0);
		printf("%-8.8s ", loginid);
		putchar(' ');
		status = findid();
		if(status == FAILURE)
			{
			srchgood = NO;
			if((goingup) && (skip < lastskip))
				printf("-%d-%d not found.\n", skip, lastskip);
			else if(skip == 1)
				printf("not found.\n");
			else
				printf("-%d not found.\n", skip);
			continue;
			}
		else
			srchgood = YES;
		printf("%-8.8s ", entry.ut_line);
		prtime(entry.ut_time);
		printf(" -> ");
		crash = reboot = NO;
		status = findout();
		if(status == FAILURE)
			printf("still logged in.\n");
		else
			{
			if(crash)
				{
				prtime(crashtime);
				printf(" (crash)");
				}
			else
				{
				prtime(entry.ut_time);
				if(reboot)
					printf(" (reboot)");
				}
			putchar('\n');
			}
		}
	}

initialize(argc, argv)
int argc;
char *argv[];
	{
	struct passwd *pwent, * getpwuid();

	if(equal(argv[1], "-w"))
		{
		if(argc <= 2)
			{
			printf("No wtmp file specified.\n");
			exit(1);
			}
		logfile = argv[2];
		argc =- 2;
		argv++;
		argv++;
		}

	if((argc < 2) || ((argc == 2) && (argv[1][0] == '-')))
		{
		pwent = getpwuid(getuid());
		if(pwent == NULL) {
			printf("Who are you?\n");
			exit(1);
		}
		defargv[0] = "lastlog";
		defargv[1] = "-2";
		defargv[2] = pwent->pw_name;
		if(argc == 2)
			defargv[1] = argv[1];
		ac = DEFARGC;
		av = defargv;
		}
	else
		{
		ac = argc;
		av = argv;
		}
	arg = 0;
	rangenum = 0;
	numranges = 1;
	firstnum[0] = lastnum[0] = 1;
	goingup = goingdown = NO;

	log = fopen(logfile, READ);
	if(log == NULL) {
		printf("Can't open %s\n", logfile);
		exit(1);
		}
	}

nextarg()
	{
	int i;
	int status;
	char *lp, *ap;
	char newid;

	if(goingup && srchgood)
		{
		skip++;
		if(skip <= lastskip)
			return(SUCCESS);
		}
	else if(goingdown)
		{
		skip--;
		if(skip >= lastskip)
			return(SUCCESS);
		}
	goingup = goingdown = NO;
	rangenum++;
	newid = NO;
	if(rangenum >= numranges)
		{
		newid = YES;
		rangenum = 0;
		for(;;printf("Bad arg %s\n", av[arg]))
			{
			arg++;
			if(arg >= ac)
				return(FAILURE);
			if(av[arg][0] != '-')
				break;
			status = getnums(&av[arg][1]);
			if(status == SUCCESS)
				{
				arg++;
				break;
				}
			}
		}
	skip = firstskip = firstnum[rangenum];
	lastskip = lastnum[rangenum];
	if(skip < lastskip)
		goingup = YES;
	else if(skip > lastskip)
		goingdown = YES;
	if(!newid)
		return(SUCCESS);
	if(arg >= ac)
		{
		printf("No id list for last arg.\n");
		exit(1);
		}
	strncpy(loginid, av[arg], NAMELEN);
	return(SUCCESS);
	}

findid()
	{
	int nskip;
	int nread;

	nskip = skip;
	myfseek(log, (long) ENTRYLEN, END);
	cycle
		{
		myfseek(log, (long) -(2*ENTRYLEN), RELATIVE);
		nread = fread(&entry, ENTRYLEN, 1, log);
		if(nread == 0)
			return(FAILURE);
		if(equaln(loginid, entry.ut_name, NAMELEN))
			{
			nskip--;
			if(nskip <= 0)
				return(SUCCESS);
			}
		}
	}

prtime(t)
long int t;
	{
	char *asciitime, *tp, *ctime();

	asciitime = ctime(&t);
	for(tp = asciitime; *tp != '\n'; tp++)
		putchar(*tp);
	}

findout()
	{
	int nread;
	char datechange;
	char oldtty[NAMELEN];

	datechange = NO;
	strncpy(oldtty, entry.ut_line, NAMELEN);
	cycle
		{
		nread = fread(&entry, ENTRYLEN, 1, log);
		if(nread == 0)
			return(FAILURE);
		if(equaln(entry.ut_line, oldtty, 8))
			return(SUCCESS);
		if(equaln(entry.ut_line, OLDDATE, 8))
			{
			datechange = YES;
			crashtime = entry.ut_time;
			myfseek(log, (long) ENTRYLEN, RELATIVE);
			continue;
			}
		if(equaln(entry.ut_line, REBOOTTY, 8))
			{
			if(datechange)
				crash = YES;
			else
				reboot = YES;
			return(SUCCESS);
			}
		datechange = NO;
		}
	}

getnums(nump)
char *nump;
	{
	char *cp;
	int nr;
	int fnumtemp[MAXNUMRANGES], lnumtemp[MAXNUMRANGES];
	int numrtemp;
	int i;

	numrtemp = 0;
	cp = nump;
	cycle
		{
		numrtemp++;
		if(numrtemp > MAXNUMRANGES)
			return(FAILURE);
		nr = numrtemp - 1;
		fnumtemp[nr] = atoi(cp);
		if(fnumtemp[nr] <= 0)
			return(FAILURE);
		while((*cp >= '0') && (*cp <= '9'))
			cp++;
		if(*cp == '\0')
			{
			lnumtemp[nr] = fnumtemp[nr];
			break;
			}
		if(*cp == ',')
			{
			lnumtemp[nr] = fnumtemp[nr];
			cp++;
			continue;
			}
		if(*cp != '-')
			return(FAILURE);
		cp++;
		lnumtemp[nr] = atoi(cp);
		if(lnumtemp[nr] <= 0)
			return(FAILURE);
		while((*cp >= '0') && (*cp <= '9'))
			cp++;
		if(*cp == '\0')
			break;
		if(*cp != ',')
			return(FAILURE);
		cp++;
		}

	for(i = 0; i < numrtemp; i++)
		{
		firstnum[i] = fnumtemp[i];
		lastnum[i] = lnumtemp[i];
		}
	numranges = numrtemp;
	return(SUCCESS);
	}

/*
 * Do an intelligent fseek that works well with backward
 * reads by seeking to a block boundary.
 */
myfseek(iop, offset, ptrname)
	register FILE * iop;
	off_t offset;
	int ptrname;
{
	long int lseek(), ftell();
	switch(ptrname) {
	case RELATIVE:
		offset += ftell(iop);
		break;
	case END:
		offset += lseek(fileno(iop), 0L, END);
		break;
	}
	fseek(iop, offset&(long)(~0777), ABSOLUTE);
	ungetc(getc(iop),iop);
	fseek(iop, offset, ABSOLUTE);
}

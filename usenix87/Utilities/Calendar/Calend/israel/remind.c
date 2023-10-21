/* Copyright 1983 - University of Maryland */

/*
 * remind [-f] [-#otlm] [time] ["reminder message"]
 *
 * (where time is in the form or hhmm or +Nm (minutes) or +Nh (hours))
 *
 * Reminds you when you have to leave, and why you have to leave.
 * Remind prompts for input and goes away if you hit return.
 * It nags you like a Nice Jewish Mother should.
 *
 * printing added 19 January 1982	by umcp-cs!andrew
 * "You have to <>" printing added	by umcp-cs!andrew
 * +N[hm] added 13sep83			by umcp-cs!andrew
 * 24hr time understanding added	by umcp-cs!andrew
 * -f option added 25sep83		by umcp-cs!andrew
 *
 * last update->Sun Sep 25 15:01:13 1983
 *
 * this was originally the leave(1) program
 *
 */

#include <stdio.h>
#include <signal.h>

static char	*sccsid = "@(#)remind.c      1.1 (U of MuD) 09/13/83";
char		origlogin[20], thislogin[20];
char		*whenleave;
char		buff[100], buff1[100];
int		ntimes = -1;		/* # of times to bother */

char	*ctime();
char	*getlogin();
int	hoursp();

main(argc, argv)
int	argc;
char	**argv; {
	long	when, tod, now, diff, hours, minutes;
	int	*nv;
	int	atoi();
	int	*localtime();
	char	absolute=0;		/* 1 if 24 hour time */
	char	quiet=0;		/* 1 if not to bitch */

	if(argv[1][0] == '-') {		/* set ntimes */
		ntimes = atoi(argv[1]+1);	/* hoser! give it a pointer! */
		if(argv[1][1] == 'f') {
			quiet=1;	/* invocation from a program */
			argv++;		/* sneakily hack argv */
			*argv = argv[-1]; /* hurrah for no bounds checking! */
			argc--;
			ntimes = -1;
			if(argv[1][0] == '-') {
				ntimes=atoi(argv[1]+1);
				argv++;
				*argv = argv[-1];
				argc--;
			}
		}
		else {
			argv++;		/* sneakily hack argv */
			*argv = argv[-1];
			argc--;		/* don't forget argc, twit! */
		}
	}

	if(argc < 2) {
		printf("When do you have to leave? ");
		fflush(stdout);
		buff[read(0, buff, sizeof buff)] = 0;	/* get answer */

		if(*buff == '\n') exit(0);	/* if \n exit */

		if(!*buff) exit(0);		/* if eof exit */

		printf("What do you have to do? ");
		fflush(stdout);
		buff1[read(0, buff1, sizeof buff1)-1] = 0; /*kill the \n*/
	}

	else if(argc == 2) {			/* remind time */
		printf("What do you have to do? ");
		fflush(stdout);
		buff1[read(0, buff1, sizeof buff1)-1] = 0; /*kill the \n*/

		if(*buff1 == '\n') exit(0);

		strcpy(buff, argv[1]);
	} else {			/* remind time "message" */
		strcpy(buff, argv[1]);
		strcpy(buff1, argv[2]);
	}

	strcpy(origlogin, getlogin());	/* do getlogin here so we won't barf */

	if(*buff == '+') {
		diff = atoi(buff+1);
		if(hoursp(buff+1)) diff *= 60;	/* 60 mins/hour */
		doalarm(diff);
	}

	if(*buff < '0' || *buff > '9') {
		if(!quiet) printf(
			"usage: %s [ -#otlm ] [time] [\"reminder-message\"]\n",
			*argv);
		exit(1);
	}

	tod = atoi(buff);
	hours = tod / 100;

	/*
	 * times like 2300 or 0730 don't get frobbed to the nearest 12 hours
	 *
	 */

	if(hours > 12) absolute = 1;
	if(*buff == '0') absolute = 1;

	if(!absolute && hours == 12) hours = 0;
	minutes = tod % 100;

	if(hours < 0 || hours > (absolute ? 24 : 12)
	   || minutes < 0 || minutes > 59) {
		if(!quiet) printf(
			"usage: %s [ -#otlm ] [time] [\"reminder message\"]\n",
			*argv);
		exit(1);
	}

#ifdef	VAX			/* this doesn't work under v7, sigh */
				/* and i don't think it's worth re-writing */
	setexit();		/* refigure time if killed */
#endif	VAX

	time(&now);
	nv = localtime(&now);
	when = (60 * hours) + minutes;
	if(!absolute && nv[2] > 12) nv[2] -= 12;	/* do am/pm bit */
	now = (60 * nv[2]) + nv[1];
	diff = when - now;

	if (now > when && absolute) {
	    if (! quiet) printf("It is already past that time.\n");
	    exit(0);
	}

	while(diff < 0)
		diff += (absolute ? 24 : 12) * 60;

	if((diff > (absolute ? 23 : 11) * 60) && !quiet) printf(
	    "That time has recently passed, but I'll remind you anyway.\n");

	doalarm(diff);
	exit(0);
}

doalarm(nmins)
long	nmins; {
	char		*msg1, *msg2, *msg3, *msg4;
	register int	i;
	long		slp1, slp2, slp3, slp4;
	long		seconds, gseconds;
	long		daytime;
	int		pid;

	seconds = 60 * nmins;

	if(seconds <= 0) seconds = 1;

	gseconds = seconds;

	msg1 = "You have to %s in 5 minutes!";

	if(seconds <= 60*5) {
		slp1 = 0;
	} else {
		slp1 = seconds - 60*5;
		seconds = 60*5;
	}

	msg2 = "Just one more minute before you have to %s!";

	if(seconds <= 60) {
		slp2 = 0;
	} else {
		slp2 = seconds - 60;
		seconds = 60;
	}

	msg3 = "Time to leave and %s!";
	slp3 = seconds;

	msg4 = "You're going to be too late to %s!";
	slp4 = 60;

	time(&daytime);
	daytime += gseconds;
	whenleave = ctime(&daytime);
	printf("You have to %s at %s", buff1, whenleave);
	if(pid=fork()) {
		if(pid == -1) {
			perror("can't fork");
			exit(1);
		}

		exit(0);
	}

	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif

	if(slp1) bother(slp1, msg1);
	if(slp2) bother(slp2, msg2);

	bother(slp3, msg3);

	if(ntimes != -1) {
		for(;ntimes--;) bother(slp4, msg4);
		exit(0);
	}

	for(;;) {
		bother(slp4, msg4);		/* be annoying */
	}
}

bother(slp, msg)
long	slp;
char	*msg; {

	delay(slp);
	printf("\7\7\7");
	printf(msg, buff1);
	printf("\r\n");
}

/*
 * delay is like sleep but does it in 100 sec pieces and
 * knows what zero means.
 */
delay(secs)
long	secs; {
	int	n;

	while(secs > 0) {
		n = 100;
		secs = secs - 100;

		if(secs < 0) {
			n = n + secs;
		}

		if(n > 0) sleep(n);

		strcpy(thislogin, getlogin());
		if(strcmp(origlogin, thislogin)) exit(0);
	}
}

#ifdef V6
char *getlogin() {
#include <utmp.h>

	static struct utmp ubuf;
	int ufd;

	ufd = open("/etc/utmp", 0);
	seek(ufd, ttyn(0)*sizeof(ubuf), 0);
	read(ufd, &ubuf, sizeof(ubuf));
	ubuf.ut_name[sizeof(ubuf.ut_name)] = 0;
	return(&ubuf.ut_name);
}
#endif

/*
 * return 1 if s is in the form of +[0-9]*h, 0 otherwise
 *
 */

int	hoursp(s)
char	*s; {
	char	numflg = 0;

	if(!*s) return 0;

	while(*s >= '0' && *s <= '9') s++;	/* skip the number */

	if(*s == 'h') return 1;

	return 0;
}

#
/*
 *      ksys -- program to count down time until shutdown will occur.
 *
 *              ksys [-minutes]
 *      or,
 *              ksys [hh:mm]
 *
 *      where minutes is the time in minutes until shutdown
 *      and hh:mm is the time for shutdown.
 *
 *      if no agrument then this will send log off message.
 *      otherwise will count the minutes until shutdown, sending
 *      encouraging messages to the users.
 *
 *      following  this it will execute the file /etc/cleanup
 *      as a shell procedure.
 *
 *      then it will send a hangup signal to /etc/init causing all
 *      active terminals to be terminated.
 *
 *      then it will kill off all background processes etc except
 *      those owned by daemon.
 *
 *      ksys with no shutdown time specified gives:
 *              if superuser: cancel any present ksys
 *              if normal user: give time to shutdown if ksys in progress
 *
 *      assumes there is a broadcaster program known as /etc/sdown
 *
 *
 *      Dennis L. Mumaugh
 *      who  vers       when            what/why
 *      dlm  1.0    18 May 1976
 *      dlm  2.0    26 Aug 1976
 *      dlm  2.1    07 Jan 1977
 *      dlm  2.2    15 Feb 1977     Add shut switch coding
 *      dlm  2.3    21 Sep 1977     Run /etc/cleanup vice specific coding
 *      dlm  2.4    09 Nov 1977     Clean wait problem; add options to
 *                                  cancel previous ksys and to tell how long
 */

char ksys_id[] "~|^`ksys.c:\t2.4\t9-Nov-1977"; /* configuration information */
#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "login.h"
#define SINGLE 0173030
#define DAEMID 01
char tsmsg1[] "Time sharing will cease in %d minutes\n";
char tsmsg2[] "Time sharing will cease in %d minute\n";
char tsmsg3[] "\n\r\07Time sharing will cease in %d minutes\07\n\r";
char tsmsg4[] "\n\r\07Time sharing will cease in %d minute\07\n\r";
/*
 *      configuration instructions:
 *
 *      if you are using the RAND unix (bobg)
 *      define the variable BOBG
 *      and the conditional compilation will not include
 *      the code to read the process tables
 *
 */
#define BOBG    1

int *localtime();
char *tp;
struct proc proc[NPROC];
struct ksys ksys;       /* defined in login.h */

main(narg,arg)
int narg;
char *arg[];
{
	register struct proc *p;
	int stime, ttime;
	long tvec;
	int h, m;
	int pid;
	int fd;         /* file descriptor for reading ksys time file */
	extern int interrupt();

	time(&tvec);
	signal(1,1);           /* hangup */
	signal(2,1);           /* interrupt (rubout) */
	signal(3,&interrupt);  /* quit  */
	signal(14,&interrupt); /* alarm */
	chdir("/");            /* root file system */
	if ( narg > 1 ) {
		if ( getuid() != 0) {
			printf("Ordinary mortals cannot turn-off timesharing.\n");
			exit(-1);
		}
		tp =  arg[1];
		if(*tp == '-'|| *tp == '+') {
			tp++;
			ttime = atoi(tp);
		}
		else {
			h = gpair();
			if (*tp == ':') tp++;
			m = gpair();
			if(h<0 || m<0) exit(printf("argument error\n"));
			if(h>23 || m>59) exit(printf("impossible time\n"));
			ttime = h*60 + m;  /* time of day to cease operating */
			h = localtime(&tvec)[2]; /* time of day in hours */
			m = localtime(&tvec)[1]; /* time of day in minutes */
			ttime = ttime - (h*60 + m);
			if (ttime <0 )  {
				printf("You are trying to shut the system ");
				printf("down before you started.\nTry a ");
				printf("later quitting time.\n");
				exit(-1);
			}
		}
		/*
		 *      write file of information so other people
		 *      can find out what is happening.
		 */
		fd = open (KSYST,0);
		if (fd > 0) {
			printf("It looks as if another ksys is running.");
			printf("\nFile %s exists\n",KSYST);
			exit(-1);
		}
		fd = creat (KSYST,0644);
		ksys.dtime = tvec +ttime*60;
		ksys.kpid = getpid();
		write(fd,&ksys, sizeof ksys);
		close(fd);

		printf("Time until shutdown: %d minutes\n",ttime);
		if (getcsw() != SINGLE)
			printf("\007Change console switches!\007\n");
		if(ttime>60) {
			sleep((ttime-60)*60);
			ttime = 60;
		}
		close(1);
		open("/dev/tty8",1);
		while ( ttime>0 ){
			if (ttime < 5 ) creat(SHUTSW,0444);
			if (ttime > 1 ) tp = stringf(tsmsg3,ttime);
				  else  tp = stringf(tsmsg4,ttime);
			if ( fork() == 0 )
			{
				execl("/etc/sdown","/etc/sdown",tp,0) ;
				exit();
			};
			stime = ttime > 2  ?   ttime/2  : 1 ;
			ttime =- stime;
			sleep ( 60*stime );
		}
	}
	else {
		fd = open(KSYST,0);
		if(fd < 0 ) {
			printf("No shutdown is scheduled.\n");
			exit();
		}
		if( read(fd,&ksys, sizeof ksys) != sizeof ksys) {
			exit(-1);
		}
		close(fd);
		if( getuid() == 0) { /* root */
			kill(ksys.kpid,9);
			interrupt();
			exit();
		}
		else { /* mortal being */
			ttime = ksys.dtime - tvec;
			printf(tsmsg1,ttime/60);
			exit();
		}
	}
	if ( fork() == 0)
		execl("/etc/sdown",0);
	sleep(15);           /* wait for chicken little to finish */
	if (getcsw() != SINGLE)
		printf("\007Change console switches!\007\n");
	creat(SHUTSW,0444);

	/* run /etc/cleanup */
	if ( fork() == 0 )
	{
		printf("cleanup and do system houskeeping\n");
		chdir("/etc");
		execl("/bin/sh","cleanup","cleanup",0) ;
		printf("\007Cannot execute /etc/cleanup\007\n");
		exit();
	}
	sleep(30); /* a bit of time for cleanup */

	/* kill terminals */
	while ( getcsw() != SINGLE )  {
		printf("\007ksys:Change console switches!\n\007");
		sleep(30);
	}
	kill(1,1);  /* hangup to /etc/init */
	sleep(15);
	fixfiles();
	nice(-120);
	pid = getpid();
#ifdef BOBG
	if (gprocs(0) != NPROC) err("NPROC mismatch--recompile ksys.c\n");
	gprocs(proc);
#else
/*
 *      get proccess tables
 *      simulates the RAND gprocs system call
 */

	setup(&nl[0], "_proc");
	setup(&nl[1], "_swapdev");
	nlist("/unix", nl);
	printf("name = %.8s type = %o value = %l\n",nl[0].name,
			nl[0].type,nl[0].value);
	if (nl[0].type==0) {
	printf("name = %.8s type = %o value = %l\n",nl[0].name,
			nl[0].type,nl[0].value);
		printf("No namelist\n");
		return;
	}
	coref = "/dev/mem";
	if ((mem = open(coref, 0)) < 0) {
		printf("No mem\n");
		exit(~0);
	}
	seek(mem, nl[1].value, 0);
	read(mem, &nl[1].value, 2);
	seek(mem, nl[0].value, 0);
	read(mem, proc, sizeof proc);
	close(mem);
#endif
	for(p = &proc[0]; p < &proc[NPROC]; p++) {
		if(p->p_stat == 0 ) continue;  /* unused slot */
		if(p->p_pid == pid) continue; /* don't kill myself */
		if(p->p_pid <= 1) continue; /* don't kill vital people */
		if(p->p_uid == DAEMID) continue;  /* nor daemons */
		kill(p->p_pid,9);
	}
	sleep(15);
	printf("There will be a short wait while all of the background\n");
	printf("processes decide to die.  Don't be impatient.\n");
	sleep(60);
	while(wait()>0);
	sleep(15);
	sync();
	printf("\007System is shut down\n\007");
	printf("To prove it type 'ss'.\n");
	exit(0);  /* finished */
}

interrupt(){
	fixfiles();
	exit();
}

fixfiles(){
	unlink(SHUTSW);
	unlink(KSYST);
}

gpair()
{
	register int c, d;

	if(*tp == 0)
		return(-1);
	c = (*tp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*tp == 0)
		return(-1);
	if ((d = *tp++ - '0') < 0 || d > 9)
		return(-1);
	return (c+d);
}
err(s)
{
	printf(s);
	flush();
	exit(1);
}
#ifndef BOBG
setup(p, s)
char *p, *s;
{
	while (*p++ = *s++);
}
#endif

/*
 *	stringf - print into string using stdio sprintf
 */

stringf(f, a)
char *f;
{
	static char string[160];

	return(sprintf(string, f, a));
}

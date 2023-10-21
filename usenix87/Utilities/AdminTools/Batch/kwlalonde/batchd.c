/*
 *  Batch daemon
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <stdio.h>
#include <nlist.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include "lex.h"
/*
 * For 4.2bsd signals.
 */
#define	mask(s)		(1 << ((s)-1))
#define	sighold(s)	sigblock(mask(s))
#define	sigrelse(s)	sigsetmask(sigblock(0)&~mask(s))

#undef DIRSIZ
#define	DIRSIZ	32	/* Used to be 14; could be MAXNAMLEN... -IAN! */

#define SPOOLDIR	"/usr/spool/batch"
#define	PROFILE		"profile"
char Pid[] = "/usr/spool/pid/batchd";
#define	NQUEUES		16	/* Max # of queues */
#define	MAXENTRIES	50	/* Max # per queue */
#define MAXTOTAL	50	/* Max # of total jobs */
#ifdef DEBUG
#define DEBUGFILE	"/usr/spool/batch/troff/debug"
#define	NSEC	10
#define STATIC
#else
#define	NSEC	(15*60)
#define STATIC	static
#endif

STATIC int nkids;

STATIC struct stat dot, odot;	/* Start of spooldir */
STATIC int alarmed;

typedef char MAIL;
#define	MAIL_START	1
#define	MAIL_END	2
#define	MAIL_CRASH	4

typedef struct {
	char	f_name[DIRSIZ+1];
	struct stat f_stat;
	int	f_pid;
	char	f_seen;
} CF_File;

#define NRLIM 6		/* Number of resources, e.g. RLIMIT_CPU */

typedef struct {
	char	q_name[DIRSIZ+1];
	CF_File q_files[MAXENTRIES];/* Sorted list of files */
	int	q_running;	/* If not set, queue is not active */
	int	q_idle;		/* If set, queue doesn't have anything in it */
	int	q_drain;	/* Set if shutdown started */
	int	q_abort;	/* Set if waiting for aborting */
	int	q_stopped;	/* Set if queue stopped for lower load */
	struct	stat q_stat, q_ostat;	/* Stat of queue directory */
	struct	stat q_prostat, q_oprostat; /* Stat of queue profile file */
	char	*q_supervisor;	/* Queue supervisor */
	MAIL	q_supmail;	/* Type of mail for supervisor */
	MAIL	q_usermail;	/* Type of notification for user */
	int	q_maxexec;	/* Max # of executing jobs */
	int	q_nexec;	/* Current # of executing jobs */
	int	q_loadsched;	/* Stop scheduling new jobs if over this load */
	int	q_loadstop;	/* Stop jobs if over this load */
	int	q_nice;		/* Job priority */
	int	q_restart;	/* Restart on system crash? */
	int	q_pgroup;	/* Process group for this queue */
	struct rlimit q_rlimit[NRLIM];	/* Resource hard/soft limits */
#ifdef NOT_IMPLEMENTED
	/* Remainder not yet implemented */
	struct {
		time_t	q_starttime;	/* Start queue at */
		time_t	q_endtime;	/* End queue at */
	} q_times[5];
	char	q_days[7];	/* Day to activate on, sun = 0 */
#endif NOT_IMPLEMENTED
} Queue;
STATIC Queue queues[NQUEUES];

typedef struct {
	int pid;
	Queue *rqp;
	CF_File *rfp;
} Running;
STATIC Running running[MAXTOTAL];

STATIC int ignore[] = { SIGINT, SIGHUP, SIGQUIT, SIGSYS, SIGPIPE, SIGTERM };

extern errno;
extern char *calloc(), *strcpy(), *strncpy();

main()
{
	register Queue *qp;
	int alarming(), reapchild();
	char fname[DIRSIZ*2];
	int i, dotu;
	FILE *f;
	extern char **environ;
	static char *env[] = { "PATH=/bin", 0 };

#ifndef DEBUG
	if(getuid() != 0)
		error("Real uid is %d, not root\n", getuid());
	if(geteuid() != 0)
		error("Effective uid is %d, not root\n", geteuid());
#endif
	if(chdir(SPOOLDIR) < 0)
		pxerror(SPOOLDIR);
	dotu = open(".", O_RDONLY, 0);
	if (dotu < 0)
		pxerror("open .");
	if(fstat(dotu, &dot) < 0)
		pxerror("stat of dot");
#ifndef DEBUG
	if(dot.st_uid != 0)
		error("%s: Not owned by root\n", SPOOLDIR);
#endif
	if(dot.st_mode & (S_IWRITE>>6))
		error("%s: Mode %03o allows general write\n", SPOOLDIR, dot.st_mode);
#ifndef DEBUG
	
	/* Log my pid */
	if (access(Pid, R_OK) == 0)
		error("%s exists.  Batchd probably already running. Please check.\n", Pid ) ;
	if (f = fopen(Pid, "w")) {
		fprintf(f, "%d\n", getpid());
		fclose(f);
	}
	for (i = 0; i < sizeof ignore/sizeof ignore[0]; i++)
		signal(ignore[i], SIG_IGN);
	nice(-40);
	nice(20);
	nice(0);
	/*
	 *  Get rid of controlling tty
	 */
	if((i = open("/dev/tty", 2)) != -1) {
		ioctl(i, TIOCNOTTY, 0);
		close(i);
	}
#endif

	/*
	 *  Get rid of most current environment variables.
	 */
	environ = env;

	signal(SIGALRM, alarming);
	signal(SIGCHLD, reapchild);
	for(;;) {
		/*
		 *  Check if the main spool directory changed.  If so, make
		 *  a new list of potential queue entries.
		 */
		if(fstat(dotu, &dot) < 0)
			pxerror("statting dot");
		if(dot.st_mtime > odot.st_mtime) {
			readqueues();
			odot = dot;
		}
		/*
		 *  Now check each potential queue entry.
		 */
		for(qp = queues ; qp < &queues[NQUEUES] ; qp++) {
			if(qp->q_name[0] == 0)
				continue;
			/*
			 *  Check if its profile file changed; if so re-read it.
			 *  Be generous, if somebody deleted it don't affect
			 *  the queue or whats runing.
			 */
			sprintf(fname, "%s/%s", qp->q_name, PROFILE);
			if(stat(fname, &qp->q_prostat) < 0) {
				pmerror(fname);
				continue;
			}
			if(qp->q_prostat.st_mtime > qp->q_oprostat.st_mtime) {
				readpro(qp);
				qp->q_oprostat = qp->q_prostat;
			}
			/*
			 *  If profile file said exec off, ignore.
			 *  If still draining then we have to call runqueue
			 *  still for purposes of starting and stopping
			 */
			if(qp->q_drain) {
				runqueue(qp);
				continue;
			}
			if(qp->q_running == 0)
				continue;
			/*
			 *  Now check the directory itself.  If it changed,
			 *  then something new might have been queued.
			 */
			if(stat(qp->q_name, &qp->q_stat) < 0) {
				pmerror(fname);
				continue;
			}
			if(qp->q_stat.st_mtime > qp->q_ostat.st_mtime) {
				requeue(qp);
				qp->q_ostat = qp->q_stat;
			}
			/*
			 *  Now, start something running in queue if it can.
			 */
			runqueue(qp);
		}
		/*
		 *  Now wait for children until either there are none; or
		 *  NSEC seconds have gone by.
		 */
		alarm(NSEC);
		alarmed = 0;
		while (!alarmed)
			pause();
		alarm(0);
#ifdef DEBUG
		muser(DEBUGFILE, "looking around...\n");
#endif DEBUG
	}
}

STATIC
reapchild()
{
	union wait status;
	int pid;

	while ((pid = wait3(&status, WNOHANG, (struct rusage *)0)) > 0) {
#ifdef DEBUG
		muser(DEBUGFILE, "wait3 exit pid=%d, stat=%o\n",
			pid, status.w_status);
#endif DEBUG
		terminated(pid, status.w_status);
	}
}

STATIC
alarming()
{
	++alarmed;
#ifdef DEBUG
	muser(DEBUGFILE, "How alarming\n");
#endif DEBUG
}

/*VARARGS1*/
STATIC
error(s)
char *s;
{
	FILE *mail, *sendmail();

	mail = sendmail("batch");
	fprintf(mail, "Fatal error; batch terminating\n%r", &s);
	mailclose(mail);
#ifdef DEBUG
	abort();
#else
	exit(1);
#endif
}

#if 0
/*VARARGS1*/
STATIC
panic(s)
char *s;
{
	error("%r", &s);
}
#endif 0

extern int	sys_nerr;
extern char	*sys_errlist[];
STATIC
pxerror(s)
char *s;
{
	register char *c;

	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	error("%s: %s\n", s, c);
}

STATIC
pmerror(s)
char *s;
{
	register char *c;
	FILE *mail, *sendmail();
	int err;

	err = errno;
	mail = sendmail("batch");
	c = "Unknown error";
	if(err < sys_nerr)
		c = sys_errlist[err];
	fprintf(mail, "%s: %s\n", s, c);
	mailclose(mail);
}

/*VARARGS1*/
STATIC
merror(s)
char *s;
{
	FILE *mail, *sendmail();

	mail = sendmail("batch");
	fprintf(mail, "%r", &s);
	mailclose(mail);
}

/*VARARGS2*/
STATIC
muser(s, m)
char *s, *m;
{
	FILE *mail;
	FILE *sendmail();
	time_t now, time();
	char *ctime();

	if(s == NULL)
		return;
	if(*s == '/') {
		if((mail = fopen(s, "a")) == NULL) {
			pmerror(s);
			mail = stderr;
		}
		now = time(0);
		fprintf(mail, "%-15.15s ", ctime(&now) + 4);
	} else
		mail = sendmail(s);
	fprintf(mail, "%r", &m);
	mailclose(mail);
}

#if 0
STATIC FILE *
sendmail(s)
{
	fprintf(stderr, "%s: ", s);
	return stderr;
}
STATIC
mailclose(file)
FILE *file;
{
	if(file != stderr)
		fclose(file);
}
#else
STATIC FILE *
sendmail(s)
char *s;
{
	FILE *mailer;
	int pid;
	struct {int read; int write;} mailp;
	register Running *rp;

	if(pipe(&mailp) < 0) {
	err:
		fprintf(stderr, "%s: ", s);
		return stderr;
	}
	switch(pid = vfork()) {
	case 0:
		dup2(mailp.read, fileno(stdin));
		close(mailp.write);
		nice(4);	/* lower priority */
		execl("/usr/lib/sendmail", "sendmail", s, (char *)0);
		perror("BATCHD execl sendmail!");
		exit(1);
	case -1:
		goto err;
	default:
		close(mailp.read);
	}
	nkids++;
	for(rp = running ; rp < &running[MAXTOTAL] ; rp++)
		if(rp->pid == 0) {
			rp->pid = pid;
			rp->rqp = NULL;
			rp->rfp = NULL;
			break;
		}
	if(rp >= &running[MAXTOTAL])
		fprintf(stderr, "BATCHD sendmail: too many jobs\n");
	mailer = fdopen(mailp.write, "w");
	if(mailer == NULL) {
		close(mailp.write);
		perror("BATCHD fdopen");
		goto err;
	}
	fprintf(mailer, "To: %s\nFrom: batchd (Batch Daemon)\n\n", s);
	return mailer;
}

STATIC
mailclose(f)
FILE *f;
{
	fclose(f);
}
#endif

/*
 *  Read the home directory, looking for all current queue entries.
 */
STATIC
readqueues()
{
	register Queue *qp, *nextq;
	register i;
	static DIR *dotp;
	struct direct *d;
	char seen[NQUEUES];
	struct stat sb;
	register CF_File *fp;

	if(dotp == NULL) {
		if((dotp = opendir(".")) == NULL)
			pxerror("opendir dot");
	} else
		rewinddir(dotp);
	bzero(seen, sizeof seen);
	while((d = readdir(dotp)) != NULL) {
		/*
		 *  Ignore everything starting with a dot.
		 */
		if(*d->d_name == '.')
			continue;
		if(stat(d->d_name, &sb) == -1) {
			pmerror(d->d_name);
			continue;
		}
		if((sb.st_mode & S_IFMT) != S_IFDIR) {
			errno = ENOTDIR;
			pmerror(d->d_name);
			continue;
		}
		/*
		 *  Found a directory.
		 */
		nextq = NULL;
		for(qp = queues ; qp < &queues[NQUEUES] ; qp++) {
			if(strcmp(d->d_name, qp->q_name) == 0)
				break;
			if(nextq == NULL  &&  qp->q_name[0] == 0)
				nextq = qp;
		}
		/*
		 *  Was it a new queue?
		 */
		if(qp >= &queues[NQUEUES]) {
			if(nextq == NULL) {
				merror("More than %d queues at %s\n",
					NQUEUES, d->d_name);
				continue;
			}
			qp = nextq;
			strncpy(qp->q_name, d->d_name, DIRSIZ);
			qp->q_nexec = 0;
			qp->q_stopped = 0;
			qp->q_ostat.st_mtime = qp->q_oprostat.st_mtime = 0;
			for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++)
				fp->f_name[0] = 0;
		}
		seen[qp-queues] = 1;
	}
	/*
	 *  Any directories which have been deleted imply the queues should
	 *  end violently since we can't get the output files..
	 */
	for(i = 0 ; i < NQUEUES ; i++)
		if(seen[i] == 0  &&  queues[i].q_name[0]) {
			if(qp->q_nexec) {
				abortall(&queues[i]);
				qp->q_abort = 1;
			} else {
				qp->q_name[0] = 0;
				qp->q_pgroup = 0;
			}
		}
}

/*
 *  Read a profile file under the given queue entry.
 */
STATIC
readpro(qp)
register Queue *qp;
{
	static struct rlimit minus_one = { -1, -1 };
	register int i;
	int run;
	FILE *pro;
	char pname[DIRSIZ*2];
	enum keyword yylex();
	enum keyword k;
	extern char yytext[];
	MAIL *mailp;

	/*
	 *  First set up queue to all defaults so if we re-read a file
	 *  we don't get the old values.
	 */
	qp->q_supervisor = NULL;
	qp->q_usermail = qp->q_supmail = MAIL_END|MAIL_CRASH;
	qp->q_nice = 0;
	qp->q_restart = 0;
	for( i=0; i<NRLIM; i++ )
		qp->q_rlimit[i] = minus_one;
#ifdef NOT_IMPLEMENTED
	qp->q_times[0].q_starttime = 0;
	qp->q_days[0] = qp->q_days[1] = qp->q_days[2] = qp->q_days[3] =
		qp->q_days[4] = qp->q_days[5] = qp->q_days[6] = 0;
#endif NOT_IMPLEMENTED
	qp->q_drain = qp->q_abort = 0;
	qp->q_maxexec = 1;
	qp->q_idle = 0;
	qp->q_loadsched = 25;	/* Incredibly high! if not specified */
	qp->q_loadstop = 50;
	if(qp->q_pgroup == 0) {
		register Queue *qp1;
		static int nextpg = 31100;	/* Random number higher than
						   system allocates for procs*/
		again:
		for(qp1 = queues ; qp1 < &queues[NQUEUES] ; qp1++)
			if(qp1->q_pgroup == nextpg) {
				nextpg++;
				goto again;
			}
		qp->q_pgroup = nextpg++;
		if(nextpg > 31200)
			nextpg = 31100;
	}
	run = 1;	/* Default is start running */

	/*
	 *  Try to read the profile file.  If not there, assume shutdown this
	 *  queue.
	 */
	sprintf(pname, "%s/%s", qp->q_name, PROFILE);
	if((pro = fopen(pname, "r")) == NULL) {
		pxerror(pname);
		rundown(qp);
		return;
	}
	lexfile(pro);
#define	syntax	goto syntaxerr
#define	lex	(k = yylex())
	while((int)lex) {
		register int i;
		register int rl;
		register int limit;
		switch(k) {
		case K_LINE:
			break;
		case K_EXEC:
			switch(lex) {
			case K_OFF:
				run = 0;
				break;
			case K_LINE:
			case K_ON:
				run = 1;
				break;
			case K_DRAIN:
				run = 0;
				qp->q_drain = 1;
				break;
			default:
				syntax;
			}
			break;
		case K_MAXEXEC:
			if(lex != K_NUMBER)
				syntax;
			qp->q_maxexec = atoi(yytext);
			break;
		case K_SUPERVISOR:
			if(lex != K_VARIABLE)
				syntax;
			qp->q_supervisor = calloc(1, strlen(yytext)+1);
			strcpy(qp->q_supervisor, yytext);
			break;
		case K_MAIL:
			mailp = &qp->q_usermail;
		mailstuff:
			*mailp = 0;
			if(lex == K_LINE)
				break;
			if(k != K_VARIABLE)
				syntax;
			if(index(yytext, 's'))
				*mailp |= MAIL_START;
			if(index(yytext, 'e'))
				*mailp |= MAIL_END;
			if(index(yytext, 'c'))
				*mailp |= MAIL_CRASH;
			break;
		case K_MAILSUPERVISOR:
			mailp = &qp->q_supmail;
			goto mailstuff;
		case K_LOADSCHED:
			if(lex != K_NUMBER)
				syntax;
			qp->q_loadsched = atoi(yytext);
			break;
		case K_LOADSTOP:
			if(lex != K_NUMBER)
				syntax;
			qp->q_loadstop = atoi(yytext);
			break;
		case K_NICE:
			if(lex != K_NUMBER)
				syntax;
			qp->q_nice = atoi(yytext);
			break;
		case K_RESTART:
			qp->q_restart = 1;
			break;
		case K_CPULIMIT:
		case K_RLIMITCPU:
		case K_RLIMITFSIZE:
		case K_RLIMITDATA:
		case K_RLIMITSTACK:
		case K_RLIMITCORE:
		case K_RLIMITRSS:
			/* If only one number is given, we set only the
			 * current limit.  If two numbers, we set current and
			 * maximum limits.   -IAN!
			 */
			i = rltoi(rl = Ktorl(k));/* index in q_rlimit array */
			if(lex != K_NUMBER)
				syntax;
			limit = atoi(yytext);
			if(lex == K_LINE){
				if(getrlimit(rl,&(qp->q_rlimit[i])) != 0){
					pmerror("getrlimit(%d,%d)\n",
					    rl,&(qp->q_rlimit[i]));
					break;
				}
			}
			qp->q_rlimit[i].rlim_cur = limit;
			if(k == K_LINE)
				break;
			if(k != K_NUMBER)
				syntax;
			qp->q_rlimit[i].rlim_max = atoi(yytext);
			break;
		default:
			syntax;
		}
		if(k != K_LINE)
			if(yylex() != K_LINE)
				syntax;
	}
	fclose(pro);
	if(run == 0)
		if(qp->q_drain)
			rundown(qp);
		else
			abortall(qp);
	qp->q_running = run;
	return;

syntaxerr:
	merror("%s: Syntax error near %s\n", qp->q_name, yytext);
	rundown(qp);
}

/*
 *  Read the directory for a queue and find all jobs ready to run.
 */
STATIC
requeue(qp)
register Queue *qp;
{
	DIR *qdir;
	struct direct *d;
	char procname[50];
	register CF_File *fp, *nextf;
	int cmp();
	static int startup = 1;

	for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++)
		fp->f_seen = 0;
	if((qdir = opendir(qp->q_name)) == NULL) {
		pmerror(qp->q_name);
		rundown(qp);
		return;
	}
	while((d = readdir(qdir)) != NULL) {
		if(strncmp(d->d_name, "cf", 2) != 0)
			continue;
		nextf = NULL;
		for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++) {
			if(fp->f_name[0] == 0) {
				/* Found the first empty slot */
				if(nextf == NULL)
					nextf = fp;
				continue;
			}
			if(strcmp(d->d_name, fp->f_name) == 0)
				break;
		}
		/*
		 * If we ran off the end of the table, we didn't
		 * find it and it must be a new entry.
		 */
		if(fp >= &qp->q_files[MAXENTRIES]) {
			if(nextf == NULL) {
				muser(qp->q_supervisor,
				   "%s: More than %d entries; %s ignored\n",
				   qp->q_name, MAXENTRIES, d->d_name);
				continue;
			}
			sprintf(procname, "%s/%s", qp->q_name, d->d_name);
			fp = nextf;
			if(stat(procname, &fp->f_stat) == -1) {
				pmerror(procname);
				continue;
			}
			strncpy(fp->f_name, d->d_name, DIRSIZ);
			fp->f_pid = 0;
		}
		fp->f_seen = 1;
	}
	/*
	 * Look at our internal table and check that every job we
	 * know about is still in the directory.  If one has been
	 * deleted and is currently running, abort it; otherwise
	 * just remove the job from the table.
	 */
	for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++){
		if(fp->f_seen == 0  &&  fp->f_name[0])
			if(fp->f_pid)
				abortjob(fp);
			else
				fp->f_name[0] = 0;
	}
	/*
	 * Check for restarted jobs.  This is done exactly once, when
	 * BATCHD gets started after a reboot.
	 */
	if(startup) {
		startup = 0;
		rewinddir(qdir);
		while((d = readdir(qdir)) != NULL) {
			if(strncmp(d->d_name, "of", 2) != 0)
				continue;
			for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++)
				if(strcmp(d->d_name+1, fp->f_name+1) == 0) {
					char cfname[DIRSIZ*2];
					mailback(0, qp, fp, MAIL_CRASH,
					qp->q_restart?"restarted":"not restarted");
					if(qp->q_restart)
						break;
					/*
					 *  If restart disallowed, delete file
					 */
					sprintf(cfname, "%s/%s", qp->q_name, fp->f_name);
					if(unlink(cfname) == -1) {
						int e = errno;

						pmerror(cfname);
						muser(qp->q_supervisor,
						    "%s: Can't unlink dead job%s",
						    qp->q_name,
							e==ENOENT? "":
							"; stopping queue");
						if (e != ENOENT)
							rundown(qp);
					}
					fp->f_name[0] = 0;
					break;
				}
			/*
			 *  Left over output files with no control files??
			 */
			if(fp >= &qp->q_files[MAXENTRIES]) {
				char ofname[DIRSIZ*2];
				sprintf(ofname, "%s/%s", qp->q_name, d->d_name);
				unlink(ofname);
				muser(qp->q_supervisor, "%s: Old output file deleted\n", ofname);
			}
		}
	}
	closedir(qdir);
	qp->q_idle = 0;
}

/*
 *  A queue that might have changed.
 *  Wander along it and start jobs if possible.
 *  If we couldn't fork we just return; in 30 seconds we'll try again.
 */
STATIC
runqueue(qp)
register Queue *qp;
{
	register CF_File *fp;
	register CF_File *bestfp;
	int load5;

	load5 = load(1);
	if(qp->q_stopped  &&  load5 < qp->q_loadstop) {
		startall(qp);
		qp->q_stopped = 0;
	}
	if(qp->q_stopped == 0  &&  load5 >= qp->q_loadstop) {
		stopall(qp);
		qp->q_stopped = 1;
	}
	if(qp->q_stopped)
		return;
	if(qp->q_drain  ||  qp->q_running == 0  ||  qp->q_idle  ||
	   load(2) >= qp->q_loadsched)
		return;
	qp->q_idle = 1;
	while(qp->q_nexec < qp->q_maxexec) {
		bestfp = NULL;
		for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++)
			if(fp->f_name[0]  &&  fp->f_pid == 0) {
				if(bestfp == NULL ||
				   strcmp(bestfp->f_name+2, fp->f_name+2) > 0)
					bestfp = fp;
			}
		if(bestfp != NULL) {
			if(startjob(qp, bestfp) == 0) {
				qp->q_idle = 0;
				return;
			}
		} else
			break;
	}
	qp->q_idle = 1;
	return;
}

/*
 *  Start an actual job executing.
 */
STATIC
startjob(qp, fp)
register CF_File *fp;
register Queue *qp;
{
	int pid;
	int i;
	char outname[50];
	char procname[50];
	int fd;
	register Running *rp;
	struct passwd *pw, *getpwuid();

	sprintf(procname, "%s/%s", qp->q_name, fp->f_name);
	if((pw = getpwuid(fp->f_stat.st_uid)) == NULL)
		return 1;	/* ? */
	sighold(SIGCHLD);		/* until in parent table */
	if((pid = fork()) == -1) {
		sigrelse(SIGCHLD);
		pmerror("forking");
		return 0;
	}
	if(pid) {
#ifdef DEBUG
		muser(DEBUGFILE, "Job %s, pid %d\n", procname, pid);
#endif DEBUG
		nkids++;
		qp->q_nexec++;
		fp->f_pid = pid;
		for(rp = running ; rp < &running[MAXTOTAL] ; rp++)
			if(rp->pid == 0) {
				rp->pid = pid;
				rp->rqp = qp;
				rp->rfp = fp;
				break;
			}
		sigrelse(SIGCHLD);	/* job is safely in table now */
		if(rp >= &running[MAXTOTAL])
			merror("More than %d jobs; too many\n", MAXTOTAL );
		if(qp->q_usermail & MAIL_START)
		muser(pw->pw_name, "%s: Job is starting now.\n", procname);
		if(qp->q_supmail & MAIL_START)
		muser(qp->q_supervisor, "%s: Job starting for %s\n", procname, pw->pw_name);
		return 1;
	}
	sigrelse(SIGCHLD);	/* for symmetry and completeness */
	/*
	 *  Child. Setuid to the owner and setup
	 *  i/o redirection stuff to mail it back.
	 */
	setpgrp(0, qp->q_pgroup);
	fp->f_name[0] = 'o';	/* Output to `of' file name */
	sprintf(outname, "%s/%s", qp->q_name, fp->f_name);
	if((fd = creat(outname, 0600)) < 0)
		pxerror(outname);
	dup2(fd, 1);
	dup2(fd, 2);
	for(i=3 ; i < NOFILE ; i++)
		close(i);
	close(0);
	open("/dev/null", 0);
	for( i=0; i<NRLIM; i++ ){
		register struct rlimit *rlp = &(qp->q_rlimit[i]);
		if( rlp->rlim_cur >= 0 && rlp->rlim_max >= 0 )
			setrlimit( itorl(i), rlp );
	}
	nice(qp->q_nice);
	initgroups(pw->pw_name, fp->f_stat.st_gid);
	setgid(fp->f_stat.st_gid);
	setuid(fp->f_stat.st_uid);
	for(i=1 ; i <= NSIG ; i++)
		signal(i, SIG_DFL);
	execl(procname, procname, (char *)0);
	pxerror(procname);
	/*NOTREACHED*/
}

/* Turn RLIMIT manifest number into Integer.
 */
STATIC int
rltoi( rl )
int rl;
{
	switch( rl ){
	case RLIMIT_CPU: return 0;
	case RLIMIT_FSIZE: return 1;
	case RLIMIT_DATA: return 2;
	case RLIMIT_STACK: return 3;
	case RLIMIT_CORE: return 4;
	case RLIMIT_RSS: return 5;
	}
	error("rltoi(%d) invalid\n", rl );
	/*NOTREACHED*/
}

/* Turn K token from LEX into RLIMIT number.
 */
STATIC int
Ktorl( k )
enum keyword k;
{
	switch( k ){
	case K_CPULIMIT: return(RLIMIT_CPU);
	case K_RLIMITCPU: return(RLIMIT_CPU);
	case K_RLIMITFSIZE: return(RLIMIT_FSIZE);
	case K_RLIMITDATA: return(RLIMIT_DATA);
	case K_RLIMITSTACK: return(RLIMIT_STACK);
	case K_RLIMITCORE: return(RLIMIT_CORE);
	case K_RLIMITRSS: return(RLIMIT_RSS);
	}
	error("Ktorl(%d) invalid\n", k );
	/*NOTREACHED*/
}

/* Turn Integer into RLIMIT number.
 */
STATIC int
itorl( i )
int i;
{
	switch( i ){
	case 0: return(RLIMIT_CPU);
	case 1: return(RLIMIT_FSIZE);
	case 2: return(RLIMIT_DATA);
	case 3: return(RLIMIT_STACK);
	case 4: return(RLIMIT_CORE);
	case 5: return(RLIMIT_RSS);
	}
	error("itorl(%d) invalid\n", i );
	/*NOTREACHED*/
}

/*
 *  A process terminated.
 */
STATIC
terminated(pid, status)
{
	register Running *rp;
	register Queue *qp;
	register CF_File *fp;
	char cfname[DIRSIZ*2];

	for(rp = running ; rp < &running[MAXTOTAL] ; rp++)
		if(rp->pid == pid)
			break;
	if(rp >= &running[MAXTOTAL]) {
		/* This should never happen, provided SIGCHLD is
		 * properly held from before the fork() until after
		 * the process pid is put in the run table.   -IAN!
		 */
		merror("Signal %d from process %d but process not in internal table\n",
			status, pid);
		return;
	}
	nkids--;
	qp = rp->rqp;
	/*  Assume this is return of a mail process */
	if(qp == NULL) {
		rp->pid = 0;
		return;
	}
	fp = rp->rfp;
	mailback(status, qp, fp, MAIL_END, (char *)0);
	if(qp->q_supmail & MAIL_END)
	muser(qp->q_supervisor, "%s/%s: Job ended, status %o\n", qp->q_name, fp->f_name, status);
	sprintf(cfname, "%s/%s", qp->q_name, fp->f_name);
	if(unlink(cfname) == -1) {
		int e = errno;
		
		pmerror(cfname);
		muser(qp->q_supervisor,
		    "%s: Can't unlink jobs%s\n", qp->q_name,
			e==ENOENT? "": "; stopping queue");
		if (e != ENOENT)
			rundown(qp);
	}
	fp->f_name[0] = 0;
	fp->f_pid = 0;
	rp->pid = 0;
	qp->q_nexec--;
	if(qp->q_nexec < 0)
		error("nexec < 0!\n");
	/*
	 *  Finally all gone??
	 */
	if(qp->q_abort)
		if(qp->q_nexec == 0) {
			qp->q_name[0] = 0;
			qp->q_pgroup = 0;
		}
	/*
	 *  Drained?
	 */
	if(qp->q_drain  &&  qp->q_nexec == 0) {
		qp->q_drain = 0;
		muser(qp->q_supervisor, "%s: Drained finally\n", qp->q_name);
	}
	qp->q_idle = 0;
	runqueue(qp);
}

STATIC
mailback(status, qp, fp, mailstat, expl)
Queue *qp;
CF_File *fp;
MAIL mailstat;
char *expl;
{
	FILE *mail, *sendmail();
	char buf[BUFSIZ];
	char outname[DIRSIZ*2];
	int n;
	int output;
	int outstat;
	struct stat sb;
	struct passwd *pw, *getpwuid();
	static char *faults[] = {
		"hangup",
		"interrupt",
		"quit",
		"illegal instruction",
		"trace trap",
		"IOT instruction",
		"EMT instruction",
		"floating point exception",
		"killed utterly",
		"bus error",
		"segmentation violation",
		"bad arg to system call",
		"broken pipe",
		"alarm",
		"terminate",
		"signal 16",
		"stop",
		"keyboard stop",
		"continue",
		"child status",
		"background read",
		"background write",
		"input available",
		"cpu time limit exceeded",
		"file size limit exceeded",
		"signal 26",
		"signal 27",
		"signal 28",
		"signal 29",
		"signal 30",
		"signal 31",
		"signal 32",
	};

	fp->f_name[0] = 'o';
	sprintf(outname, "%s/%s", qp->q_name, fp->f_name);
	fp->f_name[0] = 'c';
	outstat = stat(outname, &sb);
	/*
	 *  Send mail even if we aren't supposed to if there is output
	 *  or status is non-null.
	 */
	if((qp->q_usermail & mailstat) == 0  &&
	   (outstat==-1 || sb.st_size==0)  &&
	   status == 0) {
		if(unlink(outname) < 0)
			merror(outname);
		return;
	}
	if((pw = getpwuid(fp->f_stat.st_uid)) == NULL) {
		merror("Can't mail %s to %d\n", outname, fp->f_stat.st_uid);
		return;
	}
	/*
	 * If submitter is root, mail to $USER.
	 * This ain't pretty...
	 */
	if (pw->pw_uid == 0) {
		FILE *f;
		char *rindex();

		rindex(outname, '/')[1] = 'c';
		if ((f = fopen(outname, "r")) != NULL) {
			char *p, *up = NULL;
			struct passwd *xpw, savepw;
			
			while (fgets(buf, sizeof buf, f) != NULL) {
				if (strncmp(buf, "USER=", 5) == 0) {
					up = buf+5;
					break;
				}
				if (strncmp(buf, "setenv USER ", 12) == 0) {
					up = buf+12;
					break;
				}
			}
			fclose(f);
			if (up == NULL)
				goto nope;
			/* crack 'user' */
			while (*up && *up != '\'')
				up++;
			if (*up == 0)
				goto nope;
			p = ++up;
			while (*p && *p != '\'')
				p++;
			if (*p == 0)
				goto nope;
			*p = 0;
			savepw = *pw;
			xpw = getpwnam(up);
			if (xpw != NULL)
				pw = xpw;
			else
				*pw = savepw;
		}
	   nope:
		rindex(outname, '/')[1] = 'o';
	}
	mail = sendmail(pw->pw_name);
	fprintf(mail, "Job %s in %s queue has ", fp->f_name, qp->q_name);
	if(mailstat & MAIL_CRASH)
		fprintf(mail, "been aborted due to crash; it was %s", expl);
	else {
	fprintf(mail, "completed");
	if(status == 0)
		fprintf(mail, " successfully");
	else {
		if(status & 0177) {
			if((status & 0177) > NSIG)
				fprintf(mail, " with unknown status %o", status);
			else
				fprintf(mail, " with termination %s",
					faults[(status & 0177)-1]);
			if(status & 0200)
				fprintf(mail, " and a core dump");
		} else
			fprintf(mail, " with exit code %d", status >> 8);
	}
	}
	fprintf(mail, ".\n\n");
	if(outstat == -1)
		fprintf(mail, "Can't stat output file %s\n", outname);
	else if(sb.st_size > 0) {
		fprintf(mail, "Output %s follows:\n\n", mailstat & MAIL_CRASH?
			"to crash point": "");
		if((output = open(outname, 0)) == -1) {
			pmerror(outname);
			fprintf(mail, "**Daemon can't read the output data!\n");
		} else {
			while((n = read(output, buf, sizeof(buf))) > 0)
				fwrite(buf, n, 1, mail);
			close(output);
		}
	}
	if(unlink(outname) < 0)
		pmerror(outname);
	mailclose(mail);
}

/*
 *  Stop all jobs in a particular queue.
 */
STATIC
stopall(qp)
register Queue *qp;
{
	if(qp->q_nexec == 0)
		return;
	muser(qp->q_supervisor, "%s: Stopping; load too high\n", qp->q_name);
	killpg(qp->q_pgroup, SIGSTOP);
}

/*
 *  Restart all jobs in a particular queue.
 */
STATIC
startall(qp)
register Queue *qp;
{
	if(qp->q_nexec == 0)
		return;
	muser(qp->q_supervisor, "%s: Restarting; load is ok now\n", qp->q_name);
	killpg(qp->q_pgroup, SIGCONT);
}
	
/*
 *  Abort all jobs in a particular queue.
 */
STATIC
abortall(qp)
register Queue *qp;
{
	register CF_File *fp;

	if(qp->q_nexec == 0)
		return;
	muser(qp->q_supervisor, "%s:  Terminating all jobs with extreme prejudice", qp->q_name);
	for(fp = qp->q_files ; fp < &qp->q_files[MAXENTRIES] ; fp++)
		if(fp->f_pid)
			abortjob(fp);
}

/*
 *  Abort a particular job.  Use extreme prejudice.
 *  Note that we count on wait getting the process back later.
 */
STATIC
abortjob(fp)
register CF_File *fp;
{
	kill(fp->f_pid, SIGKILL);
}

/*
 *  Queue is to be terminated. Stop new jobs running in that queue.
 */
STATIC
rundown(qp)
register Queue *qp;
{
	if(qp->q_running)
		muser(qp->q_supervisor, "%s: Draining\n", qp->q_name);
	qp->q_running = 0;
	if(qp->q_nexec == 0) {
		muser(qp->q_supervisor, "%s: Drained\n", qp->q_name);
		qp->q_drain = 0;
	}
}

struct	nlist nl[] = {
	{ "_avenrun" },
#define	X_AVENRUN	0
	{ 0 },
};

STATIC
load(i)
{
	static int kmem = -1;
	double	avenrun[3];

	if(kmem == -1) {
		nlist("/vmunix", nl);
		if (nl[0].n_type==0) {
			merror("%s: No namelist\n", "/vmunix");
			return 0;
		}
		if((kmem = open("/dev/kmem", 0)) == -1) {
			pmerror("/dev/kmem");
			return 0;
		}
	}
	lseek(kmem, (long)nl[X_AVENRUN].n_value, 0);
	read(kmem, avenrun, sizeof(avenrun));
	return (int) (avenrun[i] + .5);
}

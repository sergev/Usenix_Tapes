/*
 *		 SH Shell.
 *
 *
 *	Copyright (c) S. Brown, 1987
 *
 *
 *	wait stuff, for processes to be waited for.
 *
 *	also, job-control.
 */

#include "defs.h"

extern char *sysmsg[];		/* signal messages */

#ifdef JOB

/*
 *	JOB-CONTROL
 */


#ifdef JOBSXT
#undef input
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/sxt.h>
#include <errno.h>
#endif JOBSXT

#define PROCESS_DOT_C
#include "job.h"
#include <signal.h>

int currentjob = 0;		/* current job */
int fgjobs = 0;			/* no. of foreground jobs */
int bgjobs = 0;			/* no. of background jobs */
int mainpgrp;			/* initial process group */
int dfltjob;			/* default jobnumber for fg, bg, notify ... */
int bdfltjob;			/* back-one default */
int maxpri = 0;			/* max. priority */
int extracnt=0;			/* number of children dead in queue */

#ifdef JOBSXT
int sxtchan = -1;		/* sxt channel set */
int savsxtchan = -1;		/* saved chan fd */
struct sxtblock sxtblock;	/* block masks */
int sxtcontrol = -1;		/* controlling sxt-tty */
int sxtkludge = 1;		/* real tty virtual offset of controller */
char blkmask;			/* virtual blocks */
TTYNOD realtty;			/* state of physical tty */

#define BLKBIT(c,n) ((c)&(1<<SXT(n)))
#define SIGBLOCK 21		/* fake signal-number for blockages */
int sxt_fd = -1;
#endif JOBSXT



struct jobnod *jobs[JOB_MAX];
struct extrawait extras[EXTRAPROCS];

/*
 *	empty all of jobs table
 */
postclr()
{
	register int i;

	for (i=1; i<JOB_MAX; i++)
		postjclr(i);
	bgjobs = fgjobs = 0;
}


/*
 *	clear out an entry in jobs table
 */
postjclr(jobno)
{
	register int i;
	if (jobs[i=jobno]){
		free((char *)jobs[i]->jb_dir);
		free((char *)jobs[i]->jb_cmd);
#ifdef JOBSXT
		if (jobs[i]->jb_sxtfg) close(jobs[i]->jb_sxtfg);
		blkmask &= ~(1<<SXT(i));
#endif 
		free((char *)jobs[i]);
		jobs[i] = (struct jobnod *)0;
	}
	if (i==dfltjob || i==bdfltjob) setdfj();
}

/*
 * set default job
 */
setdfj()
{
	register int i;

	maxpri = 0;
	bdfltjob = dfltjob;
	dfltjob = -1;
	for (i=1; i<JOB_MAX; i++)
		if (jobs[i] && jobs[i]->jb_pri > maxpri){
			maxpri = jobs[i]->jb_pri;
			bdfltjob = dfltjob;
			dfltjob = i;
		}
}


/*
 *	place process in jobs table 
 */
post(procid)
int	procid;
{
	register struct jobnod *jp;
	register int i;

	jp = jobs[currentjob];
	if (jp){
		for (i=0; i<PPERJ; i++)
			if (jp->jb_proc[i].proc_pid==0) break;
		if (i>PPERJ-1 || (jp->jb_flags.singlebits.lifes&(1<<i)))
			/*error("cannot post job")*/;	/* Never Happens */
		jp->jb_proc[i].proc_pid = procid;
		jp->jb_flags.singlebits.lifes |= (1<<i);
	}
}

/*
 *	restart in background
 */
bg(jno)
{
	if (jobs[jno]){
#ifdef JOBSXT
		ioctl(sxt_fd,SXTIOCUBLK,SXT(jno));
		sxtblock.input &= ~(1<<SXT(jno));
		sxtblock.output &= ~(1<<SXT(jno));
		blkmask &= ~(1<<SXT(jno));
#endif JOBSXT
		jobs[jno]->jb_flags.singlebits.back = jobs[jno]->jb_flags.singlebits.run = 1;
		jobs[jno]->jb_pri = ++maxpri;		/* top priority */
		bdfltjob = dfltjob;
		if (jobs[jno]->jb_flags.singlebits.stop){
			jobs[jno]->jb_flags.singlebits.stop = 0;
			if (jobs[jno]->jb_cmd){
				prs(jobs[jno]->jb_cmd);
				prs(" &\n");
			}
		}
#ifdef JOBSXT
		if (sxtchan!=-1)
#endif JOBSXT
			iflags |= jobflg;
		setjob(0);	/* so it'll stop if its reading from tty */
		bgjobs++;
	}
}

/*
 *	set the "notify" bit in a job 
 */
notify(jno)
{
	if (jobs[jno])
		jobs[jno]->jb_flags.singlebits.notify = 1;
}



/*
 *	bring a job into foreground.
 */
fg(jno)
{
	if (jobs[jno] && jno>0){
		jobs[jno]->jb_flags.singlebits.back = 0;
		jobs[jno]->jb_flags.singlebits.run = 1;
		jobs[jno]->jb_pri = ++maxpri;		/* top priority */
		bdfltjob = dfltjob;
		dfltjob = jno;
#ifdef JOBSXT
		if (sxtchan!=-1)
#endif JOBSXT
			iflags |= jobflg;			/* in case it was forgotten... */
		if (jobs[jno]->jb_cmd){
			prs(jobs[jno]->jb_cmd);
			newline();
		}
		if (jobs[jno]->jb_flags.singlebits.stop)
			jobs[jno]->jb_flags.singlebits.stop = 0;
#ifdef JOBSXT
		ioctl(sxt_fd, SXTIOCBLK, SXT(jno));
		sxtblock.input &= ~(1<<SXT(jno));
		sxtblock.output &= ~(1<<SXT(jno));
		blkmask &= ~(1<<SXT(jno));
#ifdef ERCC_SIGCONT
		killpg(jobs[jno]->jb_pgrp,SIGCONT);
#endif
		setjob(jno);
#endif SysV
		await(-1,WAIT_FG);
	}
}

/*
 *	wait for processes to terminate.
 *	waitflags is an ORing of
 *		WAIT_FG	- waiting for foreground jobs
 *		WAIT_BG - waiting for background jobs
 *		WAIT_NOPAUSE - fast wait, don't hang around
 *		WAIT_JOB - waiting for a job-no, not a process-no.
 */
await(id,waitflags)
{
	register int i;
	register int gotit=0;
#ifdef JOBSXT
	register int j;
	struct sxtblock block;
	register int oldex;
#endif

	if ((waitflags&WAIT_JOB)==0 && id>0) post(id);
	if (waitflags&WAIT_FG) fgjobs++;
	flushb();
	while (fgjobs>0 || ((waitflags&(WAIT_NOPAUSE|WAIT_BG)) && bgjobs>0)){
		if ((trapnote&SIGSET) && fgjobs==0){
			listjobs((char *)0);
			prs("wait: interrupted\n");
			break;
		}
#ifdef JOBSXT
		if (extracnt>0){
			gotit=announce();
		} else if (sxtchan!=-1 && currentjob<SXTMAX
			    		&& (waitflags&WAIT_BG)==0){
			if (setsxt(currentjob) != SXT(currentjob))
				continue;
			iflags |= sxtwaiting;
			oldex = extracnt;
			while (ioctl(sxt_fd,SXTIOCWF,0) == -1)
			iflags &= ~sxtwaiting;
			setsxt(currentjob);
			if (jobs[currentjob]->jb_sxtfg!=(-1))
				ioctl(jobs[currentjob]->jb_sxtfg,TCSBRK,1);
			ioctl(sxt_fd,SXTIOCSTAT,&block);
			for (i=1; i<SXTMAX; i++){
				BOOL blkin = (BLKBIT(block.input,i)!=0);
				BOOL blkout = (BLKBIT(block.output,i)!=0);
				if (i!=currentjob){
					if (!blkin && !blkout) continue;
					if (BLKBIT(sxtblock.input,i) || BLKBIT(sxtblock.output,i) || BLKBIT(blkmask,i))
						continue;
				} else {
					if (extracnt!=oldex)
						continue;
				}
				blkmask |= (1<<SXT(i));
				jobs[i]->jb_flags.singlebits.stop = 1;
				jobs[i]->jb_flags.singlebits.run = 0;
				jobs[i]->jb_flags.singlebits.deaths = jobs[i]->jb_flags.singlebits.lifes;
				if (jobs[i]->jb_flags.singlebits.back)
					bgjobs--;
				else {
					fgjobs--;
					setsxt(0);
				}
				for (j=0; j<PPERJ; j++)
					if (jobs[i]->jb_proc[j].proc_pid)
						jobs[i]->jb_proc[j].proc_status = (SIGBLOCK<<8)|0177;
				donotify(i);
			}
			sxtblock.input = block.input;
			sxtblock.output = block.output;
		}
		else 
#endif JOBSXT
		{    /* System-V or V7 without sxt's */
#if defined(SIGCLD) || defined(SIGCHLD)
			if (extracnt==0 && (waitflags&WAIT_NOPAUSE)==0)
				pause();
#else
			extrastack(0,0);
#endif SIGCLD or SIGCHLD
			if (extracnt>0) gotit=announce();	
		}
		if ( ((waitflags&WAIT_JOB) && gotit==id)
			    	|| ((waitflags&WAIT_NOPAUSE) && gotit!=0 && extracnt<=0))
			break;
	}
	if (fgjobs<0) fgjobs=0;
	if (extracnt<0) extracnt=0;		/* ... */
	for (i=1; i<JOB_MAX; i++)
		if (jobs[i] && jobs[i]->jb_flags.singlebits.deaths==jobs[i]->jb_flags.singlebits.lifes){
			jobs[i]->jb_flags.singlebits.deaths = 0;
			printjob(i);
			if (jobs[i]->jb_flags.singlebits.stop==0)
				postjclr(i);
		}
        setjob(0);         /* restore to control state */
}

/*
 *	trap routine for SIGCHLD/SIGCLD
 */
#if defined(SIGCHLD) || defined(SIGCLD)
job_fault()
{
	extrastack(0,0);
#ifdef JOBSXT
	if ((flags&waiting)==0 && sxtchan!=-1 && (iflags&sxtwaiting))
		setsxt(-1);
	else if (flags&waiting) announce();
#endif JOBSXT
}
#endif SIGCHLD or SIGCLD



/*
 *	set current job to "job"
 */
setjob(jno)
{
	register struct jobnod *jp = jobs[jno<0? 0 : jno];

	if ((iflags&jobflg) && jp){
#ifdef JOBSXT
		setsxt(jno);
#endif JOBSXT
	}
	currentjob = jno;
}


/*
 *	set up job stuff in parent, assumimg I/O j-control has already
 *	been set up in the child.
 */
setparjob(jno)
{
	register struct jobnod *jp = jobs[jno];

	if (jp && (iflags&jobflg)){
#ifdef JOBSXT
		sxtcontrol = jno;
#endif JOBSXT
	}
}


/*
 *	set up j-control I/O in child
 */
setchildjob(jno,sxtflag)
{
	register struct jobnod *jp = jobs[jno];

	if (jp){
		if (jp->jb_pgrp==0)
			jp->jb_pgrp = getpid();
	}
	if (sxtflag && jno){
#ifdef JOBSXT
		if (sxtflag==1){
			ioctl(sxt_fd,SXTIOCBLK,SXT(jno));
			setsxt(jno);
		} else  ioctl(sxt_fd,SXTIOCUBLK,SXT(jno));
#endif JOBSXT
	}
}

/*
 *	book a slot in the jobs-table
 */
bookjob()
{
	register int i;

	for (i=0; i<JOB_MAX; i++)
		if (jobs[i]==0){
			jobs[currentjob=i] = (struct jobnod *)alloc(sizeof(struct jobnod));
			jobs[i]->jb_dir = jobs[i]->jb_cmd = (char *)0;
			jobs[i]->jb_pgrp = 0;
#ifdef JOBSXT
			jobs[i]->jb_sxtfg = -1;
			if ((flags&(ttyflg|forked))==(ttyflg)){
				if (sxtchan!=-1)
					iflags |= jobflg;
			} else iflags &= ~jobflg;
#endif JOBSXT
			return;
		}
	/*printf("Jobs Table is Full!\n");*/	/* nobody should have THAT many jobs running! */
}




/*
 *	create storage for a new job (already chosen by "bookjob")
 */
newjob(pid,jcmd,backing,chanid)
struct argnod * jcmd;
{
	register struct jobnod *jp;
	register int j;
        register char * cmdbuffer;
        register int cmdlength;
        struct argnod * storcmd;

    /*		-- This should NEVER happen, so its commented out --
	if (currentjob<0 || currentjob>=JOB_MAX || jobs[currentjob]==0){
		prs("Job Control Error\n");
		return(0);
	}
    */
	jp = jobs[currentjob];
#ifdef notdef
	if ( (flags&(forked|ttyflg))!=ttyflg)
		pid = 0;
#endif notdef
#ifdef JOBSXT
	if (sxtchan!=-1)
		jp->jb_sxtfg = chanid;		/* parent sxt channel access */
#endif JOBSXT
	jp->jb_pgrp = pid ? pid : jobs[0]->jb_pgrp;
	jp->jb_pri = 0;
	if (jcmd){
		for (cmdlength=0, storcmd=jcmd; storcmd; storcmd=storcmd->argnxt)
			cmdlength += length(storcmd->argval);
		cmdbuffer = jp->jb_cmd = (char *)alloc(cmdlength+1);
		*cmdbuffer = '\0';
		for (storcmd=jcmd; storcmd; storcmd=storcmd->argnxt){
			cmdbuffer = movstr(storcmd->argval,cmdbuffer);
			cmdbuffer = movstr(" ",cmdbuffer);
		}
	} else jp->jb_cmd = make("(?)");
	jp->jb_flags.totalbits = 0;
	jp->jb_flags.singlebits.run = 1;
	for (j=0; j<PPERJ; j++){
		jp->jb_proc[j].proc_pid=0;
		jp->jb_proc[j].proc_status=0;
	}
	if (!backing){
		bdfltjob = dfltjob;
		dfltjob = currentjob;
		jp->jb_pri = ++maxpri;
	}
	if (pid && pid!=mainpgrp)
		setparjob(currentjob);
	return(currentjob);
}
			


/*
 *	the "jobs" command
 */
listjobs(llist)
char *llist;
{
	register struct jobnod **jp;
	register int j;

#ifdef JOBSXT
	ioctl(sxt_fd,SXTIOCSTAT,&sxtblock);
#endif

	for (jp = &jobs[1]; jp< &jobs[JOB_MAX]; jp++)
		if (*jp){
			prc('[');
			prn(jp-jobs);
			prs("] ");
			if (llist){
				for (j=0; j<PPERJ; j++)
					if ((*jp)->jb_proc[j].proc_pid){
						prn((*jp)->jb_proc[j].proc_pid);
						prs((((*jp)->jb_flags.singlebits.lifes&(1<<j)))?" ":"Z ");
					}
				prc(' ');
			} else prs("    ");
			if (jp==&jobs[dfltjob]) prc('+');
			else if (jp==&jobs[bdfltjob]) prc('-');
			else prc(' ');
			prc((*jp)->jb_flags.singlebits.notify ? '!' : ' ');
			prc(' ');
#ifdef JOBSXT
			if (BLKBIT(sxtblock.input,jp-jobs) ||
				   	BLKBIT(sxtblock.output,jp-jobs))
				if ((*jp)->jb_flags.singlebits.stop==0){
					if ((*jp)->jb_flags.singlebits.back)
						bgjobs--;
					else fgjobs--;
					(*jp)->jb_flags.singlebits.stop=1;
				}
#endif JOBSXT
			if ((*jp)->jb_flags.singlebits.stop)
				prstop(jp-jobs, llist!=(char *)0);
			else if ((*jp)->jb_flags.singlebits.run)
				prs("Running\t\t");
			else prs("(?)\t\t");
			prc('\t');
			prs((*jp)->jb_cmd ? (*jp)->jb_cmd : "(?)");
			newline();
		}
}


/*
 *	print stop-state
 */
prstop(jno,longlist)
{
#ifdef JOBSXT
	register BOOL blkin, blkout;

	prs("Blocked");
	blkin = BLKBIT(sxtblock.input,jno);
	blkout = BLKBIT(sxtblock.output,jno);
	if (blkin && blkout) prs(" (tty i/o)");
	else if (blkin) prs(" (tty input)");
	else if (blkout) prs(" (tty output)");
	else  prs(longlist?"\t\t":"\t");
#endif JOBSXT
}


/*
 *	print info about jobs, and clean out if dead
 */
donotify(jobno)
{
	register int i;

	if ((i=jobno) > 0){
		if (jobs[i] && (jobs[i]->jb_flags.singlebits.deaths)){
			jobs[i]->jb_flags.singlebits.deaths=0;
			if (jobs[i]->jb_flags.singlebits.stop){
				newline();
				printjob(i);
			} else {
				printjob(i);
				postjclr(i);
			}
		}
	}
}






/*
 *	If any processes have terminated, do the works.
 */
announce()
{
	BOOL stoppage=FALSE;
	register int i, j;
	int w, status;

	if (extracnt>0){
		extracnt--;
		w = extras[extracnt].pid;
		status = extras[extracnt].status;
	} else {
		/* This can NEVER happen! (ha ha) */
#ifdef JOBSXT
		/*printf("no wait/extra\n");*/
#endif JOBSXT
		w = 0;
	}
	if (w==0 || w==-1){
		/* Can this happen? Does it matter if it does? No. */
		/*printf("Process Failure:no wait\n");*/
		return 0;
	}
	for (i=1; i<JOB_MAX; i++){
		if (jobs[i]==0) continue;
		for (j=0; j<PPERJ; j++)
			if (jobs[i]->jb_proc[j].proc_pid == w){
				jobs[i]->jb_flags.singlebits.deaths |= (1<<j);
				jobs[i]->jb_flags.singlebits.stop = stoppage ? 1 : 0;
				jobs[i]->jb_proc[j].proc_status = status;
				if (jobs[i]->jb_flags.singlebits.deaths == jobs[i]->jb_flags.singlebits.lifes){
					if (jobs[i]->jb_flags.singlebits.run)
						if (jobs[i]->jb_flags.singlebits.back)
							bgjobs--;
						else	fgjobs--;
					jobs[i]->jb_flags.singlebits.run = 0;
					donotify(i);
				} else if (stoppage==0){
					jobs[i]->jb_flags.singlebits.lifes &= ~(01<<j);
					jobs[i]->jb_flags.singlebits.deaths &= ~(01<<j);
				}
				return(i);
		}
	}
	/* it wasn't a known process - 		 */
	/*	so forget about it.		 */
	/* printf("unknown process has died\n"); */
	/* extras[extracnt].pid = w;		 */
	/* extras[extracnt].status = status;	 */
	/* extracnt++;				 */
	return(0);
}


/*
 *	stacking function for putting terminated jobs onto extras when
 *	they die - 'cos they must be waited for immediately, but its
 *	not safe to do an announce().
 */
extrastack(p,s)
{
	int w, status;

	if (p==0){
		w = wait(&status);
	} else {
		w=p;
		status=s;
	}
	if (w>0){
		extras[extracnt].pid = w;
		extras[extracnt].status = status;
		extracnt++;
	} /* else printf("stack: wait error\n"); */
}



/*
 *	initialize jobs table
 *	(and set up sxt's under Sys-V)
 */
initjobs()
{
	if (jobs[0]) postjclr(0);
	bookjob();
	newjob(mainpgrp,(struct argnod *)0,0,0);	/* this had better be job[0] */
	jobs[0]->jb_pgrp = mainpgrp;
#ifdef JOBSXT
	terminal(&realtty);
	realtty.c_cc[VSWTCH] = 0377;
	realtty.c_cflag &= ~LOBLK;
	sxtinit();
#endif
}



/*
 *	Something has happened to a job.
 *	Print out info and clean up if necessary.
 */
printjob(jno)
{
	register struct jobnod *jp = jobs[jno];
	register int j;
	BOOL fgdone = FALSE;
	int wx=0, rc=0;
	int sig=0;
	int status=0;
	char * msg=0;

	if (jp){
		for (j=0; j<PPERJ; j++){
			int ww, ss, ww_hi;

			if (jp->jb_proc[j].proc_pid==0) continue;
			ww = jp->jb_proc[j].proc_status;
			ww_hi = (ww>>8)&LOBYTE;
			ss = (ww&0177);
			if (ss==0177){
				ss = ww_hi;
			}
			wx |= ww;
			if (rc==0){
				if (ss) sig = ss;
				if (jp->jb_flags.singlebits.back)
					continue;	/* just wanted the signal */
				rc = ss ? ss|SIGFLG : ww_hi;
				status = ww;
				fgdone = TRUE;
			}
		}
		if (jp->jb_flags.singlebits.stop==0 && jp->jb_flags.singlebits.back==0 && (sig==SIGINT || sig==SIGQUIT))
			newline();
		if (jp->jb_flags.singlebits.stop==0 
			    && jp->jb_flags.singlebits.back==0){
			TTYNOD tt;
#ifdef JOBSXT
			if (jp->jb_sxtfg!=(-1) && (flags&ttyflg) && sig!=SIGHUP){
				if (ioctl(jp->jb_sxtfg,TCGETA,&tt)==0){
					tt.c_cc[VSWTCH] = 0377; /* ^Z disabled */
					tt.c_cflag &= ~LOBLK;	/* no blocking */
					ioctl(0,TCSETA,&tt);
				}
				if (sig==SIGINT || sig==SIGQUIT){	/* Yeauch! */
					fault(sig);	/* simulate shell's tty pgrp */
					if (j==0 && jp->jb_proc[1].proc_pid)
						kill(jp->jb_proc[1].proc_pid,sig);
				}
			}
#endif JOBSXT
		}

		if ( (msg=(sig==SIGBLOCK?"Blocked":sysmsg[sig]))
			     || jp->jb_flags.singlebits.notify
			     || (ntfynod.namval) ){
			if (msg==0 && !fgdone) msg=nullstr;
			if (msg && (flags&stdflg)){
#if defined(JOBSXT) && defined(BLK_BOTTOM)
				if (jp->jb_flags.singlebits.stop)
					blk_bottom();
#endif (JOBSXT && BLK_BOTTOM)
				if (jp->jb_flags.singlebits.back || jp->jb_flags.singlebits.stop){
					prc('[');
					prn(jno);
					prs("]\t");
				}
				if (jp->jb_cmd){
					register char *p = jp->jb_cmd;
					while (*p && *p!=' ')
						prc(*p++);
					prc('\t');
				}
				prs(*msg ? msg : "Done");
				if (status&0200) prs(coredump);
				newline();
				flushb();
			}
		}
		if (fgdone)
			if (wx && (flags&errflg)) exitsh(rc);
			else {
				exitval=rc;
				exitset();
			}
	} /*else printf("trying to report unknown job\n");*/
}






/*
 *	find a job-number from a describing "%..." string
 */
findjob(str)
char *str;
{
	int number = -1;
	register struct jobnod **jpp;
	register char *p;
	int found = 0;
	int nlen;

	if (str==0 || *str=='\0'){
		number=dfltjob;
		if (number<1 || jobs[number]==0) error("No appropriate jobs");
	} else {
		if (*str=='%') str++;
		if (*str=='%' || *str=='\0' || *str=='+')
			number=dfltjob;
		else if (*str=='-')
			number=bdfltjob;
		else if (digit(*str))
			number=stoi(str);
		else if (*str=='?'){
			nlen = length(++str)-1;
			for (jpp=jobs; jpp < &jobs[JOB_MAX]; jpp++)
				if (*jpp)
					for (p=(*jpp)->jb_cmd; *p; p++)
						if (cfn(str,p,nlen)==0){
							number = jpp-jobs;
							if (found++) failed(str,"ambiguous");
							break;
						}
			if (!found) failed(str,"no such job");
		} else {
			nlen = length(str)-1;
			for (jpp=jobs; jpp < &jobs[JOB_MAX]; jpp++)
				if (*jpp && cfn(str,(*jpp)->jb_cmd,nlen)==0){
					number = jpp-jobs;
					if (found++) failed(str,"ambiguous");
				}
			if (!found) failed(str,"no such job");
		}

	}
	if (number<1 || number>=JOB_MAX || jobs[number]==0)
		failed(str,"no such job");
	return(number);
}




/*
 * check for stopped jobs
 */
stpjobs()
{
      register int i;

      for(i=0; i<JOB_MAX; i++)
              if (jobs[i] && jobs[i]->jb_flags.singlebits.stop)
                      return(TRUE);
      return(FALSE);

}



/*
 * zap 'em
 */
zapjobs()
{
	register int i;

	for (i=1; i<JOB_MAX; i++)
		if (jobs[i] && jobs[i]->jb_flags.singlebits.stop){
			killpg(jobs[i]->jb_pgrp, SIGHUP);
			jobs[i]->jb_flags.singlebits.stop = 0;
			jobs[i]->jb_flags.singlebits.back = 1;             /* so that it gets reported properly */
			jobs[i]->jb_proc[0].proc_status = SIGHUP;
			printjob(i);
		}
}



#ifdef JOBSXT

#ifdef JOBSXT_HUP
char Mstart[] = "echo \"\nsh was killed off, so I hung up the following processes:\n";
char Mend[]   = "\n\" | mail -s sh_was_killed $LOGNAME";	/* good meta-usage */

/*
 *	HUP has happened! Send mail and go away.
 */
hupmail()
{
	register int i;
	char maillist[1024];
	register char *mpoint;
	int mcnt = 0;

	sxtchan = -1;
	flags &= ~ttyflg;	/* just in case ... */
	mpoint = movstr(Mstart, maillist);
	for (i=1; i<JOB_MAX; i++)
		if (jobs[i] && jobs[i]->jb_flags.singlebits.back==0){
			killpg(jobs[i]->jb_pgrp,SIGHUP);
			if (jobs[i]->jb_cmd){
				mpoint = movstr("\t", mpoint);
				mpoint = movstr(jobs[i]->jb_cmd,mpoint);
				mpoint = movstr("\n", mpoint);
			} else mpoint = movstr("\t?\n", mpoint);
			mcnt++;
		}
	movstr(Mend, mpoint);
	if (mcnt)
		execexp(maillist,(char * *)0);	/* bye! */
}
#endif JOBSXT_HUP

/*
 *	set current job using sxt's
 */
setsxt(jno)
{
	if (sxtchan != -1 && jno<SXTMAX){
		jno = SXT(jno);
		sxtcontrol = -1;
		do {
			if (ioctl(sxt_fd,SXTIOCSWTCH,jno) == 0)
				sxtcontrol = jno;
			/*else if (errno!=EINTR)
				printf("sxt switch (errno %d)\n", errno);*/
		} while (sxtcontrol!=jno && errno==EINTR);
		return(sxtcontrol);
	} else return(-1);
}

/*
 *	block an sxt file
 */
sxtblk(jno)
{
	ioctl(sxt_fd, SXTIOCBLK, SXT(jno));
}

#endif JOBSXT


/*
 *	send a signal to all processes in a tty group.
 *	actually, it only sends to the first and last in the pipe-order,
 *	'cos it only knows the pid's for these (jb_proc[0] ... jb_proc[PPERJ]).
 *	This exists as a system-call under BSD.
 *	Luckily it knows about negative pids.
 */
killpg(gid,sig)
{
	register struct jobnod **jp;
	register int i=0;
	int err = 0, proc;;

	for (jp=jobs; jp < &jobs[JOB_MAX]; jp++)
		if (*jp && (*jp)->jb_pgrp==gid)
			for (i=0; i<PPERJ; i++){
				proc = (*jp)->jb_proc[i].proc_pid;
				if (proc && kill(-proc,sig)==-1)
					err = errno;
			}
	/* Panic Stations! It didn't work, so get really desparate... */
	if (err!=0)
		if (kill(-gid,sig)==-1 && kill(gid,sig)==-1)
			err = errno;
		else err=0;
	return(err);
}

#if defined(JOBSXT) && defined(BLK_BOTTOM)
/*
 *	move cursor to bottom of screen - when job is blocked
 *
 *	if you have TERMCAP defined, it does it properly be sending
 *	the "ll" string and clearing to end-of-line, otherwise it is
 *	pretty dumb.
 *	in fact, TERMCAP implies a lot of other stuff I havn't included
 *	here, so tough.
 *				simon@uk.ac.ed.its63b
 */
blk_bottom()
{
# ifdef TERMCAP
	STRING it, p;

	if (p=tcapstr[TCAP_LL])
		tputs(p,1,prc);
	else if (it=tcapstr[TCAP_CM]){
		p=tgoto(it,0,tcapflag[TCAP_ROW]);
		tputs(p,1,prc);			/* move to bottom */
	} else
# endif TERMCAP
	{
		register int n = 24;	/* bound to have 24 lines */
		while (n--)
			newline();	/* chug chug chug ... */
	}
# ifdef TERMCAP
	if (p=tcapstr[TCAP_CE])
		tputs(p,1,prc);		/* clear to eoln */
# endif TERMCAP
}
#endif (JOBSXT && BLK_BOTTOM)

#else !JOB
 

/*
 * for processes to be waited for
 */
#define MAXP 20
static int	pwlist[MAXP];
static int	pwc;

postclr()
{
	register int	*pw = pwlist;

	while (pw <= &pwlist[pwc])
		*pw++ = 0;
	pwc = 0;
}

post(pcsid)
int	pcsid;
{
	register int	*pw = pwlist;

	if (pcsid)
	{
		while (*pw)
			pw++;
		if (pwc >= MAXP - 1)
			pw--;
		else
			pwc++;
		*pw = pcsid;
	}
}

await(i, bckg)
int	i, bckg;
{
	int	rc = 0, wx = 0;
	int	w;
	int	ipwc = pwc;

	post(i);
	while (pwc)
	{
		register int	p;
		register int	sig;
		int		w_hi;
		int	found = 0;

		{
			register int	*pw = pwlist;

			p = wait(&w);
			if (wasintr)
			{
				wasintr = 0;
				if (bckg)
				{
					break;
				}
			}
			while (pw <= &pwlist[ipwc])
			{
				if (*pw == p)
				{
					*pw = 0;
					pwc--;
					found++;
				}
				else
					pw++;
			}
		}
		if (p == -1)
		{
			if (bckg)
			{
				register int *pw = pwlist;

				while (pw <= &pwlist[ipwc] && i != *pw)
					pw++;
				if (i == *pw)
				{
					*pw = 0;
					pwc--;
				}
			}
			continue;
		}
		w_hi = (w >> 8) & LOBYTE;
		if (sig = w & 0177)
		{
			if (sig == 0177)	/* ptrace! return */
			{
				prs("ptrace: ");
				sig = w_hi;
			}
			if (sysmsg[sig])
			{
				if (i != p || (flags & prompt) == 0)
				{
					prp();
					prn(p);
					blank();
				}
				prs(sysmsg[sig]);
				if (w & 0200)
					prs(coredump);
			}
			newline();
		}
		if (rc == 0 && found != 0)
			rc = (sig ? sig | SIGFLG : w_hi);
		wx |= w;
		if (p == i)
		{
			break;
		}
	}
	if (wx && flags & errflg)
		exitsh(rc);
	flags |= eflag;
	exitval = rc;
	exitset();
}

#endif JOB

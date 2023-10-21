#include <sys/param.h>
#include "ed.h"
#include "window.h"
#include "edit.h"
#include "process.h"

/*
 * send a line to background process
 */
send(pp,line)
	register struct process *pp;
	register char *line;
	{
#ifdef debugging
debug(73,"sending %s file=%o",line,pp->pr_tobkg);
#endif
	while(*line)
		fputc(*line++,pp->pr_tobkg);
	if(line[-1]!='\n')
		fputc('\n',pp->pr_tobkg);
	fflush(pp->pr_tobkg);
	}

/*
 * open pipes and fork background process
 */
makeproc(pp,prog)
	register struct process *pp;
	char *prog;
	{
	int pfd[2];
	
	if(pp->pr_pid) return;
	if(pipe(pfd))
		errfunc("Can't open pipes.");
	pp->pr_siifd=pfd[0];
	pp->pr_siofd=pfd[1];
	if(pipe(pfd))
		errfunc("Can't open pipes.");
	pp->pr_soifd=pfd[0];
	pp->pr_soofd=pfd[1];
	editpid=getpid();
	if((pp->pr_pid=fork())==0){	/* this is the background process */
		int i;

		close(pp->pr_siofd);
		close(pp->pr_soifd);
		close(0);
		dup(pp->pr_siifd);
		close(1);
		dup(pp->pr_soofd);
/* delete for debug */
		close(2);
		dup(pp->pr_soofd);
/* */
		for(i=3; i<NOFILE; i++)
			close(i);
/** delete this while testing */
#ifndef vax
		nice(-40);
		nice(20);
		nice(0);
#endif
/* */
		signal(SIGHUP,oldhup);
		signal(SIGQUIT,oldquit);
		signal(SIGALRM,oldalrm);
		execl(prog,prog[strlen(prog)-1]=='h'?"esh":"esp","-E",itos(editpid),0);
		/* no return */
		printf("Can't execute %s\n",prog);
		exit(1);
		}
	/* this is the main process */
	close(pp->pr_siifd);
/*	close(pp->pr_soofd);	want this open for salting */
	pp->pr_tobkg=fdopen(pp->pr_siofd,"w");
	pp->pr_fmbkg=fdopen(pp->pr_soifd,"r");
	}

killproc(pp)
	register struct process *pp;
	{
	int status;
	if(pp->pr_pid==0)
		return;
	close(pp->pr_soofd);
	fclose(pp->pr_tobkg);
	fclose(pp->pr_fmbkg);
	kill(pp->pr_pid,SIGKILL);
	wait(&status);
	pp->pr_pid=0;
	}

char *
itos(n)
	{
	register char *abuf; 
	register unsigned a, i; 
	int pr, d;
	static char	numbuf[6];

	abuf=numbuf; 
	pr=0; 
	a=n;
	for( i=10000; i!=1; i/=10){	
		if( (pr |= (d=a/i)) )
			*abuf++=d+'0';
		a %= i;
		}
	*abuf++=a+'0';
	*abuf++=0;
/* debug
fprintf(stderr,"itos: numbuf=%s\n",numbuf);
 */
	return(numbuf);
	}

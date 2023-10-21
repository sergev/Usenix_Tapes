#include <stdio.h>
#include <sys/param.h>
#include "ed.h"
#include "window.h"
#include "edit.h"
#include "shell.h"

#define shhiwat 500	/* maximum lines in tempfile */
#define shlowat 350	/* truncation point */

int	pipecount;	/* number of shell chars read from pipe */
char*	ps1;		/* shell prompt */

char*	shprog="/usr/lib/edit/sh";
/*
 * send a command line to the shell
 */
sendtoshell(line)
	char *line;
	{
	register struct window *wp;

#ifdef debugging
debug(74,"sendtoshell: w[0]=%o",w[0]);
#endif
	if(shellpid==0){
		int sleft;	/* time left on alarm clock */

		if(w[0]==0){
			init(0);
#ifdef debugging
debug(74,"after init w[0]=%o",w[0]);
#endif
			}
		flush();
		makeshell();
		sleft=alarm(0);
		pause();	/* wait for first prompt */
		alarm(sleft?sleft:30);
#ifdef SIG_HOLD
		signal(SIGSHELL,SIG_HOLD);
#else
		signal(SIGSHELL,SIG_IGN);
#endif
		}
	wp=w[0];
#ifdef debugging
debug(74,"wp=%o w[0]=%o",wp,w[0]);
#endif

	/* put the line on screen */
	/** this is rather kludgy */
	/** might be able to use updateline if $ is onscreen */
	saveloc();
	getline(wp->wi_fileno, *wp->wi_dol);	/* look at last line */
#ifdef debugging
debug(49,"dol=%s ps1=%s",linebuf,ps1);
#endif
	if(strcmp(linebuf, ps1)==0){	/* it is a prompt line */
		int n;
		strcat(linebuf,line);
		if((n=getslno(wp->wi_dol))>=0)
			clearsline(n);
		lastcmd=wp->wi_dol;	/* remember most recent command */
		wp->wi_dol--;
		}
	else
		strcpy(linebuf,line);
	append(wp,getstring,wp->wi_dol);
	shift(wp,wp->wi_last,wp->wi_dol);
	restorloc();

	/* send line to shell */
	shellactive++;
	send(&shellproc,line);
	}

/*
 * send file lines to shell
 */
linestoshell(wp)
	struct window *wp;
	{
	int *a1;
	int n;
	register char *lp;
	char line[128];

	n=strlen(ps1);
	a1=addr1.ad_addr;
	do{
		lp=strcpy(line,getline(wp->wi_fileno, *a1++));
#ifdef debugging
debug(94,"linestoshell: line=%s",lp);
#endif
		if(strncmp(lp,ps1,n)==0)
			lp += n;	/* strip prompt */
#ifdef debugging
debug(94,"after stripping line=%s",lp);
#endif
		sendtoshell(lp);
		} while(a1 <= addr2.ad_addr);
	}

/*
 * open pipes and fork background process
 */
makeshell()
	{
	if((ps1=getenv("PS1"))==0)	/* look for redefined prompt */
#ifdef vax
		ps1="% ";
		ps1="$ ";	/* since we're using the bell shell */
#else
		ps1="$ ";		/* else use default */
#endif
	makeproc(&shellproc,shprog);
	signal(SIGSHELL,onfpe);	/* because makeshell waits for first prompt */
	}

killshell()
	{
	killproc(&shellproc);
	lastcmd=0;
	}

sigshell(signo)
	{
	if(shellpid==0)
		errfunc("No shell");
	if(signo==0){	/* send eof */
		fflush(toshell);
		write(shellproc.pr_siofd,"",0);
		return;
		}
	if(signo==SIGKILL)
		killshell();
	kill(shellpid,signo);
	}

flushpipe()
	{
	register c;
	register struct window *wp = w[0];
/* the test for pipe empty is now in filbuf
	struct stat psbuf;

fstat(soofd,&psbuf);
debug(48,"st_size=%D",psbuf.st_size);
	write(soofd,"\177",1);	/* mark end of pipe */
	if((c=getc(fmshell))=='\177')	/* pipe was empty */
		return;
	ungetc(c,fmshell);
	saveloc();
	pipecount=0;
	append(wp,getpipe,wp->wi_dol);

	/* see if temp file needs truncating */
	if(wp->wi_dol-wp->wi_zero > shhiwat)
		renum();

	/** this test is no longer necessary because of the test for
	 ** pipe empty */
	if(pipecount)	/* don't change screen if nothing was added */
		shift(wp,wp->wi_last,wp->wi_dol);
	restorloc();
#define DEFERSIG 1	/* for testing */
#ifdef	DEFERSIG
	psync();	/* berkley restarts the tty read */
	flush();
#endif
	}

getpipe()
	{
	static char line[256];
	static char *lp=line;
	register c;

	while((c=getc(fmshell))!='\177'){
		pipecount++;
		*lp = c;
		if(c=='\n' || c=='\001'){
			*lp='\0';
			lp=line;
			strcpy(linebuf,line);
			return(0);
			}
		else
			lp++;
		}
	return(EOF);
	}

getstring()
	{
	static int toggle=0;	/* what a kludge */

	return(toggle++ & 1);
	}


/*
 * truncate temporary file
 ** terrible things will happen if this routine is called while
 ** the shell file is being modified.
 ** this should probably be locked out.
 */
#include "file.h"
renum()
	{
	int n;
	register struct window *wp=w[0];
	int fn;
	int *sp, *tp;

	n=(wp->wi_dol-wp->wi_zero)-shlowat;
	if(n<=0) return;
	fn = wp->wi_fileno;
	filedata[fn].tline=filedata[fn].nleft=0;
	/** lastcmd needs to be updated here */
	for(tp=wp->wi_zero, sp=wp->wi_zero+n; sp<=wp->wi_dol; sp++, tp++){
		getline(fn, *sp);
		*tp = putline(fn);
		}
	wp->wi_dol=wp->wi_dot=tp-1;
	clearwindow(wp);
	}

/*
 * this version of filbuf has been modified
 * to return '\177' if it attempts to read an empty pipe
 * it should still work with regular files but
 * probably won't work with ttys
 */

char	*malloc();


int
_filbuf(iop)
	register FILE *iop;
	{
	static char smallbuf[_NFILE];
	struct stat psbuf;

#ifndef vax
	if (iop->_flag & _IORW)
		iop->_flag |= _IOREAD;
#endif

	if ((iop->_flag & _IOREAD) == 0 || iop->_flag & _IOSTRG)
		return(EOF);

tryagain:
	if (iop->_base == NULL) {
		if (iop->_flag & _IONBF) {
			iop->_base = &smallbuf[fileno(iop)];
			goto tryagain;
			}
		if ((iop->_base = malloc(BUFSIZ)) == NULL) {
			iop->_flag |= _IONBF;
			goto tryagain;
			}
		iop->_flag |= _IOMYBUF;
		}
	iop->_ptr = iop->_base;
	/* this is the added code */
	fstat(fileno(iop), &psbuf);
	if(psbuf.st_size==0){
		iop->_cnt=0;
		return('\177');
		}
	/* ^ */
	iop->_cnt = read(fileno(iop), iop->_ptr, iop->_flag&_IONBF?1:BUFSIZ);
	if (--iop->_cnt < 0) {
		if (iop->_cnt == -1) {
			iop->_flag |= _IOEOF;
#ifndef vax
			if (iop->_flag & _IORW)
				iop->_flag &= ~_IOREAD;
#endif
			}
		else
			iop->_flag |= _IOERR;
		iop->_cnt = 0;
		return(EOF);
		}
	return(*iop->_ptr++ & 0377);
	}

/*
 * old-fashioned callunix
 */
callunix(line)
	char *line;
	{
	register char *cptr;
	register pid,rpid;
	int (*savint)();
	int retcode;
	char c;

	setnoaddr();
	erasescreen();
	cleanup();

#ifdef old
	for(cptr = &screen[cmdline].sl_text[1]; *cptr != '\n'; cptr++)
		if(*cptr == '\t' || *cptr == intab)
			*cptr = ' ';
	for(cptr++; cptr < screen[cmdline].sl_text[ed_lline]; cptr++);
		*cptr = ' ';
#endif

	if((pid = fork()) == 0){
#ifndef vax
		nice(-40);
		nice(20);
		nice(0);
#endif
		signal(SIGHUP,oldhup);
		signal(SIGQUIT,oldquit);
		signal(SIGALRM,oldalrm);
#ifdef old
		execl("/bin/sh","sh","-c",&screen[cmdline].sl_text[1],0);
#else
		execl("/bin/sh","sh","-c",line,0);
#endif
		exit();
		}
	savint=signal(SIGINT,1);
	while((rpid=wait(&retcode)) != pid && pid != -1);
	signal(SIGINT,savint);
	printf("!\n");
	flush();
	do{
		read(0,&c,1);
		} while(c!='\n');
	tsetup();
	redraw();
	statflag++;
	cmlptr = -1;
	}

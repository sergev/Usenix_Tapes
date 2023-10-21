/*
 * initialization/cleanup routines
 */
#define testing 1

#include "terminal.h"
#include "window.h"
#include "ed.h"
#include "edit.h"
#include "file.h"
#include "process.h"
#include "shell.h"

struct stat ttysbuf;

#ifdef vax
#define CERASE '\0'
#define CKILL '\0'
#endif

#define textcol 7	/* temp */

int exparray[]={1,	1,	1,	1,	1,
	/*	0	50	75	110	134.5	*/
		1,	1,	1,	2,	3,
	/*	150	200	300	600	1200	*/
		4,	6,	12,	24,	48
	/*	1800	2400	4800	9600	19200	*/
	};

cleanup()	/* restore tty to usable state */
	{
	trestor();	/* trestor does a stty ~XTABS */
	sgtty.sg_flags=origmode;
#ifdef notyet	/** this keeps loging me out */
	ioctl(0,TIOCSETN,(struct sgttyb*)&sgtty);
#else
	stty(0,(struct sgttyb*)&sgtty);
#endif

	chmod(tty,(int)ttysbuf.st_mode&0777);
	}

/** should be able to specify window size here */
init(fn)
	{
	register pid;
	register struct filedata *fp;
	register struct window *wp;

	/* set up windows */
	/* it is important that the window be set up
	 * before gettemp is called since it references
	 * dot, dol, and zero */
#ifdef debugging
debug(62,"init: fn=%d",fn);
#endif
	if(fn>=cmdwindow)
		errfunc("Bad window %d",fn);
	if(window[fn].wi_flags&wi_active)
		wp = &window[fn];
	else{
		/* make a smaller window for shell */
		int size=(nlines+1)/(nwindows+((fn==0)?2:1));
		if(fn==helpwindow) size=11;
		wp=addwindow(fn,wi_edit,size);
		}
	wp->wi_fileno=fn;
	w[fn]=wp;
	if(wp->wi_zero){		/* window is being reused */
		/** this code should be part of release */
		syncbuf(fn);
		free((char*)wp->wi_zero);
		}
	if((wp->wi_zero = (int*)malloc((wp->wi_nlall=128)*sizeof(int)))==(int*)0)
		errfunc("Out of memory");
	wp->wi_dot=wp->wi_dol=wp->wi_zero;

	fp = &filedata[fn];
	fp->modflag = (globp? -1 : 0);	/* initial file "r" doesn't count */
	close(fp->sfile);
	close(fp->tfile);
	fp->tline = 0;
	fp->iblock = -1;
	fp->oblock = -1;
	fp->ichanged = 0;
	gettemp(fn);
	/** with this code here, the mark table is destroyed each
	 ** time a window is added. if anything, it should be
	 ** destroyed when a window is deleted, but in fact, only
	 ** the invalid marks should be cleared */
	for(pid=0; pid<26; pid++)	/* init mark table */
		namet[pid]=26;

	showstat(wp);
	}

quit()
	{
	register struct filedata *fp;

	for(fp = &filedata[0]; fp<&filedata[maxfiles]; fp++){
		if(fp->tfile == -1) continue;
		unlink(fp->tfname);
		unlink(fp->sfname);
		}
	deleteline(errline);
	deleteline(errline);	/* get comdline */
	cleanup();
	if(proflag)
		monitor(0);
	killshell();
	exit(0);
	}

setup()
	{
	register struct filedata *fp;
	register struct sline *sp;

	tsetup();
	if(expsize==0)	/* allow environment override */
		expsize=exparray[sgtty.sg_ospeed];	/* set expansion limit */

	/* initialize the filedata structure */
	for(fp=filedata; fp < &filedata[maxfiles]; fp++){
		fp->tfile=fp->sfile = -1;
		fp->iblock=fp->oblock = -1;
		}

	/* mark the screen dirty */
	for(sp=screen; sp < &screen[35]; sp++)
		sp->sl_flags=sl_written;

	/* initialize the buffers */
	initbuf();

	/* setup the command window */
	w[cmdwindow]=addwindow(cmdwindow,wi_comd,1);
	}

tsetup()
	{
	tinit();
	if(gtty(0,(struct sgttyb*)&sgtty)<0){	/* input from script */
		sgtty.sg_erase=CERASE;
		sgtty.sg_kill=CKILL;
		}
	origmode=sgtty.sg_flags;
	sgtty.sg_flags |= CBREAK;	/* set rare mode */
	sgtty.sg_flags &= ~ECHO;	/* set no echo */
#ifdef eunice
	stty(0,(struct sgttyb*)&sgtty);
#else
	ioctl(0,TIOCSETN,(struct sgttyb*)&sgtty);
#endif

	/* forbid messages */
	if((tty=ttyname(2))==0){	/* in case some idiot does !edit */
		printf("Not a tty\n");
		flush();
		cleanup();
		exit(1);
		}
	if(stat(tty,&ttysbuf)<0 || chmod(tty,0600)<0){
		/** should eventually just get rid of this */
		printf("Can't change mode\n");
		flush();
#ifndef testing
		cleanup();
		exit(1);
#endif
		}
	settabs(textcol,tabspace);
#ifdef debugging
debug(16,"setup: origmode=0%o",origmode);
#endif
	}

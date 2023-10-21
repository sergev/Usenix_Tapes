#include <stdio.h>
#include <errno.h>
#include "window.h"
#include "terminal.h"
#include "edit.h"
#include "ed.h"
#include "process.h"
#include "shell.h"
#include "spell.h"

int onalarm();

getchr()
	{
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
		}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(EOF);
		}
	/** this is not really the most logical place to put this
	 ** but it should work */
	if(askhelp){
		/** should be msg */
		errmsg("Type \"help\" for help.");
		askhelp=0;
		}
	return(lastc = getcc());
	}

int cmlptr = -1;
int loading;	/* communication from loadbuf */
getcc()		/* get a character from the command line */
	{
	register c;
	register struct sline *sp;

	if(cmlptr < 0){
		clearsline(cmdline);
		if(loading)
			printf("%c> ",loading);
		else{
			prompt[0]=wname(fc);
			tputs(prompt);
			}
		if(enscreen(cmdline,0) == esc){
			cmlptr = -1;
			return(esc);
			}
		}
	sp = &screen[cmdline];
#ifdef debugging
debug(31,"getcc: cmlptr=%d last=%d c=0%o",cmlptr,sp->sl_last,sp->sl_text[cmlptr]);
#endif
	while((c=sp->sl_text[cmlptr++]) == intab)
		;
	if((cmlptr > sp->sl_last+1) || (c == '\n')){
		cmlptr = -1;
		return('\n');
		}
	else
		return(c);
	}

nextcc()	/* return next character in comand */
	{
	register c;
	register struct sline *sp;

	if(peekc)
		return(peekc);
	if(globp && *globp)
		return(*globp);
	sp = &screen[cmdline];
	if(cmlptr>0 && cmlptr<=sp->sl_last && (c=sp->sl_text[cmlptr])!='\n')
		return(c);
	return(0);
	}

static int rarecount;
rarec()
	{
	char c;
	extern errno;
	extern sourcelev, rfile[];

/* this has been moved to mapc
	if(globp){
		if(c = *globp++)
			return(c);
		globp=0;
		}
*/

    resource:
	if(sourcelev){
		/** sould be using buffered input */
		if(read(rfile[sourcelev], &c, 1)<=0){
			close(rfile[sourcelev--]);
			goto resource;
			}
		return(c&0177);
		}

    reread:
	psync();
	flush();
#ifdef DEFERSIG
	signal(SIGALRM,DEFERSIG(onalarm));
	if(shellpid)
		signal(SIGSHELL,DEFERSIG(onfpe));
	if(spellpid)
		signal(SIGEMT,DEFERSIG(onemt));
#else
#ifdef SIG_HOLD
	signal(SIGALRM,onalarm);
	if(shellpid)
		signal(SIGSHELL,onfpe);
	if(spellpid)
		signal(SIGEMT,onemt);
#endif
#endif
	if(read(0,&c,1)<=0){
#ifdef debugging
debug(15, "errno=%d",errno);
#endif
		if(errno==EINTR)
			goto reread;
		else
			return(EOF);
		}
#ifdef debugging
debug(99,"c=0%o(%c)",c,c);
#endif
#ifdef SIG_HOLD
	signal(SIGALRM,SIG_HOLD);
	if(shellpid)
		signal(SIGSHELL,SIG_HOLD);
	if(spellpid)
		signal(SIGEMT,SIG_HOLD);
#else
	rarecount++;
	if((rarecount % 100)==0)
		synctemp();
	if(shellpid && (rarecount % 5)==0)
		flushpipe();
	if(spellpid && (rarecount % 5)==0)
		getbadword();
#endif
	if(sflag)
		write(scriptfd,&c,1);
#ifdef debugging
	if(c == '\026'){	/* ^V */
		display(0);
		goto reread;
		}
	else if(c == '\016'){	/* ^N */
		display(1);
		goto reread;
		}
	else if(c == '\027'){	/* ^W */
		display(2);
		goto reread;
		}
	else if(c == '\024'){	/* ^T */
		display(3);
		goto reread;
		}
#endif
	if(c == ('R'&037)){	/* ^R redraw screen */
		redraw();
		goto reread;
		}
	c &= 0177;
	return(c);
	}

/*
 * misc ed subroutines
 */

#include "ed.h"
#include "window.h"
#include "edit.h"
#include "file.h"
#include "process.h"
#include "shell.h"

static struct window *ws;	/* source window for getcopy */

append(wp, f, a)
	register struct window *wp;
	int (*f)();
	int *a;
	{
	register nline;

	filedata[wp->wi_fileno].modflag++;
	nline = 0;
	wp->wi_dot = a;
	while ((*f)() == 0) {
		*makeslot(wp,wp->wi_dot,1)=putline(wp->wi_fileno);
		nline++;
		}
	return(nline);
	}

delete(wp)
	struct window *wp;
	{
	register *a1, *a2, *a3;
	int tl;

	filedata[wp->wi_fileno].modflag++;
	a1 = addr1.ad_addr;
	a2 = addr2.ad_addr+1;
/*
	for(tl=0; tl<highmark; tl++);
		if(marks[tl] >= a2)
			marks[tl] =- a2-a1;
		else if(marks[tl] >= a1)
			marks[tl] = a1-1;
*/
	a3 = wp->wi_dol;
#ifdef debugging
debug(58,"delete: a1=%d a2=%d a3=%d",a1-wp->wi_zero,a2-wp->wi_zero,a3-wp->wi_zero);
#endif
	wp->wi_dol -= a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1.ad_addr;
	if (a1 > wp->wi_dol)
		a1 = wp->wi_dol;
	wp->wi_dot = a1;
	}

/* VARARGS1 */
errfunc(msg,a1,a2,a3,a4)
	char *msg;
	{
	register c;

	listf = 0;
	errmsg(msg,a1,a2,a3,a4);
	count[0] = 0;
	lseek(0, (long)0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchr()) != '\n' && c != EOF && c!=EOL)
		;
	if (io > 0) {
		close(io);
		io = -1;
		}
	if(jumpset)
		longjmp(savej,1);
	quit();
	}

/* VARARGS1 */
errmsg(msg,a1,a2,a3,a4)	/* error message print routine */
	char *msg;
	{
	saveloc();
	topt(errline,errcol,(char*)0);
	el();
	/* print message and offending command */
/**	printf("\007## %s: %s ##",msg,screen[cmdline].text);
*/	printf("\007## ");
	printf(msg,a1,a2,a3,a4);
	printf(" ##");
	restorloc();
	psync();
	flush();
	if(dbflag==25) abort();
/*	mode=cmdmode;	/* temporary kludge */
	helping=0;
	}

getcopy()
	{
	if (addr1.ad_addr > addr2.ad_addr)
		return(EOF);
	getline(ws->wi_fileno, *addr1.ad_addr++);
	return(0);
	}

/** this code could be used by gnv and pov */
/* VARARGS1 */
message(msg,a1,a2,a3,a4)	/* print a message on the error line */
	char *msg;
	{
	saveloc();
	topt(errline,errcol,(char*)0);
	el();
	printf(msg,a1,a2,a3,a4);
	restorloc();
	flush();	/** this may not be needed unless this is
			 ** used by errmsg */
	}

/** this is not completely configured for cross window moves */
move(wp,cflag)
	struct window *wp;
	{
	register int *adt, *ad1, *ad2;
	struct window *wt;	/* target window */
	struct address addr;

	setdot(wp);
	nonzero();
	addr=address();
	if ((adt=addr.ad_addr)==0)
		errfunc("No destination address");
	newline();
	ad1 = addr1.ad_addr;
	ad2 = addr2.ad_addr;
	ws = addr1.ad_window;
	wt = addr.ad_window;
	if(!cflag && wt!=ws)
		errfunc("Can't move between windows. Use 't'.");
	filedata[wt->wi_fileno].modflag++;
	if (cflag) {
		int *ozero,delta;
		ad1 = wt->wi_dol;
		ozero = wt->wi_zero;
		append(wt, getcopy, ad1++);
		ad2 = wt->wi_dol;
		delta=wt->wi_zero-ozero;
		ad1 += delta;
		adt += delta;
		ws=wt;		/** this only allows cross window t's */
		}
	else
		filedata[ws->wi_fileno].modflag++;
	ad2++;
	if (adt<ad1) {
		ws->wi_dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
		}
	else if (adt >= ad2) {
		ws->wi_dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
		}
	else
		errfunc("Invalid move range");
	}

newline()
	{
	register c;

	eraseline(errline);
	if ((c = getchr()) == '\n' || c == EOF)	/* fix for global strings */
		return;
	if (c=='p' || c=='l') {
		pflag++;
		if (c=='l')
			listf++;
		if ((c = getchr()) == '\n' || c == EOF)	/* fix for global strings */
			return;
		}
#ifdef debugging
debug(78,"newline: globp=%d",globp);
#endif
	errfunc("No arguments allowed");
	}

/** this should scan all active files */
onhup()
	{
	signal(SIGINT,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	if(wc->wi_dol>wc->wi_zero){
		addr1.ad_addr=wc->wi_zero+1;
		addr2.ad_addr=wc->wi_dol;
		io=creat("ed.hup",0660);
		if(io>0)
			putfile(wc);
		}
	quit();
	}

nonzero()
	{
	register struct window *wp;

	wp=addr1.ad_window;
	if (addr1.ad_addr<=wp->wi_zero || addr2.ad_addr>wp->wi_dol)
		errfunc("Address out of range");
	}

reverse(a1, a2)
	register int *a1, *a2;
	{
	register int t;

	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
		}
	}

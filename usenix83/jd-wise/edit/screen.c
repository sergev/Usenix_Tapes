/*
 * screen management subroutines
 */

#include "window.h"
#include "ed.h"
#include "edit.h"
#include "term.h"
#include "terminal.h"

/** this is an anacronism */
clearscreen()	/* clear screen structure and erase text portion of screen */
	{
	register int j;

	home();
	ep();		/* status line is protected! */
	for(j=0; j<nlines; j++)
		clearstruct(j);
/*
	firstused = nlines;
	lastused = -1;
*/
	}

deletes(wp)	/* delete lines from window and buffer */
	struct window *wp;
	{
	register l;
	register int *a1,*a2;

#ifdef debugging
debug(8,"entering deletes globp=%o, mode=%o",globp,mode);
#endif
/**	if(!globp){	g comd requires this */
		setdot(wp);
		newline();
		nonzero();
/**		}*/
	/*clearat(wp); now done by setat */
	a1=addr1.ad_addr;
	a2=addr2.ad_addr;
#ifdef debugging
debug(34,"deletes: addr1=0%o addr2=0%o dot=0%o",addr1.ad_addr,addr2.ad_addr,wp->wi_dot);
debug(8,"before delete mode=%d",mode);
#endif
	yank(0);
	delete(wp);
	for(l=wp->wi_first; l<=wp->wi_last; l++){
		if(screen[l].sl_flags == sl_empty)
			continue;
		if(screen[l].sl_addr == a1){
			do{
				if(!(screen[l].sl_flags & sl_conting))
					a2--;
				closeup(wp,l,1);
				}
			while(screen[l].sl_flags != sl_empty
				&& screen[l].sl_addr <= a2);
			break;
			}
		}
	/* assure at least some context */
	if(l<3 && wp->wi_dot > (wp->wi_zero+3))	/* don't shift if first line */
					/* or empty screen */
		shift(wp,3+wp->wi_first,wp->wi_dot);
	else if(l>wp->wi_last-4)
		shift(wp,wp->wi_last-4,wp->wi_dot);
	else
		expand(wp);
#ifdef debugging
debug(8,"before setat mode=%d",mode);
#endif
	setat(wp);
	}

expand(wp)	/* fill the screen */
	struct window *wp;
	{
	register *a1,n;

	setused(wp);	/* temporary till used maintained incrementally */
	txflag = 0;
	if(wp->wi_ulast < 0) return;	/* screen empty */

	/* expand up */
	a1=screen[wp->wi_ufirst].sl_addr;
	n=0;
	while(--a1>wp->wi_zero && installine(wp,wp->wi_ufirst-1,a1)>=0)
		if((++n >= expsize)
		&& (wp != &window[helpwindow]))
			break;

	/* expand down */
	a1=screen[wp->wi_ulast].sl_addr;
	n=0;
	while(++a1<=wp->wi_dol && installine(wp,wp->wi_ulast+1,a1)>=0)
		if((++n >= expsize)
		&& (wp != &window[helpwindow]))
			break;
	}

findfirst(slno)
	{
	register l;

	if(screen[slno].sl_flags==sl_empty) return(-1);
	for(l=slno; screen[l].sl_flags&sl_conting; l--);
	return(l);
	}

findlast(slno)
	{
	register l;

	for(l=slno; screen[l].sl_flags&sl_contd; l++);
	return(l);
	}

getonscreen(wp,addr)	/* return screen line # */
	register struct window *wp;
	register int *addr;
	{
	register l;

	if((l=getslno(addr))<0)
		shift(wp, l=wp->wi_defline, addr);
	return(l);
	}

getslno(addr)		/* get screen line # of text line addr */
	int *addr;	/* return -1 if not on screen */
	{
	register struct sline *sptr;
	register l;

	for(l=0; l<nlines; l++){
		sptr = &screen[l];
		if(sptr->sl_flags == sl_empty)
			continue;
		if(sptr->sl_addr == addr)
			return(l);
		}
	return(-1);
	}

oneline(wp,addr)
	register struct window *wp;
	int * addr;
	{
	clearwindow(wp);
	installine(wp,wp->wi_defline,addr);
	}

setused(wp)	/* update wp->wi_ufirst and wp->wi_ulast */
	register struct window *wp;
	{
	register l;

	wp->wi_ufirst = wp->wi_last+1;
	wp->wi_ulast = -1;

	for(l=wp->wi_first; l<=wp->wi_last; l++)
		if(screen[l].sl_flags != sl_empty){
			wp->wi_ufirst=l;
			goto lookup;
			}
	return;	/* screen is empty */
    lookup:
	for(l=wp->wi_last; l>=wp->wi_first; l--)
		if(screen[l].sl_flags != sl_empty){
			wp->wi_ulast=l;
#ifdef debugging
debug(17,"setused: first=%d last=%d",wp->wi_ufirst,wp->wi_ulast);
#endif
			return;
			}
	}

shift(wp,lno,addr)	/* position addressed line at window line */
	register struct window *wp;
	int *addr, lno;
	{
	register clno;
	int down;
	int excount;

#ifdef debugging
debug(6,"shift: w=%d lno=%d flno=%d",
wp-window,lno,addr-wp->wi_zero);
#endif
	if((clno=getslno(addr)) == lno)
		return;	/* already there */
#ifdef debugging
debug(6,"in shift clno=%d",clno);
#endif
	if(clno < 0	/* not on the screen */
	|| (pagethresh && abs(lno-clno)>pagethresh)){	/* paging */
		int n;

		setused(wp);
		if(lno==wp->wi_first
		   && wp->wi_ufirst <= wp->wi_last
		   && (n=screen[wp->wi_ufirst].sl_addr-addr)<3
		   && n>0){
			clno=wp->wi_ufirst-n;
			}
		else if(lno==wp->wi_last
		   && wp->wi_ulast>=0
		   && (n=addr-screen[wp->wi_ulast].sl_addr)<3
		   && n>0){
			clno=wp->wi_ulast+n;
			}
		else{
		   brute:
			clearwindow(wp);
			installine(wp,lno,addr);
/*
			for(clno=slno; screen[clno].sl_flags != sl_empty; clno++)
				showline(clno);
*/
			expand(wp);
			return;
			}
		}
	down = clno<lno;

	/*clearat(wp); now done by setat */
#ifdef creep
	if(clno < lno)	/* move down */
		for(;clno < lno; clno++)
			opendown(wp,wp->wi_first,1);

	else		/* move up */
		for(; clno > lno; clno--)
			openup(wp,wp->wi_last,1);

#else
	if(clno<lno)	/* move down */
		opendown(wp,wp->wi_first,lno-clno);
	else		/* move up */
		/** there is an inconsistency here between openup & down
		 ** this interacts with code in openup */
		openup(wp,wp->wi_last+1-(clno-lno),clno-lno);
#endif
	expand(wp);
	for(excount=0; screen[lno].sl_addr != addr; excount++){
		if(excount>5) goto brute;
		if(down)
			opendown(wp,wp->wi_first,1);
		else
			openup(wp,wp->wi_last,1);
		expand(wp);
		}
	setat(wp);
	}

redraw()	/* completely redraw the screen */
	{
	register l;

	saveloc();
	if(termtype==INFOTON)
		printf("\014");	/* clear protected lines */
	erasescreen();
	for(l=0; l<errline; l++){
		screen[l].sl_data[0]|=0200;	/* fake out update */
		if(l==screen[l].sl_window->wi_statline){
			setbgnd();
			showline(l);
			set4gnd();
			}
		else
			showline(l);
		}
	restorloc();
	psync();
	flush();
	}

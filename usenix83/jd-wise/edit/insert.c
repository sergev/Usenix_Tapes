#include "window.h"
#include "ed.h"
#include "edit.h"
#include "enscreen.h"
#include "shell.h"	/* for lastcmd */

/** window vs screen line numbers are totally confused here */
makeblankline(wp,addr)
	register struct window *wp;
	int *addr;
	{
	int l,m;
	register struct sline *sp;

	if(wp->wi_dol==wp->wi_zero){	/* no lines on screen */
		l=wp->wi_defline;
		wp->wi_dot=addr=wp->wi_zero;
		}
	else if(wp->wi_dot==wp->wi_zero){	/* insert at top */
		shift(wp,wp->wi_defline,wp->wi_zero+1);
		l=wp->wi_defline;
		}
	else{	/** this is rather crude */
#ifdef debugging
debug(33,"before getonscreen");
#endif
		m=getonscreen(wp, addr);
#ifdef debugging
debug(33,"after getonscreen");
#endif
		if((l=(findlast(m)+1))>wp->wi_defline){
			shift(wp,wp->wi_defline+m-l+1,addr);
			l=wp->wi_defline;
			openup(wp,l,1);
			goto opened;
			}
		}
#ifdef debugging
debug(33,"before opendown");
#endif
	opendown(wp, l, 1);
   opened:
#ifdef debugging
debug(33,"after opendown");
#endif
	sp = &screen[l];
	sp->sl_addr=makeslot(wp,addr,1);
	/* sl_modified must be set so an empty line will writeout */
	sp->sl_flags=sl_written|sl_modified;
	/* at this point, all addresses at and past this line
	 * have been stepped by one */
	for(inline=l; l<=wp->wi_last; l++, sp++){
		if(sp->sl_flags==sl_empty) break;
		if(l>inline) sp->sl_addr++;
		sp->sl_lno=sp->sl_addr-wp->wi_zero;
		showdata(l);
		}
	return(inline);
	}

/* get things set up to do an insert
 * the actual work is done in dispatch
 */
appendonscreen(wp, addr)
	struct window *wp;
	int *addr;
	{

	if(globp)
		errfunc("insert not permitted in global");
	clearat(wp);
	wp->wi_dot = addr;
	showstat(wp);
	}


/* this can also be used by il in which case flag=0=> don't change dot */
int *
makeslot(wp,addr,flag)
	register struct window *wp;
	int *addr;
	{
	register *a1, *a2;
	int *rdot;

/** see below
	if(flag)
		wp->wi_dot = addr;
^*/
	if((wp->wi_dol-wp->wi_zero)+1 >= wp->wi_nlall){
		int *ozero=wp->wi_zero;
		int l,diff;
		wp->wi_nlall += 512;
		free((char *)wp->wi_zero);
		if((wp->wi_zero=(int*)realloc((char*)wp->wi_zero, wp->wi_nlall*sizeof(int)))==(int*)0){
			lastc='\n';
			wp->wi_zero=ozero;
			errfunc("Out of memory");
			}
		diff = wp->wi_zero - ozero;
		wp->wi_dot += diff;
		wp->wi_dol += diff;
		if(wp->wi_fileno==0)
			lastcmd += diff;
		addr += diff;
		if(insertadr != (int*)(-1))
			insertadr += diff;
		/* fix up addresses in screen structure */
		setused(wp);
		for(l=wp->wi_ufirst; l<=wp->wi_ulast; l++)
			screen[l].sl_addr += diff;
		}
	a1 = ++wp->wi_dol;
	a2 = a1+1;
	/** this could be cleaned up */
	/** in particular, simply retain the consequence of the else */
	/** and delete flag */
	/** thusly ...
	if(flag)
		rdot = ++wp->wi_dot;
	else{
	**/ {
		if(wp->wi_dot>=addr) wp->wi_dot++;
		if(insertadr!=(int *)(-1) && insertadr>=addr) insertadr++;
		rdot = ++addr;
		}
	/*^*/
	while (a1 > rdot)
		*--a2 = *--a1;
	*rdot = -1;	/* an empty slot has no disk addr */
	return(rdot);
	}

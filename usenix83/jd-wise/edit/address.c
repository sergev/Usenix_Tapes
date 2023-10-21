/*
 * address routines
 */

#include "ed.h"
#include "window.h"
#include "edit.h"	/* so wc & errline are defined */
#include "file.h"	/* for maxfiles */
#include "shell.h"	/* for lastcmd */

struct address
address()
	{
	register *a1, minus, c;
	int n, relerr;
	struct window *wp;
	struct address addr;

	minus = 0;
	a1 = 0;
	wp = wc;	/* default to current window */
	for (;;) {
		c = getchr();
#ifdef debugging
debug(19,"address: char=%c (0%o)",c,c);
#endif
		if ('0'<=c && c<='9') {
			n = 0;
			do {
				n *= 10;
				n += c - '0';
				}
			while ((c = getchr())>='0' && c<='9');
			peekc = c;
			if (a1==0)
				a1 = wp->wi_zero;
			if (minus<0)
				n = -n;
			a1 += n;
			minus = 0;
			continue;
			}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch(c) {
		case ' ':
		case '\t':
			continue;
	
		case ':':	/* ":w" window specifier */
			c=getchr();
		acolon:
			c=wnumber(c);
			if(c<0 || c>=maxfiles || w[c]==(struct window *)0)
				errfunc("Bad window specifier in address");
			wp = w[c];
			continue;
		case '+':
			minus++;
			if (a1==0)
				a1 = wp->wi_dot;
			continue;

		case '-':
/*		case '^':*/
			minus--;
			if (a1==0)
				a1 = wp->wi_dot;
			continue;
	
		case '?':
		case '/':
			compile(c);
			a1 = wp->wi_dot;
			for (;;) {
				if (c=='/') {
					a1++;
					if (a1 > wp->wi_dol)
						a1 = wp->wi_zero;
					}
				else {
					a1--;
					if (a1 < wp->wi_zero)
						a1 = wp->wi_dol;
					}
				if (execute(wp, 0, a1))
					break;
				if (a1==wp->wi_dot)
					errfunc(helping?"No information available":"String not found");
				}
			break;
	
		case '$':
			a1 = wp->wi_dol;
			break;
	
		case '.':
			a1 = wp->wi_dot;
			break;

		case '%':	/* last shell command */
			if((wp=w[shellwindow])==0)
				errfunc("No shell window");
			a1=lastcmd;
			break;

		case '`':
		case '\'':
#ifdef undef
			if((c=getchr())<'a' || c>'z' || (c=namet[c-'a']) == 26)
				if(c<'a'){	/* `A => `a,`b ... */
					c=c-'A';
					addr1=marks[namet[c]]; /* `a */
					a1=marks[namet[c+1]]; /* `b */
					break;
					}
				else
					errfunc("Bad mark name");
			a1=marks[c];
#endif
			if((c=getchr())<'a' || c>'z')
				errfunc("Bad mark name");
			c=namet[c-'a'];
			if(c>highmark)
				errfunc("Mark not set");
			wp=marks[c].mk_window;
			for(a1=wp->wi_zero; a1<=wp->wi_dol; a1++)
				if(marks[c].mk_addr == (*a1|01)) break;
			break;
	
		default:
			if(wnumber(c)>=0 && nextcc()==':'){
				getchr();	/* eat the colon */
				goto acolon;
				}
			peekc = c;
			addr.ad_window=wp;
			addr.ad_addr=a1;
			if (a1==0)
				return(addr);
			addr.ad_addr=(a1 += minus);
			if (a1<wp->wi_zero || a1>wp->wi_dol)
				errfunc("Address out of range");
			return(addr);
			}
		if (relerr)
			errfunc("relerr");
		}
	}

setall(wp)
	register struct window *wp;
	{
	if (addr2.ad_addr==0) {
		addr1.ad_addr = wp->wi_zero+1;
		addr2.ad_addr = wp->wi_dol;
		if (wp->wi_dol==wp->wi_zero)
			addr1.ad_addr = wp->wi_zero;
		}
	setdot(wp);
	}

setdot(wp)
	register struct window *wp;
	{
	if (addr2.ad_addr == 0)
		addr1.ad_addr = addr2.ad_addr = wp->wi_dot;
	if (addr1.ad_addr > addr2.ad_addr)
		errfunc("Second address must be larger");
	}

setnoaddr()
	{
	if (addr2.ad_addr)
		errfunc("No address allowed");
	}

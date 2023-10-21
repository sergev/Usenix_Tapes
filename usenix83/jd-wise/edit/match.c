#include "ed.h"
#include "window.h"
#include "edit.h"
#include "scan.h"

int	*llimit;
int	(*lf)();
int	dir;

any(c,s)
	register c;
	register char *s;
	{
	register x;

	while(x = *s++){
#ifdef debugging
debug(66,"any: s=%s c=%c x=%c",s,c,x);
#endif
		if(x==c)
			return(1);
		}
	return(0);
	}

char *
strend(cp)
	register char *cp;
	{
	while(*cp)
		cp++;
	return(cp);
	}

match(line,col)
	{
	register int i;
	register char *parens, *cp;
	struct window *wp;

	startscan(line,col);
	wp=wwp;
#ifdef debugging
debug(65,"after startscan cursor=%d *cursor=%c",cursor-linebuf,*cursor);
#endif
	for (cp = cursor; !any(*cp, "({[)}]");)
		if (*cp++ == 0)
			return (0);
	lf = 0;
	parens = any(*cp, "()") ? "()" : any(*cp, "[]") ? "[]" : "{}";
	if (*cp == parens[1]) {
		dir = -1;
		llimit = wp->wi_zero+1;
		}
	else {
		dir = 1;
		llimit = wp->wi_dol;
		}
#ifdef debugging
debug(65,"match: line=%d col=%d cp=%d *cp=%c parens=%s dir=%d",
line,col,cp-linebuf,*cp,parens,dir);
#endif
	wcursor = cp;
	i = lskipbal(parens);
	if(i){
		int l;
		char saveline[512];
		wwp->wi_dot=wdot;
		*wcursor |= 0200;
		/* getonscreen may destroy linebuf */
		strcpy(saveline,linebuf);
		l=getonscreen(wwp,wdot);
#ifdef debugging
debug(67,"match found: l=%d wdot=%d",l,wdot-wp->wi_zero);
#endif
		updateline(l,saveline);
		setat(wwp);
		tomark(l);
		}
	}

lskipbal(parens)
	register char *parens;
	{
	register int level = dir;
	register int c;

	do {
		if (!lnext()) {
			wdot = NOLINE;
			return (0);
			}
		c = *wcursor;
		if (c == parens[1])
			level--;
		else if (c == parens[0])
			level++;
		} while (level);
	return (1);
	}

lnext()
	{
	int l;

	if (dir > 0) {
		if (*wcursor)
			wcursor++;
		if (*wcursor)
			return (1);
		if (wdot >= llimit) {
			return (0);
			}
		wdot++;
		if((l=getslno(wdot))>=0)
			unmakeline(l);
		else
			getline(wwp->wi_fileno,*wdot);
		wcursor = linebuf;
		return (1);
		}
	else {
		--wcursor;
		if (wcursor >= linebuf)
			return (1);
#ifdef LISPCODE
		if (lf == lindent && linebuf[0] == '(')
			llimit = wdot;
#endif
		if (wdot <= llimit) {
			wcursor = linebuf;
			return (0);
			}
		wdot--;
		if((l=getslno(wdot))>=0)
			unmakeline(l);
		else
			getline(wwp->wi_fileno,*wdot);
		wcursor = linebuf[0] == 0 ? linebuf : strend(linebuf) - 1;
		return (1);
		}
	}

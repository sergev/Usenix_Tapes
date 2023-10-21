#ifndef lint
static char rcsid[] = "$Header: io.c,v 2.1 85/04/10 17:31:07 matt Stab $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines for termcap interface and general i/o support
 * Changes were made to the original code to minimize
 * the total number of "write" calls to the sockets
 * (an attempt to increase performance)
 *
 * Copyright (c) 1981
 *
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "defines.h"
#include "structs.h"

extern int sock;
static struct message message;

/*
 * Goto <x, y> on who's terminal and put
 */
/*VARARGS3*/
void cput(x, y, p, args)
int x, y;
t_player *p;
int args;
{
	char	*print();
	register char *bp;
	register char *qp;
	register c;

	move(x, y, p);
	bp = print(&args);
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}

/*
 * clear screen
 */
void clear(p)
t_player *p;
{
	register char *bp;
	register char *qp;
	register c;

	bp = p->CL;
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}


/*
 * move to <x, y>
 */
move(x, y, p)
int x;
int y;
t_player *p;
{
	extern	char *tgoto();
	register char *bp;
	register char *qp;
	register c;

	bp = tgoto(p, x, y);
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}

/*
 * Do the put, but make sure it occupies the entire line -- for
 *  status and message lines
 */
void /*VARARGS1*/
putblank(p, args)
register t_player *p;
int *args;
{
	char	*print();
	register char *bp;
	register int n;
	register char *qp;
	register c;

	qp = p->eoq;
	if (p->CE) {	/* cheat clear out line first */
		bp = p->CE;
		for (qp=p->eoq; c = *bp; bp++) {
			*qp = c;
			qp++;
		}
		*qp = NULL;
		p->eoq = qp;
		bp = print(args);
	} else {	/* have to overwrite line, then clear out remainder */
		bp = print(args);
		n = strlen(bp);
		while (n++ < 40)	/* kludge for now */
			bp[n] = ' ';
		bp[40] = NULL;
	}
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}

/*
 * Print something on the message line
 */
/*VARARGS1*/
void pmesg(p, args)
t_player *p;
int args;
{
	move(MSDATAX, MSDATAY, p);
	putblank(p, &args);
}


/*
 * Drop something on the prompt line
 */
/*VARARGS1*/
void prompt(p, s)
t_player *p;
char *s;
{
	move(PROMPTX, PROMPTY, p);
	putblank(p, &s);
}

/*
 * Print something on the status line
 */
/*VARARGS1*/
void pstatus(p, args)
t_player *p;
int args;
{
	move(STDATAX, STDATAY, p);
	putblank(p, &args);
}

/*
 * Clear out the rest of the line from where the cursor is
 *  presently located -- easy if CE exists on this terminal
 */
void clearline(p)
register t_player *p;
{
	register char *bp;
	register char *qp;
	register c;

	bp = p->CE;
	if (bp == NULL)
		return;
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}


/*
 * Print write around
 */
/*VARARGS1*/
void put(p, args)
t_player *p;
int args;
{
	char	*print();
	register char *bp;
	register char *qp;
	register c;

	bp = print(&args);
	for (qp=p->eoq; c = *bp; bp++) {
		*qp = c;
		qp++;
	}
	*qp = NULL;
	p->eoq = qp;
}


static char printbuf[80];
static char *bufp;

/*
 * Guts of the I/O...a hacked printf-like beast?
 */
static	char *print(parg)
register int *parg;	/* should be a union, or something */
{
	void	printn();
	register char *fmt = (char *)(*parg++);
	register c;
	register char *bp;

	bufp = printbuf;
	while (c = *fmt++) {
		if (c == '%') switch(c = *fmt++) {
			case 'd':	/* decimal */
				printn(*parg++, 10, 4);
				continue;
			case 'c':	/* character */
				*bufp++ = (char)(*parg++); 
				continue;
			case 's':	/* string */
				bp = (char *)(*parg++);
				for ( ; c = *bp; bp++) {
					*bufp = c;
					bufp++;
				}
				continue;
			case '%':
				*bufp++ = '%';
				continue;
			default:
				*bufp++ = '%';
		}
		*bufp++ = c;
	}
	*bufp = NULL;
	return printbuf;
}


/*
 * Handle number conversions...everything takes up "width" spaces
 */
static	void printn(n, base, width)
register int n, base;
int width;
{
	void	putn();
	char	*before;

	before = bufp;
	if (base == 10 && n < 0) {
		*bufp++ = '-';
		n = abs(n);
	} else
		*bufp++ = ' ';
	if (n == 0)
		*bufp++ = '0';
	else
		putn(n, base);
	n = bufp - before;
	while (n++ < width)
		*bufp++ = ' ';
}


/*
 * Recursive number constructor
 */
static	void putn(n, b)
register int n, b;
{
	if (n == 0)
		return;
	putn(n/b, b);
	*bufp++ = "0123456789abcdef"[n%b];
}


/*
 * Flush the output buffer to "p"s terminal
 */
void bflush(p)
register t_player *p;
{
	int	i;

	i = strlen(p->outputq);
	if (i == 0) return;
	message.mtype = p->uid+1;
	strcpy(message.text, p->outputq);
	msgsnd(sock, &message, i+8, 0);
	p->eoq = p->outputq;
	*p->eoq = NULL;
}


/*
 * Set up the termcap stuff in the player structure
 */
int ioinit(p, pf)
register t_player *p;
register t_file *pf;
{
	extern	char *malloc();
	char	*copy();
	register unsigned	nalloc;
	register char	*cp;

	nalloc = strlen(pf->p_BC)+strlen(pf->p_UP)+ strlen(pf->p_CM)+
		 strlen(pf->p_CL)+strlen(pf->p_CE)+5;
	nalloc += nalloc % 2;
	if ((cp = malloc(nalloc)) == (char *)-1)
		return(0);
	p->BC = cp;
	p->UP = cp = copy(cp, pf->p_BC);
	p->CM = cp = copy(cp, pf->p_UP);
	p->CL = cp = copy(cp, pf->p_CM);
	p->CE = cp = copy(cp, pf->p_CL);
	(void)copy(cp, pf->p_CE);
	p->ttyspeed = pf->p_speed;
	return(1);
}

static	char *
copy(s1, s2)
register char *s1, *s2;
{
	while (*s1++ = *s2++);
	return s1;
}

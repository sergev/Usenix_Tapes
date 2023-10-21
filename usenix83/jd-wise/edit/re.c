/*
 * regular expression routines
 */

#include "window.h"
#include "ed.h"
#include "edit.h"

advance(lp, ep)
	register char *lp, *ep;
	{
	register char *curlp;
	int i;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep += *ep;
			continue;
			}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep += *ep;
			continue;
			}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		if (braelist[i = *ep++]==0)
			errfunc("re advance error");
		if (backref(i, lp)) {
			lp += braelist[i] - braslist[i];
			continue;
			}
		return(0);

	case CBACK|STAR:
		if (braelist[i = *ep++] == 0)
			errfunc("re advance error");
		curlp = lp;
		while (backref(i, lp))
			lp += braelist[i] - braslist[i];
		while (lp >= curlp) {
			if (advance(lp, ep))
				return(1);
			lp -= braelist[i] - braslist[i];
			}
		continue;

	case CDOT|STAR:
		curlp = lp;
		while (*lp++)
			;
		goto star;

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep)
			;
		ep++;
		goto star;

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)))
			;
		ep += *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp==locs)
				break;
			if (advance(lp, ep))
				return(1);
			} while (lp > curlp);
		return(0);

	default:
		errfunc("Unknown character in advance");
		}
	}

backref(i, lp)
	register i;
	register char *lp;
	{
	register char *bp;

	bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0);
	}

cclass(set, c, af)
	register char *set, c;
	{
	register n;

	if (c==0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
	}

int nbra;
compile(aeof)
	{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	if ((c = getchr()) == eof) {
		if (*ep==0)
			errfunc("No remembered expression");
		return;
		}
	circfl = 0;
	nbra = 0;
	if (c=='^') {
		c = getchr();
		circfl++;
		}
	peekc = c;
	lastep = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			goto cerror;
		c = getchr();
		if (c==eof) {
			if (bracketp != bracket)
				goto cerror;
			*ep++ = CEOF;
			return;
			}
		if (c!='*')
			lastep = ep;
		switch (c) {

		case '\\':
			if ((c = getchr())=='(') {
				if (nbra >= NBRA)
					goto cerror;
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
				}
			if (c == ')') {
				if (bracketp <= bracket)
					goto cerror;
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
				}
			if (c>='1' && c<'1'+NBRA) {
				*ep++ = CBACK;
				*ep++ = c-'1';
				continue;
				}
			*ep++ = CCHR;
			if (c=='\n')
				goto cerror;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':
			goto cerror;

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			if ((peekc=getchr()) != eof)
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchr()) == '^') {
				c = getchr();
				ep[-2] = NCCL;
				}
			do {
				if (c=='\n')
					goto cerror;
				if (c=='-' && ep[-1]!=0) {
					if ((c=getchr())==']') {
						*ep++ = '-';
						cclcnt++;
						break;
						}
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
						if (ep>=&expbuf[ESIZE])
							goto cerror;
						}
					}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					goto cerror;
				} while ((c = getchr()) != ']');
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
			}
		}
   cerror:
	expbuf[0] = 0;
	nbra = 0;
	errfunc("Regular expression compile error");
	}

compsub()
	{
	register seof, c;
	register char *p;

/*	if ((seof = getchr()) == '\n') */
	if ((seof = getchr()) == '\n' || seof == ' ')
		errfunc("Bad substitution format");
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchr();
		if (c=='\\')
			c = getchr() | 0200;
		if (c=='\n'){
			if(globp)	/* this came from v7 */
				c |= 0200;
			else
				/* to please rle...
				errfunc("Bad substitute format");
				 * substitute string can end with \n */
				{
				peekc = c;
				break;
				}
			}
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2])
			errfunc("Substitute buffer overflow");
		}
	*p++ = 0;
	if ((peekc = getchr()) == 'g') {
		peekc = 0;
		newline();
		return(1);
		}
	newline();
	return(0);
	}

dosub()
	{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++&0377) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
			}
		else if (c&0200 && (c &= 0177) >='1' && c < nbra+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
			}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE])
			errfunc("Substitute buffer overflow");
		}
	lp = loc2;
	loc2 = sp - genbuf + linebuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			errfunc("Substitute buffer overflow");
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++)
		;
	}

execute(wp, gf, addr)
	struct window *wp;
	int *addr;
	{
	register char *p1, *p2, c;

	for (c=0; c<NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
		}
	if (gf) {
		if (circfl)
			return(0);
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++)
			;
		locs = p1 = loc2;
		}
	else {
		if (addr==wp->wi_zero)
			return(0);
		p1 = getline(wp->wi_fileno, *addr);
		locs = 0;
		}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
		}
	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
				}
			} while (*p1++);
		return(0);
		}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
			}
		} while (*p1++);
	return(0);
	}

getsub()
	{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++)
		;
	linebp = 0;
	return(0);
	}

global(wp, k)
	struct window *wp;
	{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

	if (globp)
		errfunc("global command not permitted in global");
	setall(wp);
	nonzero();
	if ((c=getchr())=='\n')
		errfunc("Null line in global");
	compile(c);
	gp = globuf;
	while ((c = getchr()) != '\n') {
		if (c==EOF)
			errfunc("EOF in global");
		if (c=='\\') {
			c = getchr();
			if (c!='\n')
				*gp++ = '\\';
			}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2])
			errfunc("globuf overflow");
		}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=wp->wi_zero; a1<=wp->wi_dol; a1++) {
		*a1 &= ~01;
		if (a1>=addr1.ad_addr && a1<=addr2.ad_addr && execute(wp, 0, a1)==k)
			*a1 |= 01;
		}
	for (a1=wp->wi_zero; a1<=wp->wi_dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			wp->wi_dot = a1;
			globp = globuf;
#ifdef debugging
debug(57,"\007global: line=%d globuf=%s",a1-wp->wi_zero,globp);
#endif
			commands();
			a1 = wp->wi_zero;
			}
		}
	}

char*
place(sp, l1, l2)
	register char *sp, *l1, *l2;
	{

	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE])
			errfunc("Buffer overflow in place");
		}
	return(sp);
	}

/** the v7 changes having to do with marks have not been added */
substitute(wp,inglob)
	struct window *wp;
	{
	register gsubf, *a1, nl;
	int aold,lold;

	gsubf = compsub();
	linebp=0;
	for (a1 = addr1.ad_addr; a1 <= addr2.ad_addr; a1++) {
		int *ozero;
		if (execute(wp, 0, a1)==0)
			continue;
		for(aold=0, nl=0; nl<highmark; nl++)	/* look for mark */
			if(marks[nl].mk_window==wp
			&& marks[nl].mk_addr == (*a1 | 01)){
				aold=marks[lold=nl].mk_addr;
				break;
					}
		inglob |= 01;
		dosub();
		if (gsubf) {
			while (*loc2) {
				if (execute(wp, 1, (int*)0)==0)
					break;
				dosub();
				}
			}
		*a1 = putline(wp->wi_fileno);
		if(aold)	/* line was marked */
			marks[lold].mk_addr = *a1 | 01;
		if(replaceline(wp,a1)<0){	/* line was not on screen */
			pflag++;
			txflag++;
			}
		ozero=wp->wi_zero;
		nl = append(wp, getsub, a1);
#ifdef debugging
debug(91,"substitute: nl=%d",nl);
#endif
		if(nl>0){		/* line was split */
			pflag++;
			txflag++;
			}
		nl += wp->wi_zero-ozero;
		a1 += nl;
		addr2.ad_addr += nl;
		}
	if (inglob==0)
		errfunc("Search string not found");
	}

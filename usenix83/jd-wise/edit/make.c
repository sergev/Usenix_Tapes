#include "window.h"
#include "make.h"
#include "ed.h"
#include "edit.h"
#include "enscreen.h"

#define mlinesize 576
#define max(a,b) ((a)>(b)?(a):(b))

expandline(src,tgt)	/* expand tabs and count columns */
			/* consider listf */
	char *src, *tgt;
	{
	register c,col;

	col=0;

	if(listf)
		do{
			c = *src++ & 0377;
			if(c < ' ' && c != '\0'){
				tgt[col++] = '\\';
				tgt[col++] = (c>>3)+'0';
				tgt[col++] = (c&07)+'0';
				}
			else
				tgt[col++] = c;
			}
		while(c & 0177);
	else
		do{
			c = *src++ & 0377;
			tgt[col++] = c;
			if((c&0177) == '\t')
				while(col % tabspace)
					tgt[col++] = intab;
			}
		while(c & 0177);

	return(col-1);	/* don't count end of line null */
	}

installine(wp, lno, addr)
	register struct window *wp;
	int *addr;
	{
	int nl,n;
	register struct sline *mp = madeline;

	n=nl=makeline(wp,getline(wp->wi_fileno, *addr),(struct sline *)0);
	if(lno<wp->wi_ufirst && wp->wi_ufirst<=wp->wi_ulast){	/* insertion above */
		if((lno -= (nl-1)) < wp->wi_first)
			return(-1);
		}
	else{						/* insertion below */
		if(lno+nl > wp->wi_last+1)
			return(-1);
		}
	if(lno<wp->wi_ufirst)
		wp->wi_ufirst=lno;
	while(n--){
		mp->sl_addr=addr;	/* should this be in makeline? */
		mp->sl_staraddr = *addr & (~1);	/* and this too? */
		mp->sl_lno = (addr-wp->wi_zero)&077777;	/* or enterline ? */
		mp->sl_mark=' ';
		if(n==nl-1){	/* first line. look for mark */
			int l;
			for(l=0; l<highmark; l++){
				if(marks[l].mk_window == wp && marks[l].mk_addr==(*addr|01)){
					int c;
					for(c=0; c<26; c++)
						if(namet[c]==l){
							mp->sl_mark=c+'A';
							goto resume;
							}
					}
				}
			}
		resume:
		enterline(wp, lno++, mp++);
		}
	if(--lno > wp->wi_ulast)
		wp->wi_ulast=lno;
	return(nl);
	}

makeline(wp,src,sp)	/* convert text line to screen format */
			/* returns number of screen lines */
	register struct window *wp;
	register char *src;
	struct sline *sp;
	{
	int col;
	register struct sline *mp;
	char *tgt;
	char mline[mlinesize];
	int cct,nl;
	int pfirst,plast;
	int c,n;
	int m;
	int ltext;

	if((sp-screen)==wp->wi_statline)
		ltext=79;
	else
		ltext=wp->wi_ltext;
	cct=expandline(src,mline);
	nl=max(cct-1,1)/ltext+1;
#ifdef debugging
debug(24,"makeline: cct=%d nl=%d *src=%o *mline=%o",cct,nl,*src,*mline);
#endif
/*	if(!load) return(nl);*/

	mp = &madeline[0];
	mp->sl_flags = sl_written;
	mp->sl_first = 0;
	mp->sl_window = wp;
	strncpy(mp->sl_data,"        ",8);
	tgt=mp->sl_text;
	src=mline;
	pfirst=plast = -1;
	col=0;
	if(cct==0) goto eofl;	/* clean this up */
	while(cct--){
		c = *tgt++ = *src++;
		if(nonwhite(c)){
			plast=col;
			if(pfirst<0) pfirst=col;
			}
		if(++col >= ltext || cct == 0){
    eofl:
			if(cct==0){
				/** this test can be removed */
				if(*src & 0177)
					errfunc("non-null@eofl");
				*tgt = *src;	/* preserve mark */
				}
			else
				*tgt = '\0';
			mp->sl_last=col-1;
			mp->sl_pfirst=pfirst;
			mp->sl_plast=plast;
			if(cct != 0){	/* start a new line */
				mp->sl_flags |= sl_contd;
				mp++;
				mp->sl_flags = sl_written | sl_conting;
				mp->sl_first = 0;
				mp->sl_window = wp;
				strncpy(mp->sl_data,"        ",8);
				tgt=mp->sl_text;
				pfirst=plast = -1;
				col=0;
				}
			}
		}
	/* break to right */
#ifdef debugging
debug(24,"before break t[0]=%c t[71]=%c",mp->sl_text[0],mp->sl_text[ltext]);
#endif
	if((mp->sl_flags & sl_conting) && !leftbreak && (m=ltext-col)){
		/* move (possibly marked) end of line null */
		/** this could be put inside the loop for better efficiency */
		mp->sl_text[ltext]=mp->sl_text[col];
		for(tgt = &mp->sl_text[ltext-1],
		    src = &mp->sl_text[col-1];
		    col--; src--, tgt--){
			*tgt = *src;
			*src = '\0';	/* so line will look like screen */
			}
		mp->sl_first += m;
		mp->sl_last = ltext-1;
		mp->sl_pfirst += m;
		mp->sl_plast += m;
#ifdef debugging
debug(20,"m=%d first=%d last=%d pfirst=%d plast=%d",
m, mp->sl_first, mp->sl_last, mp->sl_pfirst, mp->sl_plast);
#endif
		/** this will fail if the line exactly fits
		 ** the test for entry to this section should be split */
		mp->sl_flags |= sl_rjust;
		}
	if(sp)	/* if originally a screen line, fix up */
		for(n=nl, mp=madeline; n--; mp++){
			if(mp==madeline)	/* first line */
				strncpy(mp->sl_data,sp->sl_data,8);
			mp->sl_flags |= sp->sl_flags&(sl_modified|sl_insertline);
			mp->sl_addr=sp->sl_addr;
			mp->sl_lno=sp->sl_lno;
			mp->sl_fileno=sp->sl_fileno;
			mp->sl_mark=sp->sl_mark;
			}
	return(nl);
	}

/* this and various other tests might well be table driven */
nonwhite(c)
	register c;
	{
	if(c==' ' || c=='\t' || c==intab || c=='\0')
		return(0);
	else
		return(1);
	}

replaceline(wp, addr)
	struct window *wp;
	int *addr;
	{
	int slno;

	if((slno=getslno(addr))<0)
		return(-1);
	updateline(slno,getline(wp->wi_fileno, *addr));
	return(0);
	}

/* this could probably be a bit cleaner */
unmakeline(slno)
	{
	register struct sline *sp;
	register char *src,*tgt;
	struct sline *sfirst;
	int col;

	sfirst = sp = &screen[slno];
	if(sp->sl_flags & sl_conting)	/* not first line */
		errfunc("Can't unmake");
	col = sp->sl_first;
	src = &sp->sl_text[col];
	tgt = linebuf;

	for(;;){
		if(col++ > sp->sl_last){
			if(sp->sl_flags&sl_contd){
				int flags;

				col = (++sp)->sl_first;
				src = &sp->sl_text[col];
				/* sl_modified must be propagated
				 * through updateline */
				if(flags=sp->sl_flags&(sl_modified|sl_insertline))
					sfirst->sl_flags |= flags;
				continue;
				}
			else
				break;
			}
		*tgt = *src++;
		if(*tgt != intab)
			if(++tgt >= &linebuf[LBSIZE]){
				errmsg("Line too long (truncated)");
				tgt--;
				break;
				}
		}
	*tgt = '\0';
	*tgt = *src;	/* this is necessary to preserve marks */
#ifdef debugging
debug(32,"unmake: return=%d",tgt-linebuf);
#endif
	return(tgt-linebuf);
	}

#ifdef mar31
updateline(lno,src)	/* cause screen and structure to agree with madeline */
			/* returns # of lines window is shifted up */
	char *src;
	{
	struct sline *sp, *mp;
	register struct window *wp;
	int nl, col;
	int *addr;
	int uplines=0;
	int c;

#ifdef debugging
debug(33,"updateline: lno=%d",lno);
#endif
	sp = &screen[lno];
	if(sp->sl_flags&sl_conting)
		errfunc("line %d not first in updateline",lno);
	addr=sp->sl_addr;
	wp = sp->sl_window;
	nl = makeline(wp,src,sp);
#ifdef debugging
debug(33,"updateline src length=%d nl=%d",strlen(src),nl);
#endif
	mp = madeline;
	for(; nl--; sp++, mp++, lno++){	/** this may be bugged */
		if(lno<wp->wi_last && sp->sl_addr!=addr){
#ifdef debugging
debug(53,"updateline lno=%d inline=%d defline=%d addr=0%o insertadr=0%o",
lno,inline,wp->wi_defline,addr,insertadr);
#endif
			if(addr==insertadr /**&& lno==inline*/
			   && lno>wp->wi_defline){
				openup(wp, --lno, 1);
				uplines++;	/*t for bug3 4-27-81 */
				inline--;	/** inline is a kluge */
#ifdef debugging
debug(53,"opening up uplines=%d",uplines);
#endif
				}
			else{
				opendown(wp, lno, 1);
#ifdef debugging
debug(53,"opening dwon uplines=%d",uplines);
#endif
				}
			}
		if(lno>wp->wi_last)
			lno--;
		if(lno>=wp->wi_last){
			openup(wp, lno, 1);
			uplines++;
			sp--;
			}
		/** lno may not be synced with sp here */
		updateone(wp,lno,mp);
		}
	while(lno<=wp->wi_last && sp->sl_addr==addr && sp->sl_flags!=sl_empty)
		closeup(wp, lno, 0);
#ifdef debugging
debug(33,"updatelines returning %d",uplines);
#endif
	return(uplines);
	}
#endif
updateline(lno,src)	/* cause screen and structure to agree with madeline */
			/* returns # of lines window is shifted up */
	char *src;
	{
	struct sline *sp, *mp;
	register struct window *wp;
	int nl, col;
	int *addr;
	int uplines=0;
	int c;

#ifdef debugging
debug(33,"updateline: lno=%d",lno);
#endif
	sp = &screen[lno];
	if(sp->sl_flags&sl_conting)
		errfunc("line %d not first in updateline",lno);
	addr=sp->sl_addr;
	wp = sp->sl_window;
	nl = makeline(wp,src,sp);
#ifdef debugging
debug(33,"updateline src length=%d nl=%d",strlen(src),nl);
#endif
	mp = madeline;
	for(; nl--; sp++, mp++, lno++){	/** this may be bugged */
		if(lno<=wp->wi_last && sp->sl_addr!=addr){
#ifdef debugging
debug(53,"updateline lno=%d inline=%d addr=0%o insertadr=0%o",
lno,inline,addr,insertadr);
#endif
			if(addr==insertadr /**&& lno==inline*/
			   && lno>wp->wi_defline){
				openup(wp, --lno, 1);
				uplines++;	/*t for bug3 4-27-81 */
				inline--;	/** inline is a kluge */
				}
			else
				opendown(wp, lno, 1);
			}
		if(lno>wp->wi_last){
			openup(wp, --lno, 1);
			uplines++;
			sp--;
			}
		/** lno may not be synced with sp here */
		updateone(wp,lno,mp);
		}
	while(lno<=wp->wi_last && sp->sl_addr==addr && sp->sl_flags!=sl_empty)
		closeup(wp, lno, 0);
#ifdef debugging
debug(33,"updatelines returning %d",uplines);
#endif
	return(uplines);
	}

updateone(wp,lno,mp)
	struct window *wp;
	register struct sline *mp;
	{
	register col;
	register struct sline *sp = &screen[lno];
	char c;
	int textcol;
	int pfirst;

#ifdef debugging
debug(50,"updateone: lno=%d",lno);
#endif
	if(lno==wp->wi_statline && wp->wi_type!=wi_comd)
		textcol=0;
	else
		textcol=wp->wi_textcol;
	/** the first test is responsible for the complete
	 ** rewriting of right broken lines */
	if(/*mp->sl_first != sp->sl_first ||*/ sp->sl_flags==sl_empty){
		enterline(wp, lno, mp);
		/** the structure assignment here is redundant */
		/** no more since *sp = *mp was moved out of for stmt */
		return;
		}
	pfirst=min(mp->sl_pfirst, sp->sl_pfirst);
#ifdef debugging
debug(50,"pfirst=%d mp.pfirst=%d sp.pfirst=%d",pfirst,mp->sl_pfirst,sp->sl_pfirst);
#endif
	for(col=pfirst; col<=mp->sl_plast; col++){
		if(col<0) continue;	/* empty line */
#ifdef debugging
if(sp->sl_flags&sl_conting)
debug(52,"col=%d sc=0%o mc=0%o",col,sp->sl_text[col],mp->sl_text[col]);
#endif
		if(col<mp->sl_pfirst){	/* shortened from left */
			topt(lno, col+textcol, (char*)0);
			echo(' ');
			continue;
			}
		if((c=mp->sl_text[col]) != sp->sl_text[col]
/** to fix the above problem 
**/
		  || col<sp->sl_first
		  || col>sp->sl_plast){
			topt(lno, col+textcol, (char*)0);
			echo(c);
			}
		}
	/* it is necessary that the cursor be left after the
	 * last character of the line because:
	 * 	otherwise the el() below will erase too much
	 *	inserting at eol resyncs with curcol
	 */
	topt(lno, col+textcol, (char*)0);
	if(sp->sl_plast > mp->sl_plast){
		el();
		}
	*sp = *mp;	/* now update the structure */
	}

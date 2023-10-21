/*
 * join and split
 */

#include "terminal.h"
#include "window.h"
#include "ed.h"
#include "edit.h"
#include "make.h"
#include "enscreen.h"
#include "file.h"

join()		/* join addressed lines into a single line */
	{
	register int *a1, *a2;
	char joinbuf[LBSIZE];
	char *tgt, *cp;
	struct window *wp;
	register struct sline *sp;
	int lin;

	a1=addr1.ad_addr;
	a2=addr2.ad_addr;
	if(a1==a2) return;	/* nothing to do */
	wp=addr1.ad_window;

#ifdef debugging
debug(54,"join: a1=0%o a2=0%o",a1,a2);
#endif
	/* form joined line */
	tgt=joinbuf;
	do{
		cp=getline(wp->wi_fileno,*a1++);
		if(jsflag && a1-1>addr1.ad_addr)
			while(iswhite(*cp))	/* remove leading white space */
				cp++;
		tgt=strcopy(cp,tgt,&joinbuf[LBSIZE]);
		if(jsflag && a1<=addr2.ad_addr)
			*tgt++ = ' ';
		} while(a1<=addr2.ad_addr);

	/* diddle addresses */
	a1=addr1.ad_addr;
	getonscreen(wp,a1);
	setused(wp);
	for(sp = &screen[lin=wp->wi_ufirst]; lin<=wp->wi_ulast; lin++, sp++){
		if(sp->sl_addr<=a1) continue;
		if(sp->sl_addr<=a2){
			sp->sl_addr=a1;
			sp->sl_flags |= sl_conting;	/* fake out showdata */
			showdata(lin);	/* erase old data field */
			}
		else{
			sp->sl_addr -= a2-a1;
			sp->sl_lno -= a2-a1;
			}
		}

	/* fixup the screen */
	updateline(lin=getslno(a1), joinbuf);
#ifdef debugging
debug(54,"after updateline lin=%d",lin);
#endif

	putsline(getslno(a1));

	/* delete lines from buffer */
	addr1.ad_addr++;
	delete(wp);

	/* tidy things up */
	wp->wi_dot=a1;
	expand(wp);
	updatedata(wp);
	}

#define better 1
#ifndef better
split(line,col)
	{
	register struct sline *sp;
	register char *src,*tgt;
	struct window *wp;
	int *addr;
	char *cp, *cp2;
	char savec;

	topt(line,col,(char*)0);
	sp = &screen[line];
	wp = sp->sl_window;
	col =- wp->wi_textcol;
	addr = sp->sl_addr;
	sp->sl_text[col] =| 0200;	/* mark char */
#ifdef debugging
debug(8,"split: line=%d col=%d",line,col);
#endif
	unmakeline(line=findfirst(line));
	for(cp=linebuf; *cp; *cp++)
		if(*cp&0200){
			savec = *cp&0177;
			cp2 = cp;
			*cp = '\0';
			break;
			}
	updateline(line,linebuf);
	*addr = putline(wp->wi_fileno);
	*linebuf=savec;
	for(cp=linebuf+1; *cp2; cp++, cp2++)
		*cp = *cp2;
	makeblankline(wp, addr++);
	updateline(getslno(addr),linebuf);
	*addr = putline(wp->wi_fileno);
	setat(wp);
	}
#endif
#ifdef better
split(line,col)
	{
	register struct sline *sp;
	struct window *wp;
	int *addr;
	register char *cp, *cp2;
	char savec;
	char splitline[512];
	int diff;

	topt(line,col,(char*)0);
	sp = &screen[line];
	wp = sp->sl_window;
	col -= wp->wi_textcol;
	addr = sp->sl_addr;
	sp->sl_text[col] |= 0200;	/* mark char */
#ifdef debugging
debug(8,"split: line=%d col=%d",line,col);
#endif
	unmakeline(line=findfirst(line));
	for(cp=linebuf, cp2=splitline; *cp; *cp++)
		if(*cp&0200){
			*cp2++ = *cp&0177;
			*cp++ = '\0';
			break;
			}
	while(*cp2++ = *cp++)
		;
	cp=splitline;
	if(jsflag)	/* delete leading white space */
		while(iswhite(*cp))
			cp++;
	updateline(line,linebuf);
#ifdef debugging
debug(8,"after first updateline addr=0%o",addr);
#endif
	screen[getslno(addr)].sl_flags |= sl_modified;
/*	*cp2=savec; ** i think this is left over from previous ver. */
	/* makeblankline may reassign addr */
	diff=addr-wp->wi_zero;
	makeblankline(wp, addr);
	addr=wp->wi_zero+diff+1;
#ifdef debugging
debug(8,"after makeblankline addr=0%o",addr);
#endif
	updateline(line=getslno(addr),cp);
#ifdef debugging
debug(8,"after second updateline line=%d",line);
#endif
	screen[getslno(addr)].sl_flags |= sl_modified;
	wp->wi_dot=addr;	/* just in case */
	setat(wp);
	}
#endif

char*
strcopy(from,to,limit)
	register char *from,*to,*limit;
	{

	while(*to++ = *from++)
		if(to > limit)
			errfunc("Assembled line too long");
	return(to-1);
	}

iswhite(c)
	register c;
	{
	return(c==' ' || c=='\t');
	}

/** this implementation is simple but wasteful of temp file space
 ** because each join creates a new tempfile entry.
 ** also it wiggles the screen more than necessary.
 ** a "better" approach would probably be very profligate of code, however.
 **/
justify()
	{
	register struct window *wp;
	register struct sline *sp;
	int n,lin;
	register col;

	wp=addr1.ad_window;
	wp->wi_dot=addr1.ad_addr;
	n=addr2.ad_addr-addr1.ad_addr;
	while(n--){
		addr1.ad_addr=wp->wi_dot;
		addr2.ad_addr=wp->wi_dot+1;
		join();
/* inadequate
		sp = &screen[lin=getslno(wp->wi_dot)];
		if(sp->sl_flags&sl_contd){
*/
		while(((sp = &screen[lin=getslno(wp->wi_dot)])->sl_flags&sl_contd)
		    ||sp->sl_last>wp->wi_rmarg){
#ifdef debugging
debug(97,"justify: lin=%d first=%d last=%d rmarg=%d",
lin,sp->sl_first,sp->sl_last,wp->wi_rmarg);
#endif
			for(col=min(sp->sl_last,wp->wi_rmarg); col>=sp->sl_first; col--){
				if(iswhite(sp->sl_text[col])){
#ifdef debugging
debug(97,"splitting at col=%d lmarg=%d",col,wp->wi_lmarg);
debug(98,"splitting: lmarg=%d",wp->wi_lmarg);
#endif
					split(lin,col+wp->wi_textcol);
					if(wp->wi_lmarg!=0){
#ifdef undef
						for(col=wp->wi_lmarg; col>=tabspace; col-=tabspace)
							insertchar(curline,curcol,'\t');
						for(; col>0; col--)
							insertchar(curline,curcol,' ');
#else
						lin=getslno(wp->wi_dot);
#ifdef debugging
debug(98,"inserting: lmarg=%d lin=%d",wp->wi_lmarg,lin);
#endif
						for(col=wp->wi_lmarg; col%tabspace; col--)
							insertchar(lin,wp->wi_textcol,' ');
						for(; col; col-=tabspace)
							insertchar(lin,wp->wi_textcol,'\t');
#endif
						}
					break;
					}
				if(col<=sp->sl_first)
					errfunc("Can't split line");
				}
			}
		syncscreen(wp);
		}
	}

center()
	{
	int *a;
	register struct window *wp;
	register n;
	int lin;
	char *cp,*tp;
	char centerbuf[LBSIZE];

	wp=addr1.ad_window;
	for(a=addr1.ad_addr; a<=addr2.ad_addr; a++){
		cp=getline(wp->wi_fileno,*a);
		while(iswhite(*cp))
			cp++;
		n=((wp->wi_rmarg-wp->wi_lmarg)-strlen(cp))/2;
		if(n<=0) continue;
		for(tp=centerbuf; n>=tabspace; n-=tabspace)
			*tp++ = '\t';
		for(; n; n--)
			*tp++ = ' ';
		strcopy(cp, tp, &centerbuf[LBSIZE]);
		updateline(lin=getonscreen(wp,a),centerbuf);
		putsline(lin);
		filedata[wp->wi_fileno].modflag++;
		}
	wp->wi_dot=addr2.ad_addr;
	}

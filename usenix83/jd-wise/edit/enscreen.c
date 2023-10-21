/*
 * per character screen management
 */

#include "terminal.h"
#include "window.h"
#include "ed.h"
#include "edit.h"
#include "make.h"
#include "enscreen.h"
#include "file.h"
#include "process.h"
#include "spell.h"

static int cmode;	/* this should be more carefully maintained */
static int ckspell=0;	/* allow spell checking in putsline */

enscreen(lin,col)
	{
	register c;
	char *cp;
	register struct sline *sp;
	register struct window *wp;
	int oldline,l,maxh;
	int retval=0;
	int *addr;
	int appending;

	sp = &screen[lin];
	wp = sp->sl_window;
	cp = sp->sl_text;
/*	if(cmode=(wp->wi_type == wi_comd))	*/
	if(cmode=(lin == cmdline))
		cmlptr = 0;
	owrite=owriteflag;
#ifdef debugging
debug(76,"enscreen: lin=%d col=%d cmode=%d",lin,col,cmode);
#endif
/*    startline: */
	topt(lin,col+wp->wi_textcol,(char*)0);
	maxh=curcol;
	if(updflag){
		updflag=0;
/*		sp->sl_flags |= sl_modified;	/* since it was deleted */
		goto caseel;
		}
	while(swptr){	/* restore remembered partial word */
			/* and perform auto-indent */
		c=saveword[--swptr];
		insertchar(curline,curcol,c);
		}
    loop:
	c=mapc();

	/* it's important to keep col syncd with curcol */
	/* and to keep sp syncd with curline */
	/* so: */
	col = curcol-wp->wi_textcol;
	sp = &screen[curline];
	cp = sp->sl_text;

	/* are we really in "insert" mode? */
	appending = !cmode && mode==insmode && insertadr != (int*)(-1)
		    && sp->sl_addr==insertadr && !(sp->sl_flags&sl_contd)
		    && col==sp->sl_last+1;
#ifdef debugging
debug(92,"appending=%d maxh=%d curcol=%d",appending,maxh,curcol);
#endif
	if(appending)	/* if so set maxh */
		maxh=curcol;

	/*
	 * erase character
	 */
	/** there's a bug here if the current position
	   is internal to a line. the code from modify
	   needs to be merged with this */
#ifdef eunice
	if((c == sgtty.sg_erase || c==('H'&037)) && !globp){
#else
	if(c == sgtty.sg_erase && !globp){
#endif
		if(allflag && (col>0 && cp[col-1] == '\\')){
			/* print escaped kill or erase char */
		    pec:
			backspace();
			echo(c);
			cp[col-1]=c;
			next;
			}
#ifdef debugging
debug(27,"found erase, curcol=%d textcol=%d",curcol,wp->wi_textcol);
#endif
		if(curcol>wp->wi_textcol){
			col--;	/* because curcol is reconstructed from col */
/*			curcol--;
*/			backspace();
			gointab(left);
			deletechar(curline,curcol);
#ifdef debugging
debug(27,"after deletechar curcol=%d",curcol);
#endif
			}
		next;
		}

	/*
	 * kill character
	 */
	/* this doesn't work yet */
	if(c == sgtty.sg_kill && !globp){
		if(allflag && (col>0 && cp[col-1] == '\\'))
			goto pec;
		/* goto startline; */
		if((cmode || !(sp->sl_flags&(sl_contd|sl_conting)))
		&& col>sp->sl_last){
			char *cp;
			for(cp=sp->sl_text; cp < &sp->sl_text[82]; cp++)
				*cp='\0';
			sp->sl_first=0;
			sp->sl_last=sp->sl_pfirst = -1;
			topt(lin,wp->wi_textcol,(char*)0);
			el();
			}
		next;
		}

	/*
	 * printing character
	 */
	if(c >= ' ' && c < '\177' || c == '\t'){
		/* this is now done by insertchar 
		if(!cmode) sp->sl_flags |= sl_modified; /*mmodflag++;*/
		if(!cmode && ucflag && c>='a' && c<='z')
			c += 'A'-'a';
	    pc1:
		insertchar(curline,curcol,c);
		col=curcol-wp->wi_textcol;	/* kludgy */
		if(wflag && sp->sl_addr==insertadr && !cmode
		&& (col >= wp->wi_ltext || col>wp->wi_rmarg) /** redundant */
		&& !(sp->sl_flags&sl_conting)){
			while((c=cp[--col])!=' ' && c!= '\t'){
				cp[col]='\0';
				backspace();
				if(c==intab) continue;
				saveword[swptr++]=c;
				}
			cp[col]='\0';	/* get the space */
			el();
			c='\n';
			sp->sl_last=sp->sl_plast=col-1;	/** ? */
			goto nl;
			}
		next;
		}

	/*
	 * newline
	 */
	if(c == '\n'){
	    nl:
#ifdef debugging
debug(29,"enscreen found newline col=%d cmode=%d",col,cmode);
#endif
/*		cp[col] = (cmode ? '\n' : '\0');
*//*		if(cmode) cp[col++]='\n'; cp[col]='\0';
		sp->sl_last=col-1;
*/		if(cmode){ cp[++sp->sl_last]='\n'; cp[sp->sl_last+1]='\0';}
		/** this is a sort of kludgy way of making
		 ** ret do the right thing */
#ifdef debugging
debug(68,"mode=%d insertadr=%o addr=%o flags=%o col=%d last=%d",
mode,insertadr,sp->sl_addr,sp->sl_flags,col,sp->sl_last);
#endif
		if(!cmode && mode==insmode && insertadr != (int*)(-1)
		&&(sp->sl_addr!=insertadr
		  || (sp->sl_flags&sl_contd)
		  || col!=sp->sl_last+1)){
			/* get insert line back on screen if necessary */
			lin=getonscreen(wp,insertadr);
			wp->wi_dot=insertadr;	/* in case dot was changed */
			setat(wp);
			topt(lin,curcol,(char*)0);
			goto caseel;
			}
		topt(lin,wp->wi_textcol,(char*)0);
		if(cmode)	/* return cursor to verify reciept of comd */
			psync();
		flush();
		col=0;
		retval=c;
		goto leave;
		}

	/*
	 * escape
	 */
	if(c == esc){
#ifdef debugging
debug(76,"c=escape in enscreen cmode=%d",cmode);
#endif
		if(cmode) return(esc);
/*    l1:*/
#ifdef apr11
		c=getesc();
#else
		c=mapc();
#endif
/*    l2:*/
		oldline=curline;
		switch(c){

		/*
		 * cursor motion
		 */
		case escup:
		case escdown:
		case escleft:
		case escright:
		case eschome:
		case escel:
		case esctab:
		case escbtab:
			setused(wp);
			switch(c){

			case eschome:
				if((l=getslno(wp->wi_dot)) >= 0)
					topt(l,wp->wi_textcol,(char*)0);
				else
					topt(wp->wi_defline,wp->wi_textcol,(char*)0);
				maxh=curcol;
				break;

/* these two probably need fixing */
			case escup:
				sp = &screen[curline-1];
				if(curline > max(wp->wi_first,wp->wi_ufirst)
				  && sp->sl_flags != sl_empty){
					topt(curline-1,min(maxh,sp->sl_last
								+wp->wi_textcol+((sp->sl_flags&sl_contd)?0:1)),(char*)0);
					}
				else{
#ifdef mar12
					retval=offthetop;
					goto leave;
#else
					int *addr;
					screen[curline].sl_text[curcol-wp->wi_textcol] |= 0200;
					addr=screen[curline].sl_addr;
#ifdef debugging
debug(56,"off the top addr=0%o",addr);
#endif
					globp="^";
					commands();
					tomark(getslno(addr));
#endif
					}
				lin=curline;
				break;

			case escdown:
				sp = &screen[curline+1];
				if(curline < min(wp->wi_last,wp->wi_ulast) && sp->sl_flags != sl_empty){
					topt(curline+1,min(maxh,sp->sl_last
								+wp->wi_textcol+((sp->sl_flags&sl_contd)?0:1)),(char*)0);
					}
				else{
#ifdef mar12
					retval=offthebottom;
					goto leave;
#else
					int *addr;
					screen[curline].sl_text[curcol-wp->wi_textcol] |= 0200;
					addr=screen[curline].sl_addr;
#ifdef debugging
debug(56,"off the bottom addr=0%o",addr);
#endif
					globp=".H";
					commands();
					tomark(getslno(addr));
#endif
					}
				lin=curline;
				break;

			/** screen[curline] should be replaced with */
			/** sp when it is kept current */
			/** escleft and escright should handle broken lines */
			case escleft:
				if(curcol > wp->wi_textcol+screen[curline].sl_first)
					topt(curline,curcol-1,(char*)0);
				maxh=curcol;
				break;

			case escright:
				if(curcol < (wp->wi_textcol+screen[curline].sl_last+1)){
					topt(curline,curcol+1,(char*)0);
					gointab(right);
					}
				maxh=curcol;
				break;

			case escel:
			caseel:
				for(lin=curline; (screen[lin].sl_flags&sl_contd); lin++)
					;
				col=screen[lin].sl_last+1;
				topt(lin,col+wp->wi_textcol,(char*)0);
				maxh=curcol;
				break;

			case esctab:	/* fix up */
				setused(wp);
				do{
/*
					if(col>sp->sl_last && !(sp->sl_flags&sl_contd)) break;
					if(++col>sp->sl_last){
						if(sp->sl_flags&sl_contd){
							lin++;
							sp++;
							col=sp->sl_first;
							}
						else
							col--;
						}
*/
					if(++col > sp->sl_last){
						if(lin < wp->wi_ulast){
							lin++;
							sp++;
							col=sp->sl_first;
							}
						else{
							col=sp->sl_last;
							break;
							}
						}
					} while(col%8 || c == intab);
				topt(lin,col+wp->wi_textcol,(char*)0);
				maxh=curcol;
				break;

			case escbtab:	/* fix up */
				setused(wp);
				do{
					if(--col < sp->sl_first){
						if(curline > wp->wi_ufirst){
							sp--;
							lin--;
							col=sp->sl_last;
							}
						else{
							col=sp->sl_first;
							break;
							}
						}
					} while(col%8);
				topt(lin,col+wp->wi_textcol,(char*)0);
				maxh=curcol;
				break;
				} /* end of cursor motion switch */
			/*
			 * cursor motion common code
			 */
			/* get out of intab */
			gointab(left);
			/* put out modified line */
			/** this test has been moved to leave
			if(!cmode && mmodflag && screen[curline].sl_addr != screen[oldline].sl_addr){
				putsline(getslno(screen[oldline].sl_addr));
				modflag++;
				mmodflag=0;
				}
			/*^*/
			next;
			/*
			 * end of cursor motion
			 */

		/*
		 * other escape sequences
		 */
		case escdc:		/* delete char */
			deletechar(curline,curcol);
			/* this is now done by deletechar
			sp->sl_flags |= sl_modified; /*mmodflag++;*/
			next;

		case escow:		/* set overwrite mode */
			owrite=1;
			next;

		case escic:
			owrite=0;	/* set insert mode */
			next;

		case escst:		/* split line */
			split(curline,curcol);
			topt(getonscreen(wp,wp->wi_dot),wp->wi_textcol,(char*)0);
			next;

		case escesc:
			retval=escesc;
			goto leave;

		/* insert/delete line */
		case escd:
			wp->wi_dot=sp->sl_addr;
			globp="d";
			saveloc();
			commands();
			restorloc();
			/* put cursor on dot */
			topt(getonscreen(wp,wp->wi_dot),curcol,(char*)0);
			gointab(right);
			next;

		case esca:
		case esci:
			/** this may as well reset dot and take up
			 ** insert mode at the new line */
			/** so: */
			setused(wp);
			for(lin=wp->wi_ufirst; lin<=wp->wi_ulast; lin++)
				screen[lin].sl_flags &= ~sl_insertline;
			addr=sp->sl_addr;
			if(addr>wp->wi_zero && c==esci)	/* insert */
				addr--;
wp->wi_dot=addr;	/** */
			makeblankline(wp,addr);	/* updates dot */
			topt(getslno(wp->wi_dot),wp->wi_textcol,(char*)0);
			sp = &screen[lin=curline];
			wp->wi_dot=insertadr=sp->sl_addr;
			sp->sl_flags |= sl_insertline;
			mode=insmode;
#ifdef debugging
debug(69,"esci: insertadr=%o",insertadr);
#endif
			setat(wp);
			col=0;
			next;

		case esch:
			esccom(sp,wp,'h');
			next;

		case eschat:	/* scroll down */
		case escminus:
			esccom(sp,wp,'^');
			next;

		case escj:
			esccom(sp,wp,'j');
			next;

		/* spell functions */
		case escaccept:
			spaccept();
			next;

		case 'S':	/** for the HP */
		case escsfind:
			spfind(wp);
			next;

		case escmatch:
			match(curline,curcol);
			next;

		/* formating functions */
		case 'l':
			wp->wi_lmarg=curcol-wp->wi_textcol;
			next;

		case 'r':
			wp->wi_rmarg=curcol-wp->wi_textcol;
			next;

		default:
			errmsg("unknown escape sequence: esc %c",c);
			next;
			} /* end of escape switch */
		} /* end of escape if */

	/*
	 * all other control characters
	 */
#ifdef eunice
	if(c==ctl(D) || c==ctl(Z)){
#else
	if(c==tchars.t_eofc || c==ctl(Z)){	/* ^D,^Z */	/** temp */
#endif
		retval=en_eof;
		goto leave;
		}
	if(allflag)
		goto pc1;
	else
		next;	/* ignore all other chars */

	/*
	 * put out modified line and return
	 */
    leave:
	if(!cmode){
		ckspell=1;
		for(l=wp->wi_first; l<=wp->wi_last; l++){
			if(screen[l].sl_flags&sl_modified){
				if(screen[l].sl_flags&sl_insertline
				   &&screen[l].sl_last==0
				   &&screen[l].sl_text[0]=='.'){
					/** this is getting klugy again */
					int lastline=0;
					closeup(wp,l,1);
					addr1.ad_addr=addr2.ad_addr=wp->wi_dot;	/** insertaddr? */
#ifdef debugging
debug(37,"before delete w=%d dot=%d",wp-window,wp->wi_dot-wp->wi_zero);
#endif
					if(wp->wi_dot==wp->wi_dol) lastline++;
					delete(wp);
#ifdef debugging
debug(37,"after delete w=%d dot=%d",wp-window,wp->wi_dot-wp->wi_zero);
#endif
					if(wp->wi_dot>0) wp->wi_dot--;
					if(lastline || (wp->wi_dot==wp->wi_zero && wp->wi_dol!=wp->wi_zero)) wp->wi_dot++;
					retval=en_eof;
					}
				else{
#ifdef debugging
debug(39,"leave: putting %d",l);
#endif
					putsline(findfirst(l));
					filedata[wp->wi_fileno].modflag++;
					}
				}
			}
		ckspell=0;
		}
#ifdef newer
	if(!cmode && mmodflag){
		/** test here for inserted line ="." */
		putsline(getslno(screen[curline].sl_addr));
		mmodflag=0;
		modflag++;
		}
#endif
	lastcol=curcol; lastline=curline;
/*	mode=oldmode;*/
	if(!cmode) showstat(wp);
#ifdef debugging
debug(37,"before return w=%d dot=%d",wp-window,wp->wi_dot-wp->wi_zero);
#endif
	return(retval);
	}


deletechar(line,col)	/* returns value of deleted char */
	{
	register struct sline *sp,*mp;
	struct window *wp;
	register c;
	char *src,*tgt;
	char retchar;
	int nl, lfirst;

	sp = &screen[line];
	wp = sp->sl_window;
	col -= wp->wi_textcol;
	retchar=sp->sl_text[col];
	if(col>sp->sl_last){
		return(0);	/* can't delete eol */
		}
#ifdef debugging
debug(26,"deletechar: line=%d col=%d retchar=0%o last=%d",
line,col,retchar,sp->sl_last);
#endif
	sp->sl_flags |= sl_modified;
	if(col==sp->sl_last	/* end of left-justified line */
	   && !(sp->sl_flags&sl_contd)
	   && (leftbreak || !(sp->sl_flags&sl_conting))){
		/** the saveloc/restorloc is not needed if
		 ** deletechar is always called with curlin,curcol */
		/** if this is the case, it could be enforced
		 ** by giving it no arguments */
		saveloc();
		topt(line,col+wp->wi_textcol,(char*)0);
		el();
		sp->sl_last--;
		sp->sl_text[col]='\0';
		restorloc();
		return(retchar);
		}
	sp->sl_text[col] = 0177;	/* mark deleted char */
	lfirst=findfirst(line);		/* find beginning */
nl=	unmakeline(lfirst);
	src=tgt=linebuf;
#ifdef debugging
debug(33,"deletechar: unmake returned %d strlen(linebuf)=%d",nl,strlen(linebuf));
#endif
	while((*tgt = *src++) & 0177)
		if(*tgt == 0177)
			*src |= 0200;	/* mark first changed char */
		else
			tgt++;		/* don't step for deltd char */
#ifdef debugging
debug(33,"after scan strlen(linebuf)=%d",strlen(linebuf));
#endif
	updateline(lfirst,linebuf);
	tomark(lfirst);
	if(!cmode)
		filedata[wp->wi_fileno].modflag++;
	return(retchar);
	}

/** this is full of anacronisms */
find(wp,fmode)
	struct window *wp;
	{
	int *a1;
	register c,l;
	register char *lp;
	struct sline *sp;

	compile('\n');
	a1=addr2.ad_addr;
	/*clearat(wp); now done by setat */
	for(;;){
		if(execute(wp,0,a1)) break;
		if(++a1 > wp->wi_dol) a1=wp->wi_zero;
		if(a1 == wp->wi_dot) errfunc("String not found");
		}
	wp->wi_dot=a1;
	if((l=getslno(wp->wi_dot))<0){
		shift(wp,wp->wi_defline,wp->wi_dot);
		l=wp->wi_defline;
		}
	setat(wp);
	lp=linebuf;
	sp = &screen[l];
	c=sp->sl_first;
	while(lp++<loc1){
	    l1:
		if(++c > sp->sl_last){
			l++;
			sp++;
			c=sp->sl_first;
			}
		if(screen[l].sl_text[c] == intab) goto l1;
		}
	if(fmode)
		enscreen(l,c);
	else
		topt(l,c+wp->wi_textcol,(char*)0);
	return;
	}

/** this needs fixing */
gointab(dir)	/* get out of intab */
	{
	int textcol;
	register lin,col;
	register struct sline *sp;

	lin=curline;
	sp = &screen[lin];
	textcol=(sp->sl_window)->wi_textcol;
	col=curcol-textcol;
#ifdef debugging
debug(27,"entering gointab: curcol=%d",curcol);
#endif
	if(col<sp->sl_first){
		col=sp->sl_first;
		dir=right;	/** ? */
		}
	if(col>sp->sl_last){
		if(sp->sl_flags&sl_contd || col!=sp->sl_last+1){
			col=sp->sl_last;
			dir=left;	/** ? */
			}
		}
	while(sp->sl_text[col] == intab){
		if(dir == left){
			if(col == sp->sl_first){
				lin--;
				sp--;
				col=sp->sl_last;
				}
			else{
				col--;
				}
			}
		else{	/* dir = right */
			if(col >= sp->sl_last){
				lin++;
				sp++;
				col=sp->sl_first;
				}
			else{
				col++;
				}
			}
		}
	/* lin = sp-screen; */
	topt(lin,col+textcol,(char*)0);
#ifdef debugging
debug(27,"after gointab curline=%d curcol=%d",curline,curcol);
#endif
	}

insertchar(line,col,ch)
	register col;
	char ch;
	{
	register struct sline *sp;
	register struct window *wp;
	int lfirst;
	char *src,*tgt;
	int cct,nl;

	/** quite a lot of code could be saved by setting cp = &sp->sl_text[col] */
#ifdef debugging
debug(30,"on entry line=%d col=%d",line,col);
#endif
	sp = &screen[line];
	wp = sp->sl_window;
	topt(line,col,(char*)0);
	col -= wp->wi_textcol;
	sp->sl_flags |= sl_modified;
	/*
	 * appending to end of first line
	 */
	if(col==sp->sl_last+1 && col<wp->wi_ltext && !(sp->sl_flags&sl_contd) && ch!='\t'){
		sp->sl_last++;	/* sp->sl_last = col; */
		if(nonwhite(ch)){
			sp->sl_plast = col;
			if(sp->sl_pfirst<0)
				sp->sl_pfirst=col;
			}
		sp->sl_text[col]=ch;
		sp->sl_text[col+1]='\0';
		echo(ch);	/* fixes curcol */
		return;
		}
	/*
	 * overwriting
	 */
	/** actually want to handle tabs here so they can overwrite
	 ** existing chars with intabs.
	 ** otherwise it will have to be pushed through all the way
	 ** to makeline.
	 ** the problem is what to do with a split line. **/
	if(owrite && col<wp->wi_ltext && ch!='\t'){
		if(sp->sl_text[col]=='\t'
		   && col+1<wp->wi_ltext
		   && sp->sl_text[col+1]==intab)
			sp->sl_text[col+1]='\t';
		if(nonwhite(ch) && col>sp->sl_plast)
			sp->sl_plast = col;
		sp->sl_text[col]=ch;
		echo(ch);
		return;
		}
	/*
	 * general case
	 */
	/** this won't work inserting at ltext */
#ifdef debugging
debug(63,"general insertchar line=%d col=%d ch=0%o",line,col,ch);
#endif
	if(col >= wp->wi_ltext && (sp->sl_flags & sl_contd))
/*		(sp+1)->sl_text[0] |= 0200;	/* mark next char */
/*		screen[line+1].sl_text[0] |= 0200;	/* mark next char */
		(sp+1)->sl_text[(sp+1)->sl_first] |= 0200; /* mark next char */
	else{
		sp->sl_text[col] |= 0200;	/* mark adjacent char */
		if(col>=wp->wi_ltext)
			sp->sl_last=col;	/* so unmake will work */
		}
	lfirst=findfirst(line);	/* find beginning */
	cct=unmakeline(lfirst);
#ifdef debugging
debug(63,"cct=%d",cct);
#endif
	src=linebuf;
	tgt=genbuf;
	do{
#ifdef debugging
debug(63,"*src=0%o",*src);
#endif
		if(*src & 0200){
			*tgt++ = ch /* |0200 */;
			if(owrite){
				if(*src&0177) src++;	/* don't overwrite eol */
/** try putting this back */
			/* */	*src |= 0200;	/* fake an insertion */
				}
			}
		*tgt = *src++ /**& 0177 */;	/* leave the mark on adjacent char */
		} while(*tgt++ & 0177);
	line -= updateline(lfirst, genbuf);
	tomark(line);
	gointab(right);
	if(!cmode)
		filedata[wp->wi_fileno].modflag++;
	return;
	}

/* this routine still has some anacronisms in it */
putsline(slno)
	{
	register struct sline *sp;
	register int *addr;
int u;

	sp = &screen[slno];
u=	unmakeline(slno);
	*(addr=sp->sl_addr)=putline(sp->sl_window->wi_fileno);
	for(; sp->sl_addr == addr; sp++)
		sp->sl_flags &= ~(sl_modified|sl_insertline);
#ifdef debugging
debug(38,"putsline: slno=%d u=%d addr=0%o *addr=0%o",slno,u,addr,*addr);
#endif
	/** this is here temporarilly because it's the easiest
	 ** way to get it installed */
	if(spellpid && ckspell)
		sendspell(linebuf);
	}

tomark(slno)	/* goto and clear marked character */
	{
	register struct sline *sp;
	register struct window *wp;
	register col;

	sp = &screen[slno];
	wp = sp->sl_window;
#ifdef debugging
debug(41,"tomark: slno=%d",slno);
#endif
   loop:
	/* sl_last+1 in case mark is at the end of a full line */
	for(col=sp->sl_first; col<=sp->sl_last+1; col++){
		if(sp->sl_text[col] & 0200){
#ifdef debugging
debug(41,"tomark: line=%d col=%d char=0%o",slno,col,sp->sl_text[col]&0377);
#endif
			sp->sl_text[col] &= 0177;
			topt(slno, col+wp->wi_textcol, (char*)0);
			return(col);
			}
		}
	if(sp->sl_flags & sl_contd){
		sp++;
		slno++;
		goto loop;
		}
	return(-1);
	}

esccom(sp,wp,c)	/* execute command mode function from cursor mode */
	register struct sline *sp;
	register struct window *wp;
	char c;
	{
	static char cstring[]=" ";

	syncscreen(wp);
	wp->wi_dot=sp->sl_addr;
	cstring[0]=c;
	globp=cstring;
	saveloc();
	commands();
	restorloc();
	/* put cursor on dot */
	topt(getonscreen(wp,wp->wi_dot),curcol,(char*)0);
	gointab(right);
	}

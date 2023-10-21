#include "window.h"
#include "make.h"
#include "terminal.h"
#include <stdio.h>
#include "ed.h"
#include "edit.h"
#include "file.h"

#define creep 1	/* too much like a yo-yo otherwise */

/*
 * package initialization
 */

initscr()
	{
#ifdef old
	static char outbuf[BUFSIZ];
#else
	extern char _sobuf[BUFSIZ];
#	define outbuf _sobuf
#endif

	setbuf(stdout,outbuf);
	getterm();
	return(0);	/* should test for bad terminal type & return -1 */
	}

/*
 * routines for creating, modifying, and deleting windows
 */

static struct window *picture[maxwindows];
static char ltext[maxwindows]	={72, 72, 72, 72, 79, 76};
static char textcol[maxwindows]	={ 7,  7,  7,  7,  0,  3};

struct window *
addwindow(n,type,size)	/* size is total size (including statline) */
	{
	register struct window *wp;
	struct window	*w0=(struct window *)0,	/* existing windows */
			*w2=(struct window *)0;
	register i;
	int start,stop;
	int	start0=0, stop0=nlines,	/* new boundaries for existing windows */
		start2=0, stop2=nlines;
		

	wp = &window[n];
	if(wp->wi_flags&wi_active)
		errfunc("Window %d already active",n);
	if(type==wi_comd){
		start=cmdline;
		size=1;
		}
	else{
		/*
		 * this code is dependent on the limited number
		 * of cases possible with a maximum of 3 windows
		 *
		 * a more regular algorithm would be:
		 *	for(i=firstwindow; i<=lastwindow; i++){
		 *		if(i<new){
		 *			changeup(i);
		 *			newstart=window[i].wi_statline+1;
		 *			}
		 *		}
		 *	for(i=lastwindow; i>=firstwindow; i--){
		 *		if(i>new){
		 *			changedown(i);
		 *			}
		 *		}
		 */
		switch(nwindows++){
		case 0:			/* screen was empty */
			start=0;
			picture[0]=wp;
			break;
		case 1:
			if((w2=screen[0].sl_window)->wi_index>n){	/* above */
				start=0;
				start2=size;
				changedown(w2,start2,nlines);
				picture[1]=picture[0];
				picture[0]=wp;
				}
			else{		/* below */
				w0=w2;
				w2=(struct window *)0;
				start=nlines-size+1;	/** should be parameterized */
				stop0=start-1;
				changeup(w0,0,stop0);
				picture[1]=wp;
				}
			break;
		case 2:
			w0=screen[0].sl_window;
			w2=screen[nlines].sl_window;
			if(w0->wi_index>n && w2->wi_index>n){	/* above */
				start=0;
				changedown(w2,(nlines+size)/2,nlines);
				changedown(w0,start+size,(nlines+size)/2-1);
				picture[2]=picture[1];
				picture[1]=picture[0];
				picture[0]=wp;
				}
			else if(w0->wi_index<n && w2->wi_index<n){	/* below */
				start=nlines-size+1;
				changeup(w0,0,(nlines-size)/2-1);
				changeup(w2,(nlines-size)/2,start-1);
				picture[2]=wp;
				}
			else{		/* between */
				start = (nlines-size)/2;
				changeup(w0,0,start-1);
				changedown(w2,start+size,nlines);
				picture[2]=picture[1];
				picture[1]=wp;
				}
			break;
		default:		/* 3 or more */
			nwindows--;
			errfunc("Too many windows");
			}
		}
	if(type!=wi_comd)
		size--;
#ifdef debugging
debug(47,"addwindow: n=%d size=%d start=%d",n,size,start);
#endif
	stop=start+size-1;
	wp->wi_type=	type;
	wp->wi_first=	start;
	wp->wi_last=	stop;
	wp->wi_size=	size;
	wp->wi_statline=stop+(type==wi_comd?0:1);
	wp->wi_defline=	start+(2*size)/3;
	wp->wi_flags=	wi_active;
	wp->wi_curx=	wp->wi_cury=0;
	wp->wi_index=	n;
/*	wp->wi_ltext=	72;	/* temp */
	wp->wi_ltext=	ltext[n];
	wp->wi_lmarg=	0;
	wp->wi_rmarg=	ltext[n]-1;
/*	wp->wi_textcol=	(type==wi_comd)?3:7;*/
	wp->wi_textcol=	textcol[n];
	wp->wi_ufirst=	stop+1;
	wp->wi_ulast=	-1;
	for(i=start; i<=wp->wi_statline; i++){
		clearsline(i);
		screen[i].sl_window = wp;
		}
	return(wp);
	}

clearwindow(wp)
	register struct window *wp;
	{
	register i;

	syncscreen(wp);
	for(i=wp->wi_first; i<=wp->wi_last; i++)
		/** this test should be pushed through to clearsline */
		if(screen[i].sl_flags != sl_empty)
			clearsline(i);
	wp->wi_ufirst=wp->wi_last+1;
	wp->wi_ulast = -1;
	}

changedown(wp,newstart,newstop)
	register struct window *wp;
	{
	int l;
	int oldstart,oldstop;
	int newsize;
	int newdefline;

	oldstart=wp->wi_first;
	oldstop=wp->wi_statline;
	newsize=newstop-newstart;
	newdefline=(2*newsize)/3+newstart;
	for(l=oldstop; l<=newstop; l++)
		clearsline(l);
	wp->wi_last=newstop-1;
	wp->wi_statline=newstop;
	if(wp->wi_dot!=wp->wi_zero)
		shift(wp,newdefline,wp->wi_dot);
	for(l=oldstart; l<newstart; l++)
		clearsline(l);
	wp->wi_first=newstart;
	wp->wi_size=newsize;
	wp->wi_defline=newdefline;
	for(l=newstart; l<=newstop; l++)
		screen[l].sl_window=wp;
	for(l=wp->wi_first; screen[l].sl_flags&sl_conting; l++)
		clearsline(l);
	setused(wp);
	if(wp->wi_type!=wi_comd)
		showstat(wp);
	}

changeup(wp,newstart,newstop)
	register struct window *wp;
	{
	int l;
	int oldstart,oldstop;
	int newsize;
	int newdefline;

	oldstart=wp->wi_first;
	oldstop=wp->wi_statline;
	newsize=newstop-newstart;
	newdefline=(2*newsize)/3+newstart;
	for(l=newstart; l<oldstart; l++)
		clearsline(l);

	wp->wi_first=	newstart;
	if(wp->wi_dot!=wp->wi_zero)
		shift(wp,newdefline,wp->wi_dot);

	for(l=newstop; l<=oldstop; l++)
		clearsline(l);
	wp->wi_last=	newstop-1;
	wp->wi_size=	newsize;
	wp->wi_statline=newstop;
	wp->wi_defline=	newdefline;
	for(l=newstart; l<=newstop; l++)
		screen[l].sl_window=wp;
	for(l=wp->wi_last; (screen[l].sl_flags&sl_contd); l--)	/* delete partial lines */
		clearsline(l);
	setused(wp);
	if(wp->wi_type!=wi_comd)
		showstat(wp);
	}

deletewindow(wp)
	register struct window *wp;
	{
	register i;
	register boundary;
	int slot;

	for(slot=0; slot<maxwindows; slot++)
		if(picture[slot]==wp) break;

	for(i=wp->wi_first; i<=wp->wi_statline; i++){
#ifdef moved
		if(i==wp->wi_statline){
			deleteline(i);
			insertline(i);
			}
#endif
		clearsline(i);
		screen[i].sl_window = (struct window *) 0;
		}
	wp->wi_flags=0;
	/*
	 * this code is dependent on the special cases
	 */
	switch(nwindows--){
	case 1:
		errfunc("Deleted last window");
	case 2:
		switch(slot){
		case 0:
			changeup(picture[1],0,nlines);
			picture[0]=picture[1];
			break;
		case 1:
			changedown(picture[0],0,nlines);
			break;
			}
		picture[1]=(struct window *)0;
		expand(picture[0]);
		break;
	case 3:
		if(picture[0] == &window[shellwindow] && slot!=0)
			boundary=6;
		else
			boundary=10;
		switch(slot){
		case 0:
			changeup(picture[1],0,boundary);
			changeup(picture[2],boundary+1,nlines);
			picture[0]=picture[1];
			picture[1]=picture[2];
			break;
		case 1:
			changedown(picture[0],0,boundary);
			changeup(picture[2],boundary+1,nlines);
			picture[1]=picture[2];
			break;
		case 2:
			changedown(picture[1],boundary+1,nlines);
			changedown(picture[0],0,boundary);
			break;
			}
		picture[2]=(struct window *)0;
		expand(picture[0]);
		expand(picture[1]);
		break;
	default:
		errfunc("case not implemented in deletewindow");
		}
	}

/*
 * routines for working with windows
 */
clearline(wp,lno,cnt)
	register struct window *wp;
	{
	register i,j;

	if((j=lno+(cnt-1)) > wp->wi_last)
		return(bad);
	for(i=lno; i<=j; i++){
		clearstruct(i);
		eraseline(i);
		}
	return(good);
	}

closeup(wp,lno,update)
	register struct window *wp;
	{
	register l,first;
	int cnt=0;	/* temp */

	if(lno+(cnt-1)>wp->wi_last)
		return(bad);
	deleteline(lno);
	first = !(screen[lno].sl_flags & sl_conting);
	for(l=lno; l<wp->wi_last; l++){
		screen[l]=screen[l+1];
		if(first && update){
			screen[l].sl_lno--;
			screen[l].sl_addr--;
			showdata(l);
			}
		}
	clearstruct(wp->wi_last);
	wp->wi_ulast--;
	saveloc();
#ifdef debugging
debug(43,"closeup: before insertline");
#endif
	insertline(wp->wi_statline-1);
#ifdef debugging
debug(43,"closeup: after insertline");
#endif
	restorloc();
	}

opendown(wp,lno,cnt)
	register struct window *wp;
	{
	register i;

if(lno<0 || cnt>50) abort();
	if(lno+(cnt-1)>wp->wi_last)
		return(bad);

	/* test for modified lines leaving screen */
	for(i=findfirst(wp->wi_last-cnt+1); i<=wp->wi_last; i++){
		if(i<0) break;
		if(screen[i].sl_flags&sl_modified)
			putsline(findfirst(i));
			}

#ifdef creep
	for(i=0; i<cnt; i++){
		deleteline(wp->wi_last);
		insertline(lno);
		}
#else
	for(i=0; i<cnt; i++)
		deleteline(wp->wi_last-cnt+1);	/* delete last lines */
	for(i=0; i<cnt; i++)
		insertline(lno);		/* open up screen */
#endif

	for(i=wp->wi_last; i>lno+cnt-1; i--)	/* move screen lines down */
		screen[i]=screen[i-cnt];
	for(i=0; i<cnt; i++)
		clearstruct(lno+i);

	/* delete partial lines */
	for(i=wp->wi_last; (screen[i].sl_flags & sl_contd); i--){
#ifdef debugging
debug(45,"before clearsline i=%d",i);
#endif
		clearsline(i);
		}

	return(good);
	}

openup(wp,lno,cnt)
	register struct window *wp;
	{
	register i;

if(lno<0 || cnt>50) abort();
	if(lno+(cnt-1)>wp->wi_last)
		return(bad);

	/* test for modified lines leaving screen */
	for(i=wp->wi_first; i<wp->wi_first+cnt; i++){
		if(screen[i].sl_flags==sl_empty) continue;
		if(screen[i].sl_flags&sl_modified)
			putsline(findfirst(i));
			}

#ifdef creep
	for(i=0; i<cnt; i++){
		deleteline(wp->wi_first);
		insertline(lno+cnt-1);
		}
#else
	for(i=0; i<cnt; i++)		/* delete first lines */
		deleteline(wp->wi_first);
	for(i=0; i<cnt; i++)		/* open up screen */
		insertline(lno);
#endif
	/** there is an inconsistency here between openup and down
	 ** it is imbedded in shift
	 */
	for(i=wp->wi_first; i <= lno-/*cnt*/1; i++)	/* move screen lines up */
		screen[i]=screen[i+cnt];
	for(i=0; i<cnt; i++)
		clearstruct(lno+i);
	/* delete partial lines */
	for(i=wp->wi_first; screen[i].sl_flags & sl_conting; i++)
		clearsline(i);
	return(good);
	}

enterline(wp,lno,line)
	register struct window *wp;
	struct sline *line;
	{

	if(lno>wp->wi_statline)	/* was wi_last */
		return(bad);
	screen[lno] = *line;
/*	eraseline(lno);	showline does this */
	showline(lno);
	return(good);
	}

/*
 * utility routines 
 */

clearsline(lno)
	{
	/** this is an indirect way of dealing with
	 ** protected fields and could be cleaned up */
	if(lno == screen[lno].sl_window->wi_statline){	/* protected */
		deleteline(lno);
		insertline(lno);
		}
	else
		eraseline(lno);
	clearstruct(lno);
	}

clearstruct(lno)
	{
	register char *cp;
	register struct sline *sp;

	sp = &screen[lno];
	for(cp = sp->sl_text; cp < &sp->sl_text[82]; cp++)
		*cp = '\0';
	for(cp = sp->sl_data; cp < &sp->sl_data[8]; cp++)
		*cp = ' ';	/** was '\0' */
	sp->sl_addr=(int*)0;
	sp->sl_tl = -1;
	sp->sl_lno=0;
	sp->sl_first=0;
	sp->sl_last=sp->sl_pfirst = -1;
	sp->sl_flags=sl_empty;
	/** these two should be replaced by sl_data[ed_markcol]
	 ** and sl_data[ed_atcol] */
	/** so let's try it
	 ** taking this out seems to cause duplication of marks **/
	sp->sl_mark=' ';
	sp->sl_at=' ';
	/** so
	 **/
	}

showdata(lineno)	/* display data portion of line */
	{
	char data[8];
	register col,i;
	int r;
	register struct sline *sp;
	struct window *wp;
	int cmode;	/* temp */

	cmode=(lineno==cmdline);	/* temp */
	sp = &screen[lineno];
	wp = sp->sl_window;
	if(lineno==wp->wi_statline)
		return(1);
	cmode=(wp->wi_type==wi_comd);	/* better */
	if(sp->sl_flags == sl_empty)
		return(0);

	if(cmode){
		topt(lineno,0,prompt);	/* should use wi_prompt */
		return(1);
		}
	for(col=0; col<wp->wi_textcol; col++)
		data[col]=' ';
	data[wp->wi_textcol]='\0';
	if((sp->sl_flags & sl_conting) || helping)
		goto update;	/* clear if necessary */
	/** this test is redundant */
	if((sp->sl_flags & sl_conting) == 0){
		i = sp->sl_lno;
		col = ed_lnocol;
		do{
			r=i%10;
			i /= 10;
			data[col--] = r + '0';
			}
		while(i);
		data[ed_markcol] = sp->sl_mark;
		if(sp->sl_addr == wp->wi_dol)
			data[ed_atcol]=sp->sl_at='$';
		if(sp->sl_addr == wp->wi_dot)
			data[ed_atcol]=sp->sl_at='.';
		}

	/* only write as much of the data field as necessary */
   update:
	for(col=0; col<wp->wi_textcol; col++)
		if(data[col]!=sp->sl_data[col]){
		topt(lineno,col,&data[col]);
		break;
		}
	strncpy(sp->sl_data,data,8);
	return(1);
	}

showline(lineno)	/* display line on screen */
	{
	register struct sline *sp;
	struct window *wp;
	int icol;
	register c;
	register char *op;
	char oline[256];
	int tabsok;

#ifdef debugging
debug(21,"showline: lineno=%d",lineno);
#endif
	/* showdata now manages clearing of the data field
	eraseline(lineno);
	*/
	sp = &screen[lineno];
	wp = sp->sl_window;
	if(!showdata(lineno)) return(0);
	topt(lineno,wp->wi_textcol,(char*)0);
	el();
	tabsok = hastabs && !(sp->sl_flags&sl_conting);
	if(sp->sl_pfirst<0)	/* empty line */
		return(1);
	for(icol=sp->sl_pfirst, op=oline; icol<=sp->sl_plast; icol++){
		c=sp->sl_text[icol]&0177;
		if(c==0){
			*op=c;
			break;
			}
		else if(c == '\t')
			*op++ = tabsok?'\t':' ';
		else if(c==intab){
			if(!tabsok) *op++=' ';
			}
		else if(c<' ' || c==0177){	/* handle control char */
			char *cp;
			for(cp=showctl(c); *cp; cp++)
				*op++ = *cp;
			}
		else
			*op++ = c;
		}
	*op='\0';

	topt(lineno,sp->sl_pfirst+((lineno==wp->wi_statline)?0:wp->wi_textcol),oline);
	return(1);
	}

syncscreen(wp)	/* force tempfile to agree with screen */
	register struct window *wp;
	{
	register l;
	register struct sline *sp;

	setused(wp);
	for(sp = &screen[l=wp->wi_first]; l<=wp->wi_last; l++, sp++){
		if(sp->sl_flags&sl_modified){
			putsline(findfirst(l));
			filedata[wp->wi_fileno].modflag++;
			}
		}
	}

wname(n)	/* return window name given number */
	{
	if(n>=0 && n<=4)
#ifdef eunice
		return("SABCD"[n]);
#else
		return("@ABCD"[n]);
#endif
	else
		return('#');
	}

wnumber(c)	/* return window number given name */
	{
	if(c>='a' && c<='d' || c>='@' && c<='D')
		return((c&(~' '))-'@');
	else
		return(-1);
	}

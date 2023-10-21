#include "conf.h"
#include "window.h"
#include "terminal.h"
#include "edit.h"
#include "ed.h"
#include "make.h"
#include "file.h"


showstat(wp)
	register struct window *wp;
	{
	char	line[128];
	int	fn=wp->wi_fileno;
	int	lno=wp->wi_statline;
	int	col;

	if(dbflag&&dbflag!=34&&dbflag!=43&&dbflag!=50&&dbflag!=15) return;

	saveloc();
	setbgnd();
	for(col=0; col<128; col++)
		line[col]=' ';
	if(fn==helpwindow)
		sprintf(line,"HELP^^^^");
	else
		sprintf(line,"File%c: %-28.28s .=%-4d $=%-4d Page=%-4d",
			wname(fn),
			(fn==0)?"SHELL":filedata[fn].savedfile,
			(wp->wi_dot-wp->wi_zero)&077777,
			(wp->wi_dol-wp->wi_zero)&077777,
			((((wp->wi_dot-wp->wi_zero)&077777)-1)/56)+1
			);
	/** temp parameters*/
#ifdef eunice
	for(col=7; col<78; col++)
#else
	for(col=7; col<79; col++)
#endif
		if(line[col]==' ' || line[col]=='\0')
			line[col]='^';
	line[col]='\0';
	if(makeline(wp,line,&screen[lno])>1)
		errfunc("statline too long");	/** can't happen */
	updateone(wp, lno, madeline);
	set4gnd();
	restorloc();
#ifdef debugging
debug(34,"wp=0%o",wp);
if(dbflag==34&&(wp-window)>2)abort();
#endif
	}

static int dotline = -1;
static int dolline = -1;
setat(wp)
	register struct window *wp;
	{
	register l;

	if(wp == &window[helpwindow]) return;
	saveloc();
	if((l=getslno(wp->wi_dol))>=0){
#ifdef debugging
debug(51,"setat: dol=%d l=%d",wp->wi_dol-wp->wi_zero,l);
#endif
		showdata(l);
		dolline=l;
		}
	if((l=getslno(wp->wi_dot))>=0){
#ifdef debugging
debug(51,"setat: dot=%d l=%d",wp->wi_dot-wp->wi_zero,l);
#endif
		showdata(l);
		dotline=l;
		}
	clearat(wp);
	dotline=dolline = -1;
	restorloc();
	}

clearat(wp)
	register struct window *wp;
	{
	register lin;

	for(lin=wp->wi_first; lin<=wp->wi_last; lin++){
		if(lin==dotline || lin==dolline) continue;
		/** sl_at is being eliminated
		if(screen[lin].sl_at!=' '){
		 **/
		if(screen[lin].sl_data[ed_atcol] != ' '){
			saveloc();
			topt(lin,ed_atcol," ");
			screen[lin].sl_data[ed_atcol]=' ';
			restorloc();
			screen[lin].sl_at=' ';
			}
		}
	}

updatedata(wp)
	register struct window *wp;
	{
	register l;

	for(l=wp->wi_first; l<=wp->wi_last; l++)
		showdata(l);
	}

#ifdef vax
int *
finmon(){}
#endif
#ifdef eunice
profil(){}
#endif

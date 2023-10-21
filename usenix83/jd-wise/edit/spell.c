#include <sys/param.h>
#include "ed.h"
#include "window.h"
#include "edit.h"
#include "process.h"
#include "spell.h"

char*	spprog="/usr/lib/edit/spell";

/*
 * send a line to the spell checker
 */
sendspell(line)
	char *line;
	{
	if(spellpid==0)
		errfunc("Spell checker not active");

	send(&spellproc,line);
	}

/*
 * open pipes and fork background process
 */
makespell()
	{
#ifdef nospell
	errfunc("Spell checking not implemented");
	return;
#endif
	makeproc(&spellproc,spprog);
#ifdef SIG_HOLD
	signal(SIGEMT,SIG_HOLD);
#else
	signal(SIGEMT,SIG_IGN);
#endif
	}

killspell()
	{
#ifdef nospell
	errfunc("Spell checking not implemented");
	return;
#endif
	killproc(&spellproc);
	}

getbadword()
	{
	register c;
	register char *cp;

	for(cp=badword; (c=getc(fmspell))!=0177; cp++){
		if(c=='\n') break;
		*cp=c;
		}
	if(cp==badword) return;	/* pipe was empty */
	*cp='\0';
	set4gnd();	/* in case we were writing statline */
	errmsg("Spelling error: %s",badword);
	}

spaccept()
	{
	if(spellpid==0) return;
	fprintf(tospell,"\001%s\n",badword);
	fflush(tospell);
	getbadword();
	}

spfind(wp)
	register struct window *wp;
	{
	int *a1;
	int c,l;
	char *lp;
	register struct sline *sp;

	if(spellpid==0) return;
	globp=badword;
	compile(EOF);
	getbadword();
	for(a1=wp->wi_dot-1;;a1--){
		if(a1<wp->wi_zero)
			a1=wp->wi_dol;
		if(execute(wp,0,a1)) break;
		if(a1==wp->wi_dot)
			errfunc("Misspelled word not found");
		}
	if((l=getslno(a1))<0)
		errfunc("Misspelled word not on screen");
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
	topt(l,c+wp->wi_textcol,(char*)0);
	return;
	}

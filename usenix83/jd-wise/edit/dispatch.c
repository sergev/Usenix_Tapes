/*
 * mode switcher
 */

#include "ed.h"
#include "terminal.h"
#include "window.h"
#include "edit.h"
#include "enscreen.h"

dispatch()
	{
	int lin;
	int autoi;
	register struct sline *sp;

	for(;;){
		insertadr = (int*)(-1);
		setat(wc);
		switch(commands()){

		case co_inserting:
			mode=insmode;
		    nextinsert:
			/* do auto-indent */
			autoi=0;
			if(wc->wi_lmarg!=0){
				register n=wc->wi_lmarg;
				autoi=n;
				while(n%tabspace){
					saveword[swptr++]=' ';
					n--;
					}
				while(n){
					saveword[swptr++]='\t';
					n -= tabspace;
					}
				}
			else if(indflag && wc->wi_dot>wc->wi_zero){
				register char *cp;
				/* look at previous line */
				cp=getline(wc->wi_fileno,*wc->wi_dot);
				while(*cp==' ' || *cp=='\t')
					cp++;
				autoi=cp-linebuf;
				while(--cp>=linebuf)
					saveword[swptr++] = *cp;
				}
#ifdef debugging
debug(33,"before makeblankline dot=0%o",wc->wi_dot);
#endif
			if(updflag)
				lin=getslno(wc->wi_dot);
			else
				lin=makeblankline(wc,wc->wi_dot);
		    modify:
			sp = &screen[lin];
			setat(wc);	/* since dot was changed */
			showstat(wc);
			/* without the next line, enscreen won't put out
			 * a null line
			 ** not so, sl_modified now set im makeblankline
			 * it's also necessary to mark the line
			 * as the insert line */
			/** setting sl_modified here forces a writeout
			 *  of dot every time modify mode is entered
			 *  since this is now done in makeblankline,
			 *  don't do it here
			sp->sl_flags |= (sl_modified|sl_insertline);
			 **/
			sp->sl_flags |= (sl_insertline);
			insertadr = sp->sl_window->wi_dot;
#ifdef debugging
debug(33,"before enscreen dot=0%o lin=%d",wc->wi_dot,lin);
#endif
			switch(enscreen(lin,0)){

			case escesc:
			case en_eof:	/* ^D */
				continue;
			case '\n':
#ifdef debugging
debug(96,"newline return,mode=%d",mode);
#endif
				if(mode==modmode) continue;
#ifdef debugging
debug(33,"after enscreen dot=0%o",wc->wi_dot);
#endif
				if((  sp->sl_text[0]=='.'
				   && sp->sl_last==0)
				|| (  autoi
				   && sp->sl_pfirst==sp->sl_plast
				   && sp->sl_text[sp->sl_pfirst]=='.')){
					addr1.ad_addr=addr2.ad_addr=sp->sl_window->wi_dot;
/*
					deletes(wc);
					expand(wc);
*/					globp=".d"; commands();	/** yuck */
					continue;
					}
				else{
					int nl=1;		/** temp */
					inline += nl;	/** temp */
					goto nextinsert;
					}
				}
		case co_help:
			help();
			continue;
		case co_more:
			more();
			continue;
		case co_normal:
			continue;
		case co_esc:
			if((lin=getslno(wc->wi_dot))<0)
				lin=wc->wi_defline;
#ifdef debugging
debug(55,"co_esc: lin=%d",lin);
#endif
			if(screen[lin].sl_flags==sl_empty){
				mapc();	/* eat second char */
				errfunc("Window empty");
				}
			globp="\033";	/** kludgy */
			mode=modmode;
			goto modify;
		case co_shell:
		case co_find:
			errfunc("not implemented");
			}
		}
	}

#include "ed.h"
#include "window.h"
#include "scan.h"

startscan(line,col)
	{
	register struct sline *sp;
	struct window *wp;
	int *addr;
	char *cp;

	/** some of this code is common with split and should
	 ** be factored */
	topt(line,col,(char*)0);
	sp = &screen[line];
	wp = sp->sl_window;
	col -= wp->wi_textcol;
	addr = sp->sl_addr;
#ifdef debugging
debug(64,"startscan: line=%d col=%d char=%c",line,col,sp->sl_text[col]);
#endif
	sp->sl_text[col] |= 0200;	/* mark char */
	unmakeline(line=findfirst(line));
	sp->sl_text[col] &= 0177;	/* clear mark */
	for(cp=linebuf; *cp; *cp++)
		if(*cp&0200){
			*cp &= 0177;
			cursor = cp;
			break;
			}
	wwp=wp;
	wdot=sp->sl_addr;
	wcursor=cursor;	/* just in case */
	}

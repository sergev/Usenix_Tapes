#include <stdio.h>
#include <errno.h>
#include "window.h"
#include "make.h"
#include "terminal.h"
#include "edit.h"
#include "ed.h"
#include "file.h"

#ifdef debugging
/* VARARGS2 */
debug(n,s,x1,x2,x3,x4,x5,x6,x7,x8)
	char *s;
	{
	if(dbflag!=n)
		return(x1);
	saveloc();
	deleteline(statline);
	insertline(statline);
	printf(s,x1,x2,x3,x4,x5,x6,x7,x8);
	restorloc();
	psync();
	flush();
	if(dbflag>1)
		sleep(2);
	return(x1);
	}

#ifdef old
static int map[nlines]={
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,statline,cmdline};
#else
static
map(l)
	{
	if(l>=nlines-1) return(cmdline);
	if(l==nlines-2) return(statline);
	return(l);
	}
#endif
display(fmt)	/* display current state of screen structure */
	{
	register struct sline *sptr;
	register c,col;
	int l;

	saveloc();
	erasescreen();

	if(fmt==3){
		for(l=0; l<nlines-1; l++){
			sptr = &screen[map(l)];
			printf("%4d ",sptr->sl_lno);
			for(col=0; col<10; col++)
				printf(" %2x",sptr->sl_text[col]);
			printf(" |");
			for(col=ed_lline-10; col<ed_lline+2;col++)
				printf(" %2x",(sptr->sl_text[col])&0377);
			printf("\n");
			}
		goto ret;
		}
	if(fmt==2){
		for(l=0; l<nlines-1; l++){
			sptr = &screen[map(l)];
			printf("%4d %6o %6o %2d %2d %2d %2d %3o %3o %5x %8s\n"
				,sptr->sl_lno
				,sptr->sl_addr
				,sptr->sl_staraddr
				,sptr->sl_first
				,sptr->sl_last
				,sptr->sl_pfirst
				,sptr->sl_plast
				,sptr->sl_flags
				,sptr->sl_mark
				,sptr->sl_window
				,sptr->sl_data
				);
			}
		printf("lno  addr   staddr fi ls pf pl flg mrk windo data");
		goto ret;
		}
	for(l=0; l<nlines; l++){
		sptr = &screen[map(l)];
		printf("%4d %6o %2d %2d %2o %2o "
			,sptr->sl_lno
			,sptr->sl_addr
			,sptr->sl_first
			,sptr->sl_last
			,sptr->sl_flags
			,sptr->sl_mark
			);
		if(dbflag && fmt == 0){
			for(col=0; col<13; col++)
				printf(" %2x",sptr->sl_text[col]);
			printf(" |");
			for(col=ed_lline-1; col<ed_lline+3;col++)
				printf(" %2x",(sptr->sl_text[col])&0377);
			}
		else if(!dbflag && fmt == 0){
			for(col=0; col<7; col++)
				printf("%c",sptr->sl_data[col]);
			printf("|");
			for(col=0; (c=sptr->sl_text[col]) && col<40; col++)
				printf("%c",c);
			}
		else if(fmt == 1){
			printf("%6o ",*sptr->sl_addr);
			if(l<highmark) printf("%6o ",marks[l].mk_addr);
			}
		printf("\n");
		}
	printf("curline=%d, curcol=%d, dot=%d, fused=%d, lused=%d\n",
		(curstack[cursptr-1]>>8)&0377
		,curstack[cursptr-1]&0377
		,(wc->wi_dot-wc->wi_zero)&077777
		,wc->wi_ufirst		/* temp */
		,wc->wi_ulast		/* temp */
		);
				/* print screen[cmdline] in hex
	for(c=0; c<24; c++)
		printf("\\%2x",screen[cmdline].sl_text[c]);
		*/

    ret:
	flush();
	restorloc();
	}
#else
debug(){}
display(){}
#endif

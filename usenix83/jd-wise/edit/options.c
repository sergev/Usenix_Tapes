#include "ed.h"
#include "edit.h"
#ifdef debugging
#include <stdio.h>
#endif
#include "window.h"

struct option options[]={
	'a',op_bool,		0,"Accept control chars",	/* allflag */
	'b',op_bool,		0,"Shell beep",		/* sbflag */
	'c',op_int,		0,"Crash pid",		/* crashpid */
	'd',op_int,		0,"Debugging",		/* dbflag */
	'i',op_bool,		0,"Auto-indent",	/* indflag */
	'j',op_bool,		1,"Join spacing",	/* jsflag */
	'k',op_bool|op_spcl,	0,"Script",		/* sflag */
	'l',op_bool,		0,"Left break",		/* leftbreak */
	'm',op_trap,		0,"Margins",		/* margflag */
	'o',op_bool,		0,"Overwrite mode",	/* owriteflag */
	'p',op_int,		0,"Page threshold",	/* pagethresh */
	'r',op_bool,		0,"Read only mode",	/* rflag */
	's',op_bool|op_spcl,	0,"Spell checking",	/* spellflag */
	't',op_int|op_spcl,	8,"Tab spacing",	/* tabspace */
	'u',op_bool,		0,"Upper case text",	/* ucflag */
	'w',op_bool,		0,"Word break",		/* wflag */
	'x',op_bool,		0,"Auto expand",	/* xflag */
	'X',op_str|op_init,	0,"",			/* runfile */
	'y',op_bool,		0,"Spare",		/* yflag */
	'z',op_int,		0,"Expand size",	/* expsize */
	0
	};

struct option *
findopt(c)
	register c;
	{
	register struct option *op;

	for(op=options; op->op_name; op++)
		if(op->op_name==c)
			return(op);
	return((struct option *)0);
	}

getopt(c,str)	/* get options */
	char *str;
	{
	register char *p1;
	register struct option *op;

	if(c){
		if((op=findopt(c))==(struct option *)0
		|| (op->op_flag&op_init)){
			newline();
			errfunc("Bad option: %c",c);
			}
		if(op->op_flag&op_trap){
			dotrap(c);
			}
		else{
			pov(op->op_text,op->op_value,op->op_flag&op_int);
			op->op_value=gnv(op->op_flag&op_int);
			if(op->op_flag&op_spcl)
				dospcl(c);
			}
		}
	else{
		for(p1=str; *p1 != '\0'; p1++){
			c = *p1;
			if((op=findopt(c))==(struct option *)0
			|| (op->op_flag&op_trap)){
				errmsg("Bad option: %c",c);
				continue;
				}
			if(op->op_flag&op_str){
				c = *++p1;
#ifdef vax
				op->op_value = (int)(++p1);
#else
				(char*)op->op_value= ++p1;
#endif
				while(*++p1 != c)
					if(*p1=='\0') goto ckspcl;
				*p1='\0';
				}
			else if(op->op_flag&op_int){
				op->op_value=0;
				while(p1[1]>='0' && p1[1]<='9')
					op->op_value = op->op_value*10+(*++p1)-'0';
				}
			else
				op->op_value = !op->op_value;
#ifdef undef
fprintf(stderr,"c=%c value=%d flag=%d\n",*p1,op->op_value,op->op_flag);
sleep(3);
#endif
		ckspcl:
			if(op->op_flag&op_spcl)
				dospcl(c);
			}
		}
	}

dospcl(c)
	{
#ifdef undef
fprintf(stderr,"dospcl: c=%c (0%o)\n",c,c);
sleep(3);
#endif
	switch(c){
	case 'd':
		signal(SIGQUIT,SIG_DFL);
		break;
	case 'k':
		if((scriptfd=creat("script",0640))<0){
			errmsg("Can't create script file");
			exit(1);
			}
		break;
	case 's':
#ifdef undef
fprintf(stderr,"dospcl case=%c spellflag=%d\n",c,spellflag);
sleep(3);
#endif
		if(spellflag)
			makespell();
		else
			killspell();
		break;
	case 't':
		if(tabspace==0) tabspace=8;
		/** this was deleted for some reason.
		/** doing so really fouls up non-default tabspacing. */
		settabs(ed_textcol,tabspace);
		break;
		}
	}

dotrap(c)
	{
	switch(c){
	case 'm':
		pov("Left margin",wc->wi_lmarg+1,1);
		wc->wi_lmarg=gnv(1)-1;
		if(wc->wi_lmarg<0) wc->wi_lmarg=0;
		peekc='\n';	/** yuck */
		pov("Right margin",wc->wi_rmarg+1,1);
		wc->wi_rmarg=gnv(1)-1;
		if(wc->wi_rmarg<0) wc->wi_rmarg=wc->wi_ltext-1;
		break;
		}
	}

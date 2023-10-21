#include <stdio.h>
#include <errno.h>
#include "window.h"
#include "terminal.h"
#include "edit.h"
#include "ed.h"
#include "process.h"
#include "spell.h"


backspace()	/* echo a backspace character */
	{
/*	putchar('\b');	this is handled by psync now */
	curcol--;
	}

echo(c)
	register c;
	{
	c &= 0177;	/* in case it is marked */
	if(c=='\t' || c==intab) c=' ';
	if(c<' ' || c==0177)
		tputs(showctl(c));
	else{
		psync();
		putchar(c);
		}
	curcol++;
	Curcol++;
/*	flush(); not needed since the next rarec will get it */
	}

flush()
	{
	fflush(stdout);
	}

gol()		/* get optional letter */
	{	/* return (letter-@) or -1 if no letter */
	register c;

	c=getchr();
#ifdef debugging
debug(80,"gol: c=%c n=%d",c,(c&(~' '))-'@');
#endif
	if((c>='@' && c<='Z') || (c>='a' && c<='z'))
		return((c&(~' '))-'@');
	peekc=c;
	return(-1);
	}

gow()		/* get optional window */
	{	/* window # if no argument */
	register c,n;

	c=getchr();
	if((n=wnumber(c))>=0)
		return(n);
	peekc=c;
	return(-1);
	}

gnv(fmt)		/* get new value */
	{
	register c,i;

	eraseline(cmdline);
	write(1,": ",2);

	if(fmt==0){
		if((c=getchr())=='o') i=(getchr()=='n');
		else i=c-'0';
		while(getchr()!='\n');
		}
	else{
		for(i=0; (c=getchr()) != '\n';)
			i=i*10+(c-'0');
		}
	eraseline(errline);
	return(i);
	}

pov(msg,value,fmt)	/* print old value */
	char *msg;
	{
	register c;

	if((c=getchr()) != '\n'){
		peekc=c;
		return;
		}
	topt(errline,errcol,(char*)0);
	el();
	if(fmt==0)
		printf("%s was %s",msg,value?"on":"off");
	else
		printf("%s was %d",msg,value);
	printf(". Enter new value:");
	flush();
	}

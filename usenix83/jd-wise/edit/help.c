#include "ed.h"
#include "window.h"
#include "edit.h"

char* helpfile="/usr/lib/edit/help";

help()
	{
	register c;
	char word[128];
	char *cp=word;

	helping++;
	while(nonwhite(c=getchr()))	/* eat "lp" */
		if(c=='\n'){
			goto noarg;
			}
	while(!nonwhite(c=getchr()))
		;
	peekc=c;
	while((c=getchr())!='\n')
		*cp++ = c;
noarg:
	*cp = '\0';
	if(w[helpwindow]==0){
		sprintf(globp=globbuf,"ed %s\ne%c",helpfile,wname(fc));
		commands();
		}
	sprintf(globp=globbuf,word[0]?":d/[{,]%s[,}]/+1h":":d1h",word);
	commands();
	helping=0;
	}

more()
	{
	helping++;
	while(getchr()!='\n')	/* eat rest of line */
		;
	globp=":d.+4h";
	commands();
	helping=0;
	}

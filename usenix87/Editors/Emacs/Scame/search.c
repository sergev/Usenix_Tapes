/*	SCAME search.c				*/

/*	Revision 1.0.1  1985-02-23		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

#include "scame.h"

/* Incremental Search */

# define FIXCUR cur(screenlen-1,10+rlen+8*(failed<=level)+8*backward);

/* a few variables common for isearch() and find() */
# define ISEARCHDEPTH 250
char  oldsstr[80];
static char *dotarr[ISEARCHDEPTH];
static int failed=15000;
static char sstr[80];
static char iprompt[]="I-Search: ", riprompt[]="Reverse I-Search: ";
static int len, rlen, level;

void isearch(backward)
Bool backward;
{
char charr[ISEARCHDEPTH];
int faillen=0;
Bool done=FALSE;
int c;
int sb = gvars.search_backward,
    sf = gvars.search_forward;
	closegap();
	*commandprompt='\0';
	if (backward) echo(riprompt);
	else echo(iprompt);
	len=rlen=level=0;
 	dotarr[0]=dot;
	oldhome = home;
	while (!done) {
		charr[level]=c=inchar(TRUE);
		switch (c) {
			case 7:	 pchar(BELL);		/* ^G */
				 if (failed <= level) {
					level = failed - 1;
					len = faillen;
					sstr[len] = '\0';
					rlen = strsize(sstr);
					if (backward)
 						echo(riprompt);
					else echo(iprompt);
					strout(sstr);
					dot = dotarr[level] + len*(1-backward);
					break;
				 }
				 /* else fall through to... */
			case EOF:dot= dotarr[0];
				 done = TRUE;
				 FIXCUR
				 outchar(BELL, FALSE);
				 pchar(BELL);
				 break;
			case 17: c = inchar(FALSE);	/* ^Q */
				 FIXCUR
				 outchar(c, FALSE);
				 sstr[len++]=c;
				 sstr[len] = '\0';
				 rlen = rlen + 1 + (c < ' ' || c > '~');
				 find(dotarr[level],backward);
				 if (failed>level) faillen++;
				 break;
			case DEL: if (level > 0) {
					level--;
					if (charr[level] != sf
 					 && charr[level] != sb) {
					    rlen= rlen -1 -(sstr[--len]<' ');
					    sstr[len] = '\0';
					    FIXCUR
					    cleol(TRUE);
					    if (failed>level+1)faillen--; }
					dot=dotarr[level] + faillen;
					if (failed==level+1){
					    if (backward)
						echo(riprompt);
					    else echo(iprompt);
					    strout(sstr); } }
				  else pchar(BELL); break;
			default:  if (c == sf || c == sb) {
					if (level==0) {
				 	    backward = (c == sb);
					    strcpy(sstr,oldsstr);
					    faillen=len=strlen(sstr);
					    cur(screenlen-1,10+8*backward);
					    strout(sstr);
					    rlen = strsize(sstr);
					    find(dot, backward);
					}
					else if (failed<=level
					    && ((c==sf)^backward))
						pchar(BELL);
					else {
					  backward = (c==sb);
					  find(dotarr[level]+1-2*backward,backward);
					}
				}
				else if (c>=' ' && c<='~') {
					FIXCUR
					pchar(c);
					hpos++;
					sstr[len++]=c;
					sstr[len] = '\0';
					rlen++;
					find(dotarr[level], backward);
					if (failed>level) faillen++;
				}
				else {
					done=TRUE;
					strcpy(oldsstr,sstr);
					if (c != gvars.search_exit_char)
						unget(c);
				}
		} /* switch */
		if (dot < home || dot > away || updateflag) {
			updateflag = TRUE;
			update();
			modeline();
		}
		else findxy();
		refresh(TRUE);
/*
		if (!typeahead() && (cury != vpos || curx != hpos))
			cur(cury,curx);
*/
	}			/* while */
	if (abs(dot - dotarr[0]) > gvars.auto_push_point_option) {
		FIXCUR
		outchar('\0', FALSE);
		cur(cury, curx);
		pswap(dot,dotarr[0]);
		pushpopmark(1L);
		dot = dotarr[0];
	}
	clearecho();
}

static find(tdot,backward)
register char *tdot;
Bool backward;
{
static int oldway = -1;
char *tmpdot;
	tmpdot = dot;
	dot = tdot;
	if (failed <= level && backward == oldway) {
		dotarr[level+1]=dotarr[level];
		level++;
	}
	else {
		if (search(sstr, backward)) {
			dotarr[++level]=dot;
			if (!backward) dot += strlen(sstr);
			failed=15000;
		}
		else {
			pchar(BELL);
			echo("Failing ");
			strout(backward ? riprompt : iprompt);
			strout(sstr);
			dotarr[level+1]=dotarr[level];
			if (failed>++level) failed=level;
			dot = tmpdot;
		}
	}
	oldway = backward;
}

Bool search(str,backward)
register char *str;
Bool backward;
{
int len=strlen(str);
register char *tdot, *end;
register Bool swedish = (cb.majormode == SWEDISH);
char tstr[80];			/* should be len+1 */
	strncpy(tstr, str, min(80-1, len));
	end = tstr;
	while (*end = upcase(*end,swedish)) end++;
	tdot = dot;
	if (!backward) {
		closegap();
		end = z - len;
		while (tdot <= end && (*tstr != upcase(*tdot,swedish)
			|| !strneq(tdot,tstr,len,swedish))) tdot++;
		if (tdot > end)
			return(FALSE);
		else {
			dot = tdot;
			return(TRUE);
		}
	}
	else {
		if (tdot > z - len) tdot = z - len;
		end = buf;
		while (tdot >= end && (*tstr != upcase(*tdot,swedish)
			|| !strneq(tdot,tstr,len,swedish))) tdot--;
		if (tdot < end)
			return(FALSE);
		else {
			dot = tdot;
			dot= tdot;
			return(TRUE);
		}
	}
}

/* Replace */

void replace(old, new, query)
register char *old,*new;
Bool query;
{
register int c, oldlen,newlen;
Bool goahead = !query,
     checkforquit=FALSE;
	oldlen = strlen(old);
	newlen = strlen(new);
	while (TRUE) {
		if (!checkforquit && typeahead()) {
			c = inchar(TRUE);
			checkforquit = TRUE;
			if (c=='\007') { pchar(BELL); return; }
			else unget(c);
		}
		if (!search(old, FALSE))
			return;
		dot += oldlen;
query0:
		if (goahead) c = ' ';
	    	else {
	    		update();
			modeline();
	    		refresh(TRUE);
			*commandprompt = '\0';
			c = inchar(FALSE);
		}
	    	switch (c) {
	    		case '\007': pchar(BELL);	/* ^G */
			case EOF:
			case '\033': return;		/* ESC */

			case '\022': recurse();		/* ^R */
				     dot = max(dot-oldlen, buf);
				     break;

			case '\037':			/* ^_ */
			case '?':
				vfile(scamelib,"queryrep",FALSE,TRUE,NIL);
			    	goto query0;

			case '!': goahead = TRUE;
			case '.':
	    		case ' ': dot -= oldlen;
				  pushpopmark(1L);
				  if (newlen > oldlen) {
 				       insertc('\0',(long)(newlen-oldlen));
				       dot -= newlen-oldlen;
 			  	  }
	    		  	  else if (oldlen > newlen)
 				  	delchar((long)(oldlen-newlen));
	    		  	  strncpy(dot,new,newlen);
	    		  	  cb.modified = TRUE;
	        	  	  dot += newlen;
				  updateflag = TRUE;
				  if (c == '.') return;
			  	  break;

			case DEL: dot -= oldlen-1;
				  break;

			default:  pchar(BELL);
				  goto query0;
	    	}
	}
}

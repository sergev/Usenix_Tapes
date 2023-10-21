/*	SCAME help.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

#include "scame.h"

extern int k_void(), k_ubk();

/* HELP! */
help()
{
int c;
	*commandprompt = '\0';
	echo(NIL);
hquery:
	typing = FALSE;
	strout("Doc (? for help): ");
	outchar(c = upcasearr[inchar(FALSE)], TRUE);
	switch (c) {
		case '\007': pchar(BELL); break;

		case '\037':
		case '?':
			vfile(scamelib,"helpmenu",FALSE,FALSE,NIL);
			echo(NIL);
			goto hquery;

		case '1':
		case '2':
			fixbuftab(PUT);
			sprintf(cb.filename,"%s/%s%c",scamelib,"tut",c);
			findfile();
			basename(cb.filename,cb.filename,FALSE);
			modeline();
			break;
			  
		case 'A': apropos(); break;
		case 'B': vfile(scamelib,"basic",TRUE,TRUE,NIL); break;
		case 'C': where(); break;
		case 'L': showlastinput(); break;
		case 'N': vfile(scamelib,"news",TRUE,TRUE,NIL); break;
		case 'Q': break;
		case 'S': vfile(scamelib,"summary", TRUE,TRUE,NIL); break;
		case DEL: break;
		default:
  			echo(NIL);
			outchar(c, TRUE);
			strout(" is meaningless here. ");
			pchar(BELL);
			goto hquery;
	}
	echo(NIL);
}

where()
{
funcp *table;
int c;
char s[80];
	c = b_getkey("Character");
	key_name(c, s, TRUE);
	if (c == b_control('X')) {
		if (!quiet) strout("C-X ");
		c = upcasearr[inchar(FALSE)];
		strcat(s, " ");
		key_name(c, &s[strlen(s)], FALSE);
		table = c_x_disptab;
	}
	else if (c == b_meta('O')) {
		if (!quiet) outchar(' ', FALSE);
		c = inchar(FALSE);
		strcat(s, " ");
		key_name(c, &s[strlen(s)], FALSE);
		table = m_o_disptab;
	}
	else table = disptab;
	if (table[c] == k_void)
		strcat(s, " is an illegal key");
	else if (table[c] == k_ubk)
		strcat(s, " is not bound to any function");
	else {
		strcat(s, " runs the function ");
		func_name(table[c], &s[strlen(s)]);
	}
	strcat(s, ".");
	typeout(s);
}


showlastinput()
{
char outstr[80];
int i;
char c;
	i = lstindex;
	if (lastinput[i] == (char) 0200) i=0;
	*outstr = '\0';
	do {
		c = lastinput[i];
		if (c == (char) 0200) break;
		if (c & 0200) { 
			strcat(outstr,"Meta-");
 			c &= 0177;
		}
		if	(c == '\010') strcat(outstr,"Backspace");
		else if (c == '\011') strcat(outstr,"Tab");
		else if (c == '\012') strcat(outstr,"Linefeed");
		else if (c == '\r'  ) strcat(outstr,"Return");
		else if (c == '\033') strcat(outstr,"Escape");
		else if (c ==  ' '  ) strcat(outstr,"Space");
		else if (c ==  DEL  ) strcat(outstr,"Rubout");
		else {
			if (c < ' ' || c==DEL) { 
				strcat(outstr,"^");
 				c = (c+64) & 0177;
			}
			sprintf(&outstr[strlen(outstr)],"%c",c);
		}
		strcat(outstr," ");
		if (strlen(outstr) > screenwidth - 15) {
			typeout(outstr);
			*outstr = '\0';
		}
		i = (i + 1) % LASTINPUTSIZE;
	} while (i != lstindex);
	if (*outstr != '\0') typeout(outstr);
}


/* apropos */
apropos()
{
char c, tstr[80];
	*tstr = '\0';
	c = instring("Apropos: ",tstr,0,"\033","\007\r");
	if (c=='\r') vfile(scamelib,"aproposlist",FALSE,TRUE,tstr);
}


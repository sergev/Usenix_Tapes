/*	SCAME vars.c				*/

/*	Revision 1.0.1  1985-02-19		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

/*	Scame Variables				*/

# include "scame.h"

struct {
	char *name;
	long *varp;
} vartab[] = {
	{ "Auto Fill Mode",		&cb.minormodes.autofill },
	{ "Auto Push Point Option",	&gvars.auto_push_point_option },
	{ "Bottom Display Margin",	&gvars.bottom_display_margin },
	{ "CBREAK Mode",		&gvars.cbreak_mode },
	{ "Comment Column",		&gvars.commcol },
	{ "Display Matching Paren",	&cb.lvars.disp_matching_paren },
	{ "Fill Column",		&cb.lvars.fillcolumn },
	{ "Hackmatic",			&gvars.hackmatic },
	{ "Mail Check Interval",	&gvars.mail_check_interval },
	{ "Overwrite Mode",		&cb.minormodes.overwrite },
	{ "Quote Char",			&gvars.quote_char },
	{ "Search Backward",		&gvars.search_backward },
	{ "Search Exit Char",		&gvars.search_exit_char },
	{ "Search Forward",		&gvars.search_forward },
	{ "System Output Holding",	&gvars.system_output_holding },
	{ "Tab Self Insert",		&gvars.tab_self_insert },
	{ "Top Display Margin",		&gvars.top_display_margin },
	NIL
	};

setvariable(a)
long a;
{
char str[80], tstr[80], *p;
int c,i;
	if (!xarg) {
		errmes(ILA);
		return;
	}
	*str = '\0';

	/* This code should be merged with getfuncname() */
	do {
		c=instring("Variable: ",str,strlen(str),"\t","\007\033\r ?");
		if (c == 7 || (c == 13 && *str=='\0')) {
			typing = FALSE;
			return;
		}
		switch (c) {
		    case '?':
			listmatching((struct tabstruct *)vartab,str); break;
		    case ' ':
			strcpy(tstr,str);
			strcat(str, " ");
			if (tablk((struct tabstruct *)vartab,str,0) <0) {
			    strcpy(str,tstr);
			    (void) tablk((struct tabstruct *)vartab,str,0);
			    if (strlen(str) == strlen(tstr))
				pchar(BELL);
			}
			if ((p=index(str+strlen(tstr), ' ')) != NIL)
				*(p+1) = '\0';
			break;
		    default:
			if ((i = tablk((struct tabstruct *)vartab,str,0)) <0)
				pchar(BELL);
		}
	} while (i<0 || index("\033 ?", c) != NIL);
	switch (i) {
		case 2: if (a >=40 && a <=100)
				gvars.bottom_display_margin = a;
			else errmes(ILA); break;
		case 3: *(vartab[i].varp) = a;
			ttycbreak(); break;
		case 8: gvars.mail_check_interval = a;
			chkmailcnt = gvars.mail_check_interval;
			break;
		case 15: if (a <= 40 && a >= 0)
				gvars.top_display_margin = a;
			else errmes(ILA); break;
		default: *(vartab[i].varp) = a;
	}
	typing = FALSE;
}

viewvariable()
{
char str[80];
int c,i;
	*str = '\0';
	do {
		c=instring("Variable: ", str,strlen(str),"\t","\007\033\r?");
		if (c == 7 || (c == 13 && *str=='\0')) {
			typing = FALSE;
			return;
		}
		if (c == '?') listmatching((struct tabstruct *)vartab,str);
		else if ((i = tablk((struct tabstruct *)vartab,str,0)) <0)
			pchar(BELL);
	} while (i<0 || c==ESC || c=='?');
	echo(NIL);
	printf("%s = %ld", vartab[i].name,*(vartab[i].varp));
	fflush(stdout);
	typing = FALSE;
}


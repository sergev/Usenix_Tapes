/*	SCAME x_funcs.c				*/

/*	Revision 1.0.1  1985-02-17		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

/*	Functions normally not bound to keys	*/

# include "scame.h"

extern int k_ubk(), k_void(), k_insertc();

x_set_key()
{
funcp fp, *table;
int c;
char s[80], ts[40];
	fp = getfuncname("Function Name: ");
	if (fp != (funcp) 0) {
		sprintf(s, "Put %s on key", func_name(fp, ts));
		c = b_getkey(s);
		if (c == b_control('X')) {
			if (!quiet) strout("C-X ");
			c = inchar(FALSE);
			c = upcase(c,FALSE);
			table = c_x_disptab;
		}
		else if (c == b_meta('O')) {
			if (!quiet) outchar(' ',FALSE);
			c = inchar(FALSE);
			table = m_o_disptab;
		}
		else table = disptab;
		if (c != -1 && yesorno("Go Ahead") == 1)
			table[c] = fp;
	}
}

x_viewfile()
{
static char tfile[FILENAMESIZE] = "";
	if (getfilename("View File", tfile))
		vfile(NIL, tfile,TRUE,TRUE,NIL);
	modeline();
}

x_wallchart()
{
register int i;
funcp fp;
char s[80];
Bool same=FALSE;
	for (i=0; i<512; i++) {
		fp = disptab[i];
		if (fp != k_ubk && fp != k_void && fp != k_insertc) {
			if (i > 0 && i < 511 && fp == disptab[i-1]
			    && fp == disptab[i+1])
				same = TRUE;
			else {
				if (same) {
					insertstr("thru\n",4);
					same = FALSE;
				}
				key_name(i, s, FALSE);
				if (strlen(s) < 8) strcat(s, "\t");
				strcat(s, "\t");
				func_name(fp, &s[strlen(s)]);
				strcat(s, "\n");
				insertstr(s, strlen(s));
			}
		}
	}
	for (i=0; i<128; i++) {
		fp = c_x_disptab[i];
		if (fp != k_ubk && fp != k_void) {
			strcpy(s, "C-X ");
			key_name(i, &s[strlen(s)], FALSE);
			if (strlen(s) < 8) strcat(s, "\t");
			strcat(s, "\t");
			func_name(fp, &s[strlen(s)]);
			strcat(s, "\n");
			insertstr(s, strlen(s));
		}
	}
	for (i=0; i<128; i++) {
		fp = m_o_disptab[i];
		if (fp != k_ubk && fp != k_void) {
			strcpy(s, "M-O ");
			key_name(i, &s[strlen(s)], FALSE);
			if (strlen(s) < 8) strcat(s, "\t");
			strcat(s, "\t");
			func_name(fp, &s[strlen(s)]);
			strcat(s, "\n");
			insertstr(s, strlen(s));
		}
	}
}

x_insertfile()
{
static char infilename[FILENAMESIZE]="";
	if (getfilename("Insert File",infilename))
		insertfile(infilename);
}

x_adamode()
{
	setmode(ADA);
}

x_autofillmode()
{
	setminormode(&cb.minormodes.autofill, arg);
}

x_ovwrtmode()
{
	setminormode(&cb.minormodes.overwrite, arg);
}

x_cmode()
{
	setmode(C);
}

x_cbreakmode()
{
	gvars.cbreak_mode = xarg ? arg : (long) !(gvars.cbreak_mode);
	ttycbreak();
}

x_checkmail()
{
	checkmail(TRUE);
}

x_dired()
{
	exec("dired");
}

x_fundamentalmode()
{
	setmode(FUNDAMENTAL);
}

x_inschr()
{
	if (xarg && arg >= 0 && arg <= 127)
				insertc((char)arg,1L);
			else errmes(ILA);
}

x_lispmode()
{
	setmode(LISP);
}

x_pascalmode()
{
	setmode(PASCAL);
}

x_setvariable()
{
	setvariable(arg);
}

x_swedishmode()
{
	setmode(SWEDISH);
}

x_revert_file()
{
	if (xarg || yesorno("Restore file from disk") == 1) {
		saveforundo(x_revert_file);
		revertfile();
	}
}

x_tabify()
{
	saveforundo(x_tabify);
	tabify();
}

x_sortbuf()
{
#ifdef SORT
	saveforundo(x_sortbuf);
	pipe_through(SORT, NIL, NIL, NIL, buf, z);
	upd2();
	modeline();
#else
	errmes(NYI);
#endif
	killing = FALSE;
}

x_flush_matching_lines()
{
#ifdef GREP
char flushstr[80];
	saveforundo(x_flush_matching_lines);
	if (instring("Pattern: ", flushstr, 0, "","\r\007") == '\r')
		pipe_through("/usr/ucb/grep", "-v", flushstr, NIL, dot, z);
	upd2();
	modeline();
#else
	errmes(NYI);
#endif
	killing = FALSE;
}

x_savekbdmac()
{
char tfile[FILENAMESIZE];
	*tfile = '\0';
	if (getfilename("Save macro on file", tfile))
		savekbdmac(tfile);
}

x_loadkbdmac()
{
char tfile[FILENAMESIZE];
	*tfile = '\0';
	if (getfilename("Load macro from file", tfile))
		loadkbdmac(tfile);
}

x_replace()
{
char old[SCRDIMX],new[SCRDIMX], tprompt[SCRDIMX], *tmpdot=dot;
	typing = FALSE;
	strcpy(tprompt,"Replace: ");
	if (instring(tprompt,old,0,NIL,"\r\007") != '\007' && *old !='\0') {
		if (strlen(old) <= 20)
			sprintf(&tprompt[strlen(tprompt)-2],"  %s    ",old);
		else *tprompt='\0';
		strcat(tprompt,"With: ");
		if (instring(tprompt,new,0,NIL,"\r\007") != '\007') {
			saveforundo(x_replace);
			replace(old, new, FALSE);
		}
	}
	echo(NIL);
	dot=tmpdot;
}

x_rot13_region() {
	saveforundo(x_rot13_region);
	rot13_region();
}

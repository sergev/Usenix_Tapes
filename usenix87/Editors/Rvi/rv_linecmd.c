#include "rv.h"
#include <ctype.h>

boolean	ed_undo;	/* Set TRUE if last mod was direct ed cmd */

extern boolean did_botprint;   /* used by botprint() */

static struct cm_cmdlist {  /* Commands */
	char cm_cmd[12];
	INT  cm_index;
} cmdlist[] = {
	"E",	3,
	"Q",	2,
	"edit", 3,
	"file", 5,
	"next", 4,
	"quit",	2,
	"se",	1,
	"set",	1,
	"sh",	8,
	"shell",8,
	"w",	6,
	"wq",	7,
	"write", 6,
	"xit",	7,
	"z",	7,
};

static struct op_oplist {   /* Options */
	char op_option[12];
	INT  op_index;
} oplist[] = {
	"ai",		1,
	"all",		2,
	"autoindent", 	1,
	"debug",	3,
	"fortran",	4,
	"list",		5,
	"scroll",	6,
	"shiftwidth",	7,
	"sw",		7,
	"tabstops",	8,
	"timeout",	10,
	"to",		10,
	"ts",		8,
	"wrapscan",	9,
	"ws",		9,
};


void
rv_linecmd(cmd)
/*
 * Execute command line
 */
char *cmd;
{
	register char *s, *s2;
	INT	i, j;
	boolean	flag;
	char	buf[12];
	
	if ((s2 = cmd) == NULL || *s2 == '\0')
		return;
	while (isalnum(*s2))
		++s2;
	j = *s2;
	*s2 = '\0';
	i = binsearch(cmd, (char *) cmdlist, sizeof(struct cm_cmdlist),
		sizeof(cmdlist) / sizeof(struct cm_cmdlist));
	if (i >= 0)
		i = cmdlist[i].cm_index;
	*s2 = j;

	xmit_curline();
	switch(i) {
case 1:
	/*
	 * Set
	 */
	if (*s2 == '\0')
		s2 = "all";
	for (;;) {
		while (isspace(*s2) && *s2 != '\0')
			++s2;
		if (*s2 == '\0')
			break;
		s = s2;
		while (isalnum(*s2))
			++s2;
		if (*s2 != '\0')
			*s2++ = '\0';
		if (*s == 'n' && *(s+1) == 'o') {
			s += 2;
			flag = FALSE;
		} else
			flag = TRUE;
		i = binsearch(s, (char *) oplist, sizeof(struct op_oplist),
			sizeof(oplist) / sizeof(struct op_oplist));
		if (i >= 0)
			i = oplist[i].op_index;

		switch (i) {

		case -2:
			botprint(TRUE, "\"%s\" is not unique\n", s);
			break;

		case -1:
			botprint(TRUE, "\"%s\": No such option - 'set all' gives all option values\n", s);
			return;

		case 1: /* autoindent */
			set_autoindent = flag;
			break;
		
		case 2: /* all */
			botprint(FALSE, "%sautoindent\n", set_autoindent ?
				"" : "no");
			botprint(FALSE, "debug=%d\n", set_debug);
			botprint(FALSE, "%sfortran\n", set_fortran ? "" :
				"no");
			botprint(FALSE, "%slist", set_list ? "" : "no");
			botprint(FALSE, "scroll=%d\n", set_scroll);
			botprint(FALSE, "shiftwidth=%d\n", set_shiftwidth);
			botprint(FALSE, "tabstops=%d\n", set_tabstops);
			botprint(FALSE, "%stimeout", set_timeout ? "" : "no");
			botprint(FALSE, "%swrapscan", set_wrapscan ? "" : "no");
			break;

		case 3: /* debug */
			set_debug = atoi(s2);
			while (isdigit(*s2))
				++s2;
			break;

		case 4:
			set_fortran = flag;
			break;

		case 5: set_list = flag;
			fetch_window(screen.sc_lineno-NUM_WINDOW_LINES/4-
				LINES-2+1, TRUE);
			break;

		case 6: /* scroll */
			set_scroll = atoi(s2);
			while (isdigit(*s2))
				++s2;
			if (set_scroll <= 0)
				set_scroll = 1;
			break;

		case 7: /* shiftwidth */
			set_shiftwidth = atoi(s2);
			while (isdigit(*s2))
				++s2;
			if (set_shiftwidth <= 0)
				set_shiftwidth = 1;
			else if (set_shiftwidth > 40)
				set_shiftwidth = 40;
			break;

		case 8: /* tabstops */
			set_tabstops = atoi(s2);
			while (isdigit(*s2))
				++s2;
			if (set_tabstops <= 0)
				set_tabstops = 1;
			else if (set_tabstops > 40)
				set_tabstops = 40;
			break;

		case 9: /* wrapscan */
			set_wrapscan = flag;
			break;

		case 10: /* timeout */
			set_timeout = flag;
#ifdef USG
			keypad(stdscr, flag ? 1 : 2);
#endif
			break;

		default:
			botprint(FALSE, "That option is not yet implemented\n");
			break;
		}
	}
	break;


case 2: /* quit */
	if (file.fi_modified && *s2 != '!') {
		botprint(TRUE, "No write since last change (:q! overrides)");
		return;
	}
	Quit();
	break;

case 3: /* edit */
	if (file.fi_modified && *s2 != '!') {
		botprint(TRUE, "No write since last change (:edit! overrides)");
		return;
	}
	if (*s2 == '!')
		++s2;
	while (isspace(*s2))
		++s2;
	edit(s2);
	break;

case 4: /* next */
	if (file.fi_modified && *s2 != '!') {
		botprint(TRUE, "No write since last change (:next! overrides)");
		return;
	}
	if (*s2 == '!')
		++s2;
	while (isspace(*s2))
		++s2;
	if (*s2 == '\0' && *nextfile == '\0') {
		botprint(TRUE, "No more files to edit");
		return;
	}
	edit(s2);
	break;

case 5: /* file */
	while (isspace(*s2))
		++s2;
	if (*s2 == '\0') {
		sizemsg();
		return;
	}
	xmit_ed("f %s\n", s2);
	strncpy(file.fi_name, s2, 126);
	xmit_sync();
	(void) recv_sync(TRUE);
	sizemsg();
	break;

case 6: /* write */
	i = 0;
	if (*s2 == '!') {
		++s2;
		i = 1;
	}
	while (isspace(*s2))
		++s2;
	if (*s2 == '\0' && file.fi_name[0] == '\0') {
		botprint(TRUE, "No current filename");
		return;
	}
	if (i)
		xmit_ed("!rm -f %s\n", *s2 ? s2 : file.fi_name);
	if (*s2)
		xmit_ed("w %s\n", s2);
	else {
		xmit_ed("w\n");
		s2 = file.fi_name;
	}
	xmit_sync();
	botprint(FALSE, "\"%s\"", s2);
	hitcr_continue();
	refresh();
	if (recv_sync(TRUE)) {
		botprint(FALSE, "\"%s\" %d lines", s2, file.fi_numlines);
		if (strcmp(s2, file.fi_name) == 0)
			file.fi_modified = FALSE;
	}
	else
		hitcr_continue();
	break;

case 7: /* wq */
	if (file.fi_modified)
		rv_linecmd("w");
	if (!file.fi_modified)
		rv_linecmd("q");
	break;

case 8: /* shell */
	rv_shell("sh -i");
	break;
	
default:
	if (*s2 == '!') {  /* Shell escape */
		if (strcmp(++s2, "sh") == 0)
			s2 = "sh -i";
		else if (strcmp(s2, "csh") == 0)
			s2 = "csh -i";
		rv_shell(s2);
		return;
	}
	did_botprint = TRUE;
	i = cmd[strlen(cmd)-1];
	if (i == 'a' || i == 'c' || i == 'i' || i == 'H' || i == 'X'
		     || i == 'P') {
		botprint(FALSE, "That command is reserved");
		return;
	}
	toss_undo();
	if (i == 'w') {
		ed_undo = FALSE;
		file.fi_modified = FALSE;
	} else
		ed_undo = TRUE;
	xmit_ed("%d\n", screen.sc_lineno);
	xmit_sync();
	xmit_ed("%s\n", cmd);
	xmit_sync();
	xmit_ed(".=\n");
	(void) recv_sync(FALSE);
	(void) recv_sync(2);
	(void) fgets(buf, 10, file.fi_fpin);
	if ((i = atoi(buf)) <= 0)
		i = screen.sc_lineno;
	hitcr_continue();
	fetch_window(i-NUM_WINDOW_LINES/4-LINES/2+1, TRUE);
	move_abs_cursor(i, COL_FIRST_NONWHITE);
	break;
	}
}

/*
 * wgraph(str)
 *
 * translate a string to a graphics rendition, via the "graph" database.
 * The terminal-independent codes in str are:
 *
 *	-	horizontal line
 *	|	vertical line
 *	+	cross
 *	T	upper tee
 *	i	lower tee (look at an `i' on a vt100 for details)
 *	>	left tee
 *	<	right tee
 *	,	LR corner
 *	.	UR corner (backslash would be preferable but you can't gage it)
 *	`	LL corner
 *	/	UL corner
 *
 * On SVR2, this provides SVR3 curses characters ACS_* for use everywhere.
 */

#include <curses.h>

#ifndef ACS_DIAMOND

extern char *strchr();

struct graph {
	char g_name[6];
	short g_graph;
} __graph[] = {
	"hor",		'-',
	"vert",		'|',
	"cross",	'+',
	"ltee",		'+',
	"rtee",		'+',
	"utee",		'+',
	"dtee",		'+',
	"ll",		'+',
	"ul",		'+',
	"lr",		'+',
	"ur",		'+',
	"scan9",	'_',
	"scan1",	'-',
	"up",		'^',
	"down",		'v',
	"left",		'<',
	"right",	'>',
	"dimnd",	'+',
	"check",	':',
	"deg",		'\'',
	"plmin",	'#',
	"bullet",	'o',
	"board",	'#',
	"lamp",		'#',
	"block",	'#',
	"",		'\0',
};

extern char *getenv();

/*
 * Format of tgraph database:  /usr/lib/tgraph
 *
 * ttyspec [ | ttyspec ...], name=ch, ...
 *
 * In other words, terminfo format.  However, a graphics code is ONE
 * character in length (this is forced by curses, unless someone wedges
 * wgraph into the curses source).
 *
 * NOTE:  They did (sVr3).  However, it remains one character per glyph.
 */

__tgr_load() {
	FILE *fp;
	char *cp;

	if ((cp = getenv("TGRAPH")) == (char *) 0)
		cp = "/usr/lib/tgraph";
	if ((fp = fopen(cp, "r")) == (FILE *) 0)
		return;
	if ((cp = getenv("TERM")) == (char *) 0)
		cp = "unknown";
	if (!__tgr_find(fp, cp))
		return;
	if (feof(fp)) {
		fclose(fp);
		return;
	}
	while (__tgr_gent(fp))
		;
	fclose(fp);
}

__tgr_find(fp, term)
FILE *fp;
char *term; {
	char iline[1024];
	char *cp, *dp;
	char oldch;
	
	while (fgets(iline, sizeof iline, fp) != (char *) 0) {
		if (iline[0] == '\n' || iline[0] == '\t' || iline[0] == ' ' || iline[0] == '#')
			continue;
		for (cp = iline; *cp != '\0' && *cp != '\n' && *cp != ','; cp++) {
			while (*cp == ' ' || *cp == '\t')
				cp++;
			if (*cp == '\n' || *cp == ',' || *cp == '\0')
				break;
			for (dp = cp; *dp != ' ' && *dp != '\t' && *dp != ',' && *dp != '\n' && *dp != '\0'; dp++)
				;
			if (*dp == '\n' || *dp == '\0')
				break;
			oldch = *dp;
			*dp = '\0';
			if (strcmp(cp, term) == 0)
				return 1;
			if (oldch == '\n' || oldch == '\0')
				break;
			*dp = oldch;
			cp = ++dp;
		}
	}
	return 0;
}

__tgr_gent(fp)
FILE *fp; {
	static int atbol = 1;
	char hunk[512];
	char *hunkp;
	char ch;
	struct graph *tgrp;
	
	hunkp = hunk;
	while ((ch = getc(fp)) != ',') {
		if (feof(fp))
			return 0;
		if (ch == '\n') {
			atbol = 1;
			continue;
		}
		else if (atbol && ch != '\t' && ch != ' ')
			return 0;
		else if (hunkp == hunk && (ch == '\t' || ch == ' ')) {
			atbol = 0;
			continue;
		}
		else
			atbol = 0;
		*hunkp++ = ch;
	}
	*hunkp = '\0';
	for (hunkp = hunk; *hunkp != '\0'; hunkp++)
		if (*hunkp == '=')
			break;
	if (*hunkp == '\0')
		return 1;
	*hunkp++ = '\0';
	for (tgrp = __graph; tgrp->g_name[0] != '\0'; tgrp++)
		if (strcmp(tgrp->g_name, hunk) == 0)
			break;
	if (tgrp->g_name[0] == '\0')
		return 1;
	tgrp->g_graph = canon(hunkp) | A_ALTCHARSET;
	return 1;
}

canon(str)
char *str; {
	char ch;

	/* We only do the first character, that being the limit for curses */

	switch (*str) {
	case '\\':
		switch (*++str) {
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 'b':
			return '\b';
		case 'r':
			return '\r';
		case 'f':
			return '\f';
		case 'E':
			return '\033';
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			ch = *str - '0';
			while (*++str != '\0' && strchr("01234567", *str) != (char *) 0) {
				ch *= 8;
				ch += *str - '0';
			}
			return ch;
		default:
			return *str;
		}
	case '^':
		return (*++str) & 0x1f;
	default:
		return *str;
	}
}

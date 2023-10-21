/* local.c --- local features that didn't fit in elsewhere:
 *	error reporting:  warnings about local features
 *			  warnings about syntax errors &c
 *			  fatal error reporting
 */
#include <ctype.h>
/* give users warnings if they use non-standard features... */
#ifndef MAXPTR
#include "tdef.h"
#endif MAXPTR
#include "ext.h"
#ifndef WARN_ONCE
#include "local.h"
#endif WARN_ONCE
#if defined(WARNLEVELREG) || defined(REPORTERRS)
/* v.h defines a single structure, to which the extern applies.
 * It contains the names of the general-purpose number-registers.
 * We use it for giving the line-number in error messages, and for
 * a "warning level" register to enable turning off warnings about local
 * features from within troff.
 * - req
 */
extern
#include "v.h"
#endif 
#ifdef LOCALWARN

/*ARGSUSED1*/
lwarn(s, a1, a2, a3, a4, a5)
	char *s;
{
#ifdef WARNLEVELREG
	if (v.wl) {
		if (v.wl & WARN_ONCE) {
			errmsg(EWARN, "Input uses local features; use troff -W for explicit information\n");
			v.wl &= (~WARN_ONCE);
		} else {
			errmsg(0, 0, s, a1, a2, a3, a4, a5);
		}
	}
#else !WARNLEVELREG
	if (warninglevel) {
		/* stderr comes from tdef.h; different to stdio.h's stderr! */
		if (warninglevel & WARN_ONCE) {
			errmsg(EWARN, "Input uses local features; use troff -W for explicit information\n");
			warninglevel &= (~WARN_ONCE);
		} else {
			errmsg(0, 0, s, a1, a2, a3, a4, a5);
		}
	}
#endif WARNLEVELREG
}
#endif LOCALWARN
#ifdef REPORTERRS

char *progname;   /* set in n1.c:main() */
char *ifilename;  /* set by n1.c:nextfile() */
int reporterrs = LERR_EVERYTHING;

/*ARGSUSED2*/
void
errmsg(func, arg, mesg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	int (*func)();	/* called after printing the message, if != 0 */
	int arg;	/* passed to func */
	char *mesg;	/* passed to printf */
{

	/* don't give warnings if they're turned off
	 * note that this turns off warnings about local features too, but
	 * you can set warninglevel to achieve that more cleanly!
	 *
	 */
	if (func == 0 && !(reporterrs & LERR_WARNINGS)) {
		return;
	}
	/* NOTE:  troff has its own version of printf and stdio! */
	if (!progname || !*progname) {
		progname = "ditroff";
	}
	/* progname might contain %.  Remember that we're not using stdio */
	fprintf(stderr, "%s:", progname);
	if (ifilename && *ifilename) {
		fprintf(stderr, "%s:", ifilename);
		/* ifilename set by n1.c:nextfile();  it might not always
		 * change when .so happens, though!
		 */
	}
	fprintf(stderr, "%d:%s", v.cd, (func == 0) ? "warning: " : " ");

	if (mesg && *mesg) {
		fprintf(stderr, mesg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		if (mesg[strlen(mesg)-1] != '\n') { /* no trailing newline */
			/* terminal modes may be odd, so add \r as well */
			fprintf(stderr, "\r\n");
		}
	} else {
		fprintf(stderr, "Internal error: mesg NULL (__FILE__:__LINE__)\n");
	}

	if (func != 0) {
		(* func)(arg);
	} /* else simply return */
}

char *
realname(a)
	tchar a;
{
	/* return string representation of the name.  Trivial now, but might
	 * become complex if long names are allowed
	 * -- req
	 */


#ifdef TCHARTOS
	char one, two;
	char *rp;
#endif TCHARTOS
	static char buffer[30];		/* max is 2 char name + 1 for \0 */
					/* but also use "[none]" etc */
	buffer[0] = buffer[1] = buffer[2] = '\0';
	buffer[0] = (char) a;
	buffer[1] = a >> BYTE;

	if (!buffer[0] && !buffer[1]) {
		return(strcpy(buffer, "[null name]"));
	} 
	if (!buffer[0]) {
		/* don't know if this can happen */
		buffer[0] = ' ';
	}
#ifdef TCHARTOS
	one = buffer[0];
	two = buffer[1];
	if (buffer[0]) strcpy(buffer, tchartos((tchar) one));
	if (buffer[1]) strcat(buffer, tchartos((tchar) two));
#endif TCHARTOS
	return buffer;
}

#endif REPORTERRS
#ifdef ROTATEPAGE

int pageangle = 0;

void
caserp()	/* .rp n --- rotate page by n degrees */
		/* .rp n x y -- put topleft at x, y */
{
	static int oldangle = 0;
	int newx = 0, newy = 0;

#ifdef LOCALWARN
	lwarn(".rp (rotate page) is non-standard");
#endif LOCALWARN
	if (skip()) {
		pageangle = oldangle;
	} else {
		oldangle = pageangle;
		/* does inumb() allow +n to be an increment? */
		pageangle = inumb(&pageangle);
	}
	/* don't give % a non-portable -ve argument, in case it
	 * ever matters... */
	while (pageangle < 0) {
		pageangle += 360;
	}
	if (pageangle >= 360) {
		pageangle %= 360;
	}
	/* swap over width & length of page to apply to the nearest
	 * diagonal
	 */
	if ((pageangle < 45 && pageangle > 0) ||
	    (pageangle >= 315) || (pageangle >= 135 && pageangle < 225)) {
		extern int paperwidth, paperlength;	/* in t10.c */

		int tmpwid = paperwidth;

		paperwidth = paperlength;
		paperlength = tmpwid;
	}
	/* now look for newx, newy */
	if (!skip()) {
		newx = inumb(&newx);
		if (skip()) {
#ifdef REPORTERRS
			errmsg(EWARN, ".rp: new y pos missing (assuming 0)");
#endif REPORTERRS
			newy = 0;
		} else {
			newy= inumb(&newy);
			if (!skip()) {
#ifdef REPORTERRS
				errmsg(EWARN,
				".rp: extra characters ignored from \"%s\"",
							tchartos(ch));
#endif REPORTERRS
			}
		}
	}
	ptpangle(newx, newy);
}


/* ptpangle() is called from newpage() in t10.c.  Probably won't be able to
 * affect the 1st page, as newpage() gets called at the end of each page.
 * But it should also be called on the 1st-page transition, to start page 1.
 * I haven't checked to see if it is.
 * - req
 */
ptpangle(newx, newy)
	int newx, newy;
{
	static int angle = 0;
	/* only print out command if the angle
	 * 	is non-zero (on every page, to help pre-processors)
	 *	has changed (obvious!)  Note that it might change to 0.
	 */
	if (pageangle || pageangle != angle) {
		fprintf(ptid, "x A %d %d %d\n", pageangle, newx, newy);
		angle = pageangle;
	}
}
#endif ROTATEPAGE
#ifdef USEFONTPATH
char *
followpath(path, file, mode)
	char *path, *file;
	int mode;
{
	/* "path" is interpreted as a colon-separated list of names.
	 * We look in each of them for "file".
	 */
	int fd;	/* try to open the file at each stage */

	register char *p = path;
	static char result[NS];

	while (*p) {
		register char *q = result;

		*q = '\0';
		while (*p && *p != ':') {
			if (q - result >= NS - (strlen(file) + 2)) {
				/* 2 is for '/' and trailing '\0' */
#ifdef REPORTERRS
				errmsg(EWARN, "Name in path too long (%s)",
							result);
#else !REPORTERRS
				fprintf(stderr, "troff: \"%s\" too long in \"%s\"",
					result, path);
#endif REPORTERRS
				return 0;
			}
			*q++ = *p++;
			*q = '\0';
		}
		if (*p && *p == ':')
			p++;	/* step over the colon */
		if (q == result)
			*q++ = '.';
		*q++ = '/';
		*q = '\0';

		if ((fd = open(strcat(result, file), mode)) >= 0) {
			/* opened it OK... */
			close(fd);
			return(result);
		}
		q = result;
	}
	/* failed... */
	return (char *) 0;	/* not using stdio -- NULL not defined */
}
#endif USEFONTPATH
#ifdef FONTFAMILIES

/* caseff -- fontfamily */
caseff()
{
	char familyname[NS];
	int j, k;
	tchar i;

	if (skip()) {
#ifdef REPORTERRS
		errmsg(EWARN, "call to .ff with no arguments ignored");
		return;
#endif REPORTERRS
	}
#ifdef LOCALWARN
	lwarn("Font Family (.ff) is non-portable");
#endif LOCALWARN
	lgf++; /* don't want ligatures in our nice FamilyName */
	for (k = 0; k < (NS - 1); k++) {
		if (((j = cbits(i = getch())) <= ' ') || (j > 0176))
			break;
		familyname[k] = j;
	}
	familyname[k] = 0;
	ch = i;
	lgf--;
	if (familyname[0]) {
		setfamily(familyname);
	}
#ifdef REPORTERRS
	else {
		errmsg(EWARN, ".ff: illegal font family name");
	}
#endif REPORTERRS
}

/* setfamily(name) -- use font family ``name'' */
/* We do this by reading a ``FamilyFile'' looking for a line that starts
 * with name followed by a colon.  After the colon there is a comma-
 * separated list of font names to load in successive positions.
 * Later I'd like to add more (that's why the commas are there):
 * S=n would set the slant to n on that font position, H the height,
 * b the bd factor, s the size, ....
 * But that's for later.  Maybe *after* breakfast.
 * - req 20/10/1986
 */

#ifndef FAMILYFILENAME
#define FAMILYFILENAME "FamilyFile"
#endif !FAMILYFILENAME
int
setfamily(name)
	char *name;
{
	char tmp[NS]; /* to store filename if !fontpath */
	char *FamilyFileName = tmp;
	int famfile;
	char linebuf[NS];
	int linenumber = 0; /* for error reporting. */
	int pos = 0;
	int c;  /* current input char from family file */
	int inextf; /* index into nextf[] */

	extern char *fontpath;

#ifdef USEFONTPATH
	if (fontpath && *fontpath) {
	       if (!(FamilyFileName = followpath(fontpath, FAMILYFILENAME, 0))){
#ifdef REPORTERRS
			errmsg(EWARN, "Can't find \"%s\" in path \"%s\" to load font family %s from",
					FAMILYFILENAME, fontpath, name);
#else !REPORTERRS
			fprintf(stderr
				"troff: can't find \"%s\" for font family %s\n",
					FAMILYFILENAME, name);
#endif REPORTERRS
			return 0;
		}
	} else
#endif USEFONTPATH
		sprintf(tmp, "/usr/lib/troff/descs/dev%s/%s",
						devname, FAMILYFILENAME);
	if (FamilyFileName != tmp) {
		(void) strcpy(tmp, FamilyFileName);
		FamilyFileName = tmp;
	}
	if ((famfile = open(FamilyFileName, 0)) < 0) {
#ifdef REPORTERRS
		errmsg(EWARN,
		"Can't open font family file \"%s\" looking for family \"%s\"",
							FamilyFileName, name);
#else !REPORTERRS
		fprintf(stderr, "troff: can't open %s to load family %s\n"",
							FamilyFileName, name);
#endif REPORTERRS
		return 0;
	}
	/* now have an open file `famfile' */

	/* look for a line starting with name: */
	
	while ((c = getfamchar(famfile)) >= 0) {
		linenumber++;
		/* at start of line... */

		/* skip initial whitespace */
		while (c > 0 && (c == ' ' || c == '\t') )
			c = getfamchar(famfile);

		if (c == *name) {
			char *p = name + 1;

			while ((c = getfamchar(famfile)) > 0) {
				if (!*p || *p++ != c) { 
					break;
				}
			}
			if (!*p && c == ':') { /* found */
				break;
			} else {
				/* mismatch... */
				c = '#';
			}
		} else {
			c = '#'; /* ignore rest of line! */
		}
		if (c != '#' ) {
#ifdef REPORTERRS
			errmsg(EWARN,
"Error reading font family file \"%s\" line %d: \'%c\' unexpected",
						FamilyFileName, linenumber, c);
#else !REPORTERRS
			fprintf(stderr, "troff: %s: %d: format error\n",
						FamilyFileName, linenumber);
#endif REPORTERRS
			close (famfile);
			return 0;

		} else { /* comment or a mismatch */
			while ((c = getfamchar(famfile)) > 0 && c != '\n')
				;
			if (c < 0) { /* EOF */
				break;
			}
		}
	}
	/* now either c < 0 (EOF or error) or c is 1 char past a match */
	if (c < 0) {
#ifdef REPORTERRS
		errmsg(EWARN, "Font Family %s not found in \"%s\"",
						name, FamilyFileName);
#else !REPORTERRS
		fprintf(stderr, "troff: Font Family %s not found in \"%s\"",
						name, FamilyFileName);
#endif REPORTERRS
		return 0;
	}
	/* now we have a match.  Want to look for font names */
	while ((c = getfamchar(famfile)) > 0 && c != '\n') {
		char shortname[3];
		char *p = shortname;

		/* skip whitespace */
		while (c > 0 && (c == ' ' || c == '\t') )
			c = getfamchar(famfile);

		if (c == '\n' || c < 0)
			break;

		/* now the font name */
		/* c already contains the 1st char of this. */
		shortname[0] = c;
		if ((c = getfamchar(famfile)) > 0 && c != '\n') {
			shortname[1] = c;
		} else break;
		if (c == ' ' || c == '\t')
			shortname[1] = '\0';
		else
			shortname[2] = '\0';
		
		/* got a font name */
		/* try to mount it on the next position */
		if (setfp(++pos, PAIR(shortname[0], shortname[1]), 0) < 0) {
			break; /* setfp does an error message */
		}

		/* skip whitespace */
		while ((c = getfamchar(famfile)) > 0 && (c == ' '||c == '\t'))
			;

		if (c == ':') {
			/* do a .so on the filename */
			/* I'd like to push back ".so file" here!
			* - req
			*/
			inextf = nextf[0] = 0;

			/* skip whitespace */
			while (((c = getfamchar(famfile)) > 0) &&
						(c == ' ' || c == '\t')) {
					continue;
			}
			while (c > 0 && c != ' ' && c != '\n' && c != '\t') {
				nextf[inextf++] = c;
				nextf[inextf] = 0;
				c = getfamchar(famfile);
			}

			if (!inextf || !nextf[0]) {
#ifdef REPORTERRS
				errmsg(EWARN, "%s: %d: colon (:) unexpected",
						FamilyFileName, linenumber);
#else !REPORTERRS
				fprintf(stderr,
					"troff: %s: %d: colon (:) unexpected",
						FamilyFileName, linenumber);
#endif REPORTERRS
				break;
			}
			/* skip trailing whitespace */

			while (c > 0 && (c == ' '||c == '\t')) {
				(c = getfamchar(famfile));
			}
#ifdef REPORTERRS
			if (c != '\n' && c != '#') {
				if (c < 0) {
					errmsg(EWARN, "Unexpected EOF at line %d of font family file \"%s\"", linenumber, FamilyFileName);
				} else {
					errmsg(EWARN, "%s: %d: \"%c\" unexpected after filename \"%s\"", FamilyFileName, linenumber, c, nextf);
					c = '#';
				}
			}
#endif REPORTERRS
			/* do this after checking the rest of the line so that
			 * we can use nextf in the error message!
			 */
			dosofile("Loading Font Family");
		}
		if (c == '\n' || c == '#' || c < 0) {
			break;
		} else if (c == ',') {
			continue;
		} else {
#ifdef REPORTERRS
			errmsg(EWARN, "%s: %d: Expected a comma, found \"%c\"",
					FamilyFileName, linenumber, c);
#else !REPORTERRS
			fprintf(stderr,
				"troff: %s: %d: Expected a comma, found \"%c\"",
					FamilyFileName, linenumber, c);
#endif REPORTERRS
			break;
		}
	}
	close (famfile);
#ifdef REPORTERRS
	/* pos == 0 implies no fonts loaded
	 * inextf == 0 implies no file loaded (via dosofile())
	 */
	if (pos == 0 && inextf == 0) {
		errmsg(EWARN, ".ff: no action in \"%s\" for family \"%s\"",
					FamilyFileName, name);
	}
#endif REPORTERRS
	return pos;
}

/* this is the bulk of caseso().  I've factored it out to use the same code
 * in both places */
int
dosofile(wherefrom)
	char *wherefrom;
{
	register i;
	register char	*p, *q;
	/* offl, ioff and ipl are from n1.c */
	extern long offl[];
	extern long ioff;
	extern filep ipl[];

	if ((i = open(nextf, 0)) < 0 || (ifi >= NSO)) {
#ifdef REPORTERRS
		errmsg(done, 02, "%s: can't open file %s", wherefrom, nextf);
#else !REPORTERRS
		fprintf(stderr, "troff: can't open file %s\n", nextf);
		done(02);
#endif REPORTERRS
	}
	flushi();
	ifl[ifi] = ifile;
	ifile = i;
	offl[ifi] = ioff;
	ioff = 0;
	ipl[ifi] = ip;
	ip = 0;
	nx++;
	nflush++;
	if (!ifl[ifi++]) {
		p = ibuf;
		q = xbuf;
		xbufp = ibufp;
		xeibuf = eibuf;
		while (p < eibuf)
			*q++ = *p++;
	}
}


int
getfamchar(f)
	int f;
{
	/* return the next char from file 'fam' */
	/* wouldn't it have been nice to have used stdio ? */
	/* should probably do this like .cf I suppose.  */

	static char smallbuf[NS];
	static char *p = 0;

	if (!p || !*p) {
		if ((read(f, smallbuf, NS - 1)) <= 0) {
			/* EINTR problem here? */
			return -1;
		}
		p = smallbuf;
	}
	return (*p++) & 0377; /* discourage sign extension */
}

#endif FONTFAMILIES
#ifdef ANYBASESTR
/* would like to define setbasestr() here.
 * the idea is to allow text to be set on an arbitrary baseline.
 * \R'l 1i 1i'hello there'
 * should draw "hello there" at a 45 degree angle.
 * Perhaps it'd be easier to stick to straight lines for now, as then at
 * least we can work out the width of the things!  Perhaps it should be
 * \R'n'str'.
 * But I don't know how to do that, either!
 * - req
 */
#endif ANYBASESTR
#ifdef BLANKSMACRO

static int bmmacro = PAIR('s', 'p');

casebm()
{
	lgf++;
	skip();
	if (!(bmmacro = getrq())) {
		bmmacro = PAIR('s', 'p');
	}
}

int nrspaces, nrlines;
/* this is really for the .B and .N registers in n4.c: the idea is that one
 * could write a paragraph macro that was invoked by magic on blank lines:
 * .de P
 * .sp \\n(.NV \" .N is the number of blank lines
 * .if \\n(.B>0  .ti +\\n(PDu \" .B is indent...
 * ..
 * with other processing as well.  This is a big improvement, as it means
 * that blank lines in the output can be handled consistently.  It also
 * allows for things like poetry, and for style parameters (e.g. the para
 * indent in "P" above is fixed irrespective of the number of spaces on
 * the input line.
 * 
 * bug: callblank is called for each blank line instead of once at the end
 *
 * req
 */
callblank(spaces, lines)
	int spaces;	/* number of leading spaces */
	int lines;	/* after this many blank lines */
{
	/* 2nd arg to control() is a 1 if we have to collect args for a
	 * user-defined macro. */
	nrspaces = spaces;
	nrlines = lines;
	if (bmmacro == PAIR('s', 'p')) {
		callsp();
	} else {
		control(bmmacro, 0);
	}
}
#endif BLANKSMACRO
#ifdef TCHARTOS

/* routine to provide a string representation of a tchar
 * BUG:
 * uses a static buffer to hold the result.
 * req 1986
 */

char *
tchar0(i)
	tchar i;
{
	static char resultbuf0[5]; /* plenty! */
	static char resultbuf1[5]; /* plenty! */
	static char which = '0';

	int ch = cbits(i);
	register char *resultbuf = (which == '0') ? resultbuf0 : resultbuf1;

	which ^= '0';
	*resultbuf = '\0';	/* paranoia */

	switch (ch) {
	case 002: case 003: case 005: case 006: case 007: /* unfair? */
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B':
	case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
	case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
	case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W':
	case 'X': case 'Y': case 'Z': case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7': case '8': case '9': case '!':
	case '@': case '#': case '$': case '%': case '^': case '&': case '*':
	case '(': case ')': case '_': case '-': case '=': case '+': case '`':
	case '~': case ']': case '[': case '{': case '}': case '\'': case '"':
	case ';': case ':': case '<': case '>': case ',': case '.': case '/':
	case '?': case '|': case '\\': case ' ':
	case '\n': case '\r': case '\f':
		resultbuf[0] = ch;
		resultbuf[1] = '\0';
		return resultbuf;

	default:
		ch &= 0177;	/* bottom 7 bits (must be ASCII) */
		if (iscntrl(ch)) {
			resultbuf[0] = '^';
			resultbuf[1] = (ch + '@') & 0177;  /* ASCII ONLY */
			resultbuf[2] = '\0';
			return resultbuf;
		}
		/* ``shouldn't happen'' */
		errmsg(EWARN, "Unknown char 0%o in tchar0()", ch);
		return "[UNKNOWN]";
	}
}

char *
tchartos(c)
	tchar c;
{
	/* use 2 buffers and alternate, so 2 calls to tchartos() OK.  Ugh. */
	static char resultbuf0[20]; /* biggest case currently \h'32767' */
	static char resultbuf1[20];
	static char which = '0';

	register char *p = (which == '0') ? resultbuf0 : resultbuf1;
	char *resultbuf = p;

	which ^= '0';
	*p = '\0';

	if (ismot(c)) {
		if (isvmot(c)) {
			(void) strcpy(p, tchar0(eschar));
			while (*p) {
				p++;
			}
			sprintf(p, "v'%d'", sbits(c));
			return resultbuf;
		} else {
			return "[\\h or TAB]";
		}
	}

	if (iszbit(c)) {
		(void) strcat(p, tchar0(eschar));
		*p++ = 'z';
		*p = '\0';
	}

	if (cbits(c) > 128) { /* special char */
		(void) strcpy(p, tchar0(eschar));
		strcat(p, "(");
		while (*p) {
			p++;
		}
		(void) strcpy(p, &chname[chtab[cbits(c) - 128]]);
		return resultbuf;
	}

	switch(cbits(c)) {

	case 0:		return "[NUL]";
	case IMP:	return "[IMP]";
	case TAB:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "t");
	case RPT:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "l");
	case CHARHT:	(void) strcat(p, tchar0(eschar));
			while (*p)
				p++;
			sprintf(p, "H'%d'", sbits(c));
			return resultbuf;
	case SLANT:	(void) strcat(p, tchar0(eschar));
			while (*p)
				p++;
			sprintf(p, "S'%d'", sbits(c) - 180);
			return resultbuf;
	case 027:	(void) strcat(p, tchar0(eschar));
			return strcat(p, " ");	/* em-space */
	case FONTPOS:	(void) strcat(p, tchar0(eschar));
			while (*p)
				p++;
			*p++ = 'f';
			if (((c>>16) >> BYTE) && ((c>>16) >> BYTE))
				*p++ = '(';
			*p++ =  (c>>16) & BMASK;
			*p++ =  ((c>>16) >> BYTE);
			*p++ =  '\0';
			return resultbuf;
	case DRAWFCN:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "D");
	case LEFT:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "{");
	case RIGHT:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "}");
	case FILLER:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "&");
	case OHC:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "%"); /*WRONG*/
	case CONT:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "c");
	case PRESC:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "e");
	case XPAR:	(void) strcat(p, tchar0(eschar));
			return strcat(p, "!");
	case FLSS:	return "[FLSS]";
	case WORDSP:	(void) strcat(p, tchar0(eschar));
			return strcat(p, " ");
	case ESC:	(void) strcat(p, tchar0(eschar));
			return strcat(p, tchar0(eschar));

	default:	return strcpy(p, tchar0(c));
	}

	/*NOTREACHED*/
}
#endif TCHARTOS

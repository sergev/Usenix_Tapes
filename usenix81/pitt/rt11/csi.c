/*
 *	RT11 EMULATOR
 *	command string interpretation
 *
 *	Daniel R. Strick
 *	May 31, 1979
 *	1/9/81  -- added filename extension defaulting to csigen()
 *	4/15/81 -- added switch recognition to csinter() via cswitch()
 *	7/23/81 -- implemented the linbuf option for csi requests (untested!)
 *	8/24/81 -- implemented the .GTLIN request
 */

#include "define.h"
#include "syscom.h"

#define	FOUTMAX	2
#define	FINMAX	8
#define	CSTSIZ	100	/* size of cstring read from tty */
#define	NAMSIZ	50	/* max size of filename from cstring */
#define	MAXLBF	81	/* size of linbuf argument */

struct	filspec	{	/* input file specification block */
	rad50	f_dev;		/* device name */
	rad50	f_name[2];	/* file name */
	rad50	f_ext;		/* extension */
};

struct	outspec	{	/* output file specification block */
	rad50	f_dev;
	rad50	f_name[2];
	rad50	f_ext;
	word	f_size;
};

struct	comspec	{	/* csispc generated file spec block */
	struct outspec	ospec[FOUTMAX+1];
	struct filspec	ispec[FINMAX-FOUTMAX];
};


/*
 *  Interpret a command string.
 *  The rt11 user stack pointer at the time of the system call
 *  is the first argument.  The second argument flags the special
 *  mode of the csi.
 *  The system call arguments are popped off of the stack, the
 *  command string is obtained and interpreted, the option
 *  values are pushed onto the stack, and the resulting stack
 *  pointer is returned.
 */
word *
csinter (asp, specflag)
word *asp;
{
	register char *cp, *cp2;
	register unsigned int chan;
	struct comspec *outspc;
	word *sp;
	rad50 *defext;
	char *cstring;
	char *linbuf;
	char *prompt;
	char cstore[CSTSIZ];
	char name[NAMSIZ];
	char *csifile();
	struct comspec comspec;
	word *cswitch();
	char *strcop();

	cstring = (char *) *asp++;
	defext = (rad50 *) *asp++;
	outspc = (struct comspec *) *asp;
	linbuf = NULL;
	prompt = "*\200";
	if ((int) outspc & 1) {
		linbuf = (char *) *++asp;
		if (-- (* (int *) &outspc)   ==   0)	/* yecch */
			prompt = (char *) defext;	/* for .GTLIN */
	}
	sp = asp;
	*sp = 0;		/* an empty switch list */
	for (;;) {
		if (specflag)
			clear (&comspec, &comspec+1);
		else
			closall (FINMAX);
		cp = cstring;
		if (cp == NULL) {
			cp = cstore;
			getcst (cp, cp+CSTSIZ, prompt);
		}
		if ((cp2 = linbuf) != NULL)
			strcop (cp, cp2, cp2+MAXLBF);
		if (outspc == NULL)
			return (++sp);
		chan = FOUTMAX+1;
		for (cp2 = cp; *cp2 != EOS;)
			if (*cp2++ == '=')
				chan = 0;
		for (;;) {
			cp = csifile (cp, name, name+NAMSIZ);
			sp = cswitch (sp, chan, name+1);
			if (name[1] != EOS  &&  syscom.c_error < 0) {
			    if (specflag)
				csispc (chan, name+1, defext, &comspec);
			    else
				csigen (chan, name+1, defext);
			}
			if (name[0] == EOS)
				break;
			if (syscom.c_error >= 0)
				continue;
			if (chan >= FINMAX ||
			    chan > FOUTMAX  &&  name[0] == '='  ||
			    chan == FOUTMAX  &&  name[0] != '=') {
				syscom.c_error = 0;
				continue;
			}
			if (name[0] == '=')
				chan = FOUTMAX;
			++chan;
		}
		if (syscom.c_error < 0) {
			if (specflag)
				copy (&comspec, outspc, outspc+1);
			return (sp);
		}
		if (!specflag)
			closall (FINMAX);
		if (cstring != NULL) {
			*asp = 0;
			return (asp);
		}
		switch (syscom.c_error) {

			default:
				printf("?ILL CMD?\n");
				break;

			case 1:
				printf("?ILL DEV?\n");
				break;

			case 3:
				printf("?DEV FUL?\n");
				break;

			case 4:
				printf("?FIL NOT FND?\n");
				break;
		}
		syscom.c_error = -1;
	}
}


/*
 *  Get command string from user (terminal).
 */
getcst (cstring, cstrend, prompt)
char *cstring, *cstrend, *prompt;
{
	register c;
	register char *cp;

	do {
		rtprint (prompt);
		cp = cstring;
		do {
			c = getchr();
			if (c <= 0) {
				c = EOS;
				syscom.c_error = 0;
				putchr ('\n');
			}
			if (c == '\n')
				c = EOS;
			if (c == ' '  ||  c == '\t')
				continue;
			*cp++ = c;
			if (cp >= cstrend) {
				syscom.c_error = 0;
				--cp;
			}
		} while (c != EOS);
	} while (*cstring == EOS);
}


/*
 *  Copy command string input up to the next
 *  filename delimiter into argument "name"+1.
 *  The delimiter is stored in name[0].
 *  Blanks and tabs are ignored.
 *  Upper case is converted into lower case.
 *  The value of this function is a pointer
 *  to the first unused character in the
 *  argument string.
 */
char *
csifile (acp, name, endname)
char *acp, *name, *endname;
{
	register int c;
	register char *cp, *np;

	cp = acp;
	np = name + 1;
	do {
		c = *cp++;
		if (c == ' '  ||  c == '\t')
			continue;
		if (c >= 'A'  &&  c <= 'Z')
			c += 'a' - 'A';
		if (np < endname)
			*np++ = c;
		else
			syscom.c_error = 0;
	} while (c!=EOS  &&  c!=','  &&  c!='=');
	*--np = EOS;
	*name = c;
	return (cp);
}


/*
 *  This monumental kludge returns the number
 *  whose representation in octal is the same as the
 *  representation of the argument number in decimal.
 */
unsigned int
fix8 (n)
unsigned int n;
{
	if (n < 10)
		return (n);
	return (fix8(n/10) * 8 + n%10);
}


/*
 *  Convert the switch value at the beginning of the
 *  argument character string into a word, stash that
 *  word in the argument switch value location, and
 *  return a pointer to the first charcter that follows
 *  the switch value in the argument character string.
 *  A switch value may be an octal number, a decimal
 *  number (followed by a period), or a string of from
 *  0 to 3 radix50 characters.
 */
char *
swival (acp, sp)
char *acp;
word *sp;
{
	register int c;
	register char *cp;
	register unsigned int n;
	char *fnstri();

	n = 0;
	for (cp = acp;  (c = *cp) >= '0'  &&  c <= '9';  ++cp)
		n = n * 10 + c - '0';
	if (c == '.'  &&  cp > acp)
		c = *++cp;
	else
		n = fix8 (n);
	if (cp > acp  &&  (c == '/'  ||  c == ':'  ||  c == '\0'))
		*sp = n;
	else
		cp = fnstri (acp, sp, 1);
	return (cp);
}


/*
 *  Strip switches off the end of the argument file specification
 *  string and push descriptions of them onto the rt11 stack.
 *  A pointer to the new stack top is returned.
 */
word *
cswitch (sp, chan, name)
word *sp;
unsigned int chan;
char *name;
{
	register char *cp;
	register int c, sno;

	for (cp = name;  *cp != '/'  &&  *cp != '\0';  ++cp)
		;
	while (*cp != '\0') {
		if (cp[0] != '/'  ||  cp[1] == '\0') {
			syscom.c_error = 0;	/* syntax */
			return (sp);
		}
		*cp++ = '\0';
		c = *cp++ & 0177;
		if (c >= 'a'  &&  c <= 'z')
			c += 'A' - 'a';
		do {
			sno = *sp;
			*sp = (chan << 8  |  c);
			if (*cp == ':') {
				sp[-1] = sp[0] | 0100000;
				cp = swival (++cp, sp);
				--sp;
			}
			*--sp = ++sno;
		} while (*cp == ':');
	}
	return (sp);
}


/*
 *  Process a file in "general" mode.
 *  This means opening the file on the argument channel.
 *  If a file name has no extension, the default extension
 *  for a file on the given channel is supplied.
 *  A file name has an extension if it contains a period.
 *  If a file name has a trailing period, that period is ignored
 *  (explicit specification of a null extension to avoid the default).
 */
csigen (chan, aname, defext)
unsigned int chan;
char *aname;
rad50 *defext;
{
	register unsigned int ch;
	register char *ap, *np;
	register int exflag;
	char name[NAMSIZ+5];
	char *fnschr();

	ch = chan;
	exflag = NO;
	ap = aname;
	for (np = name;  (*np = *ap++) != EOS; ) {
		if (*np++ == '.')
			exflag = YES;
		if (np >= name+NAMSIZ) {
			syscom.c_error = 0;	/* name too long */
			return;
		}
	}
	if (exflag == NO) {
		*np++ = '.';			/* append default extension */
		fnschr (np, defext, 1);
		if (ch <= FOUTMAX)
			fnschr (np, defext+ch+1, 1);
	} else {
		if (*--np == '.')		/* trim period for null ext. */
			*np = EOS;
	}
	if (ch <= FOUTMAX) {
		if (fenter(ch,name) >= 0)
			syscom.c_error = 3;
	} else {
		if (flookup(ch,name,RDWR) >= 0)
			syscom.c_error = 4;
	}
}


/*
 *  Process a file in "special" mode.
 *  This means filling in a file specification block
 *  to be used in a future ENTER or LOOKUP system call.
 *  File size specification (for output files) is not
 *  recognized.
 */
csispc (chan, name, defext, cspecp)
unsigned int chan;
char *name;
rad50 *defext;
struct comspec *cspecp;
{
	register unsigned int ch;
	register char *cp;
	register struct filspec *fsp;
	char *fnstri();

	if (*name == EOS)
		return;
	ch = chan;
	if (ch > FINMAX)
		return;
	fsp = &cspecp->ispec[ch-FOUTMAX-1];
	if (ch <= FOUTMAX)
		fsp = (struct filspec *) &cspecp->ospec[ch];
	fnstri("dk", &fsp->f_dev, 1);
	cp = fnstri (name, fsp->f_name, 2);
	if (*cp == ':') {
		fsp->f_dev = fsp->f_name[0];
		cp = fnstri (cp+1, fsp->f_name, 2);
	}
	if (*cp == '.') {
		cp = fnstri (cp+1, &fsp->f_ext, 1);
	} else {
		fsp->f_ext = defext[0];
		if (ch <= FOUTMAX)
			fsp->f_ext = defext[ch+1];
	}
	if (*cp != EOS)
		syscom.c_error = 0;
}


/*
 *  Fill "len" words at location "r50p" with the radix50
 *  representation of the first "len"*2 characters  (from the
 *  radix50 character set) in "string".
 *  If the string is less than "len"*2 characters long, the
 *  radix50 string is padded with zeros (blanks).
 *  The value of this function is a pointer to the
 *  non-radix50 character that terminated "string".
 */
char *
fnstri (string, r50p, len)
char *string;
rad50 *r50p;
{
	register char *cp;
	register int r50, rc;
	int lc2;

	cp = string;
	do {
		r50 = 0;
		for (lc2 = 3; --lc2 >= 0;) {
			r50 = r50 * 050;
			if ((rc = radix50(*cp)) < 0)
				continue;
			r50 += rc;
			++cp;
		}
		if (len > 0) {
			*r50p++ = r50;
			--len;
		}
	} while (len > 0  ||  rc >= 0);
	return(cp);
}


/*
 *  Convert the radix50 string of "len" words into
 *  a character string in the argument "string".
 *  The argument "string" is assumed to be at least
 *  "len"*3+1 characters long.
 *  Trailing blanks are trimmed.
 *  The value of this function is a pointer
 *  to the end of the generated string.
 */
char *
fnschr (string, r50p, len)
char *string;
rad50 *r50p;
{
	register unsigned int r50;
	register char *cp;
	register int i;

	cp = &string[len*3];
	*cp = EOS;
	while (len > 0) {
		r50 = r50p[--len];
		for (i = 3; --i >= 0;) {
			*--cp = r50toa (r50 % 050);
			r50 = r50 / 050;
			if (cp[0] == ' '  &&  cp[1] == EOS)
				cp[0] = EOS;
		}
	}
	while (*cp != EOS)
		++cp;
	return (cp);
}


/*
 *  Construct the name of a file in the argument string.
 *  The argument string is assumed to be large
 *  enough to hold the file name.
 *  The file name is contructed according to the usual RT11 syntax:
 *		<device> : <name> . <extension>
 *  Certain devices are handled specially to produce path names
 *  that are useful on unix systems:
 *	"dk:" and "sy:" are ignored.
 */
makfnam (string, fspecp)
char *string;
struct filspec *fspecp;
{
	register char *cp, *stp;
	register struct filspec *fp;

	cp = string;
	stp = cp;
	fp = fspecp;
	if (fp->f_dev != 0) {
		cp = fnschr (cp, &fp->f_dev, 1);
		*cp++ = ':';
	}
	*cp = '\0';
	if (streq (stp, "dk:")  ||  streq (stp, "sy:"))
		cp = stp;
	cp = fnschr (cp, fp->f_name, 2);
	if (fp->f_ext != 0) {
		*cp++ = '.';
		fnschr (cp, &fp->f_ext, 1);
	}
}

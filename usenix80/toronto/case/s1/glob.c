#                            
/* global command --

   Modified for Case Shell

   glob params

   "*" in params matches r.e ".*"
   "?" in params matches r.e. "."
   "[...]" in params matches character class
   "[...a-z...]" in params matches a through z.

   perform command with argument list
  constructed as follows:
     if param does not contain "*", "[", or "?", use it as is
     if it does, find all files in current directory
     which match the param, sort them, and use them

   use pathstr to find command.


   args to glob are as follows:

	argv[0] - glob name
	argv[1] - path string
	argv[2] - shell name
	argv[3] - command name
	argv[4]-argv[n] - arguments

*/

#include "errnos.h"

#define STRSIZ  5120
char    ab[STRSIZ];             /* generated characters */
char    *ava[200];              /* generated arguments */
char    **av &ava[1];
char    *string ab;
int     errno;
int     ncoll;
char    *dfltfile;      /* for user bin dir */
char	*shellnam, *pathstr;

main(argc, argv)
char *argv[];
{
	register char *cp;

	if (argc < 5) {
		write(2, "Arg count\n", 10);
		return;
	}
	argv++;
	pathstr = *argv++;
	shellnam = *argv++;
	*av++ = *argv;
	while (--argc >= 4)
		expand(*++argv);
	if (ncoll==0) {
		write(2, "No match\n", 9);
		return;
	}
	pexec(ava[1], &ava[1]);
}

expand(as)
char *as;
{
	register char *s, *cs;
	register int dirf;
	char **oav;
	static struct {
		int     ino;
		char    name[16];
	} entry;

	s = cs = as;
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) {
			*av++ = cat(s, "");
			return;
		}
	}
	for (;;) {
		if (cs==s) {
			dirf = open(".", 0);
			s = "";
			break;
		}
		if (*--cs == '/') {
			*cs = 0;
			dirf = open(s==cs? "/": s, 0);
			*cs++ = 0200;
			break;
		}
	}
	if (dirf<0) {
		write(2, "No directory\n", 13);
		exit(1);
	}
	oav = av;
	while (read(dirf, &entry, 16) == 16) {
		if (entry.ino==0)
			continue;
		if (match(entry.name, cs)) {
			*av++ = cat(s, entry.name);
			ncoll++;
		}
	}
	close(dirf);
	sort(oav);
}

sort(oav)
char **oav;
{
	register char **p1, **p2, **c;

	p1 = oav;
	while (p1 < av-1) {
		p2 = p1;
		while(++p2 < av) {
			if (compar(*p1, *p2) > 0) {
				c = *p1;
				*p1 = *p2;
				*p2 = c;
			}
		}
		p1++;
	}
}

toolong()
{
	write(2, "Arg list too long\n", 18);
	exit(1);
}

match(s, p)
char *s, *p;
{
	if (*s=='.' && *p!='.')
		return(0);
	return(amatch(s, p));
}

amatch(as, ap)
char *as, *ap;
{
	register char *s, *p;
	register scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc =& 0177) == 0)
			scc = 0200;
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;
		while (cc = *p++) {
			if (cc==']') {
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

umatch(s, p)
char *s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}

compar(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

cat(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;
	register int c;

	s2 = string;
	s1 = as1;
	while (c = *s1++) {
		if (s2 > &ab[STRSIZ])
			toolong();
		c =& 0177;
		if (c==0) {
			*s2++ = '/';
			break;
		}
		*s2++ = c;
	}
	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ])
			toolong();
		*s2++ = c = *s1++;
	} while (c);
	s1 = string;
	string = s2;
	return(s1);
}


/*
 * pexec - path search and execute a file
 */

pexec(name, argv)
char *name, *argv[];
{
	extern errno;
	register char *cp;
	char tline[48];
	char txe2big, txeacces;
	int txtbsy;	/* kludge cntr for ETXTBSY fix */

	txeacces = txe2big = 0;
	txtbsy = 0;
	cp = pathstr;	/* normal case -- search */
	if (any('/', name)) {
		cp = "";	/* sh: exec only cmd name as given */
	}
	do {
		cp = pcat(cp, name, tline, sizeof tline);
	retry:
		execv(tline, argv);
		switch (errno) {
		case ENOEXEC:
			argv[-1] = shellnam;
			argv[0] = tline;
			execv(shellnam, &argv[-1]);
			die(shellnam, "No shell!");
		case EACCES:
			txeacces++;	/* file there, missing x (probably) */
			break;
		case ENOMEM:
			die(name, "too large");
		case E2BIG:
			txe2big++;
			break;
		case ETXTBSY:
			if ((txtbsy =+ 10) > 60)
				die(name, "text busy");
			sleep(txtbsy);
			goto retry;
		}
	} while(cp);
	if (txe2big)
		die(name, "arg list too long");
	if (txeacces)
		die(name, "file not executable");
	die(name, "not found");
}

die(str1, str2)
char *str1, *str2;
{
	prs(str1);
	prs(": ");
	prs(str2);
	prs("\n");
	exit(1);
}

pcat(so1, so2, si, sz)
register char *so1, *so2;
char *si;
int sz;
{
	register char *s;

	s = si;
	while(*so1 != ':' && *so1 != '\0' && --sz) *s++ = *so1++;
	if(si != s && --sz > 0) *s++ = '/';
	while(*so2 && --sz > 0) *s++ = *so2++;
	if (--sz < 0) {
			*si = '\0';
			die(0, "Command line overflow");
	}
	else *s = '\0';
	return *so1 ? ++so1 : 0;
}

prs(s)
register char *s;
{

	if (s == 0)
		return;
	while(*s)
		putc(*s++);
}


putc(c)
{

	write(2, &c, 1);
}


any(c, s)
register char c, *s;
{

	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}

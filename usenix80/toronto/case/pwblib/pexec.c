#

/*
 * pexec - path search and execute a file
 */

#include "errnos.h"

char pathstr[128], shellnam[16];

pexec(name, argv)
char *name, *argv[];
{
	extern errno;
	register char *cp;
	char tline[48];
	char txe2big, txeacces;
	int txtbsy;	/* kludge cntr for ETXTBSY fix */
	int i;
	char *sharg[20];	/* to copy args for shell */

	pexinit();
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
			for (i = 1; argv[i] && i < 20; i++)
				sharg[i+1] = argv[i];
			sharg[i+1] = 0;
			sharg[1] = tline;
			sharg[0] = shellnam;
			execv(shellnam, sharg);
			die("No shell!");
		case EACCES:
			txeacces++;	/* file there, missing x (probably) */
			break;
		case ENOMEM:
			die("too large");
		case E2BIG:
			txe2big++;
			break;
		case ETXTBSY:
			if ((txtbsy =+ 10) > 60)
				die("text busy");
			sleep(txtbsy);
			goto retry;
		}
	} while(cp);
	if (txe2big)
		die("arg list too long");
	if (txeacces)
		die("file not executable");
	die("not found");
}

static die(str)
char *str;
{
	prs(str);
	prs("\n");
	exit(1);
}


static prs(s)
register char *s;
{

	if (s == 0)
		return;
	while(*s)
		putc(*s++);
}


static putc(c)
{

	write(2, &c, 1);
}


static any(c, s)
register char c, *s;
{

	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}


static pcat(so1, so2, si, sz)
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
			die("arg list too long");
	}
	else *s = '\0';
	return *so1 ? ++so1 : 0;
}



static copy(source, sink)
register char *source, *sink;
{
	 while(*sink++ = *source++);
}

/*
 *	pexinit: fills in pathstr and shellnam.
 *	may be invoked before fork to avoid unnecessary .path opening.
 *	returns 0 if OK, -1 if any error.
 */
pexinit()
{
	char pathbuf[128 + 16];
	register n, f;
	char *newpath, *newshell;
	char *p;
	extern char *logdir();

	if (pathstr[0] != 0 || shellnam[0] != 0) return(0);
	newshell = "/bin/sh";
	pcat(logdir(), ".path", pathbuf, sizeof pathbuf);
	if ((f = open(pathbuf, 0)) < 0)
		newpath = getuid() & 0377 ? ":/bin:/usr/bin" : ":/bin:/usr/bin:/etc:/";
	else {
		n = read(f, pathbuf, sizeof pathbuf);
		close(f);
		if (n <= 0) {
			prs("cannot read .path"); prs("\n");
			return(-1);
		}
		if (pexline(pathbuf, pathbuf + n, 128, &newpath, &p)
		|| pexline(p, pathbuf + n, 16, &newshell, &p))
			return(-1);
	}
	copy(newpath, pathstr);
	copy(newshell, shellnam);
	return(0);
}

/*
 *	pexline: scan for a line (if any) beginning at ptr,
 *	ending at ptrlim -1, for line up to psize bytes long.
 *	convert it to string, return beginning addr in pret
 *	and addr of next line in pnext
 *	return 0 if OK (or not present), -1 if runs off end in middle of line
 *	or if line too long.
 */
static pexline(ptr, ptrlim, psize, pret, pnext)
register char *ptr, *ptrlim;
int psize;
char **pret, **pnext;
{
	if (ptr >= ptrlim)
		return(0);
	*pret = ptr;
	if (ptrlim > ptr + psize)
		ptrlim = ptr + psize;
	for (; ptr < ptrlim; ptr++)
		if (*ptr == '\n') {
			*ptr++ = '\0';
			*pnext = ptr;
			return(0);
		}
	prs(".path too long"); prs("\n");
	return(-1);
}

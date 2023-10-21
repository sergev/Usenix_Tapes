#include "hd.h"
#include "mydir.h"
#include "strings.h"
#include "classify.h"
#include <pwd.h>

char *getuser();

/* File gets a file name, converts it into a full path name, and
   selects that file.  */

file (argv) char **argv; 
{	/* Select specific file */
    register char *msg;
    char nname [STRMAX];

    msg = 0;
    if (*argv == CNULL)
	/* Flag path searching */
	msg = "~File: ";
    if (getfname (*argv, nname, msg) == BAD) return NOREPLOT;

    return (enterfile (nname));
}

/* Create accepts a file name, creats that file, and then enters it.  */

#define	CRTEXT	'1'
#define	CRDIR	'2'
#define	CRCOPY	'3'
#define	CRLINK	'4'
#define	CRMOVE	'5'

static char *cr_msgs[3] = {
	";\rCopy from:", ";\rLink from:", ";\rRename from:"
};

static int force = 0;

create () 
{

    char nnamebuf [STRMAX], oname [STRMAX];
    register char ch, *cp;
    register int class;
    char *nname;

    nname = nnamebuf;
    if (getfname (CNULL, nname, "!Create: ") == BAD) return NOREPLOT;

    if (!access (nname, 0)) 
    {
	putmsg (nname); printf (": Already exists");
	if (force) {
		printf(", creating anyways");
		sleep(1);
	}
	else {
		printf(", !filename to overwrite");
		return NOREPLOT;
	}
    }
    else
	force = 0;

    /* Can we write-access the directory ? */
    strcpy(oname, nname);
    todotdot (oname);
    class = classify (oname);

    if (class == CL_NULL || access (oname, 3)) 
    {
	myperror (oname); return NOREPLOT;
    }
    else if (class != CL_DIR) 
    {
	putmsg (oname); printf (": Not a directory");
	return NOREPLOT;
    }

    erase ();
    hilite ("Creating");

    printf (" %s\r\n\n1  Text\r\n2  Directory\r\n3  Copy a file\r\n4  Link a file\r\n5  Rename a file or directory\r\n\n", nname);
    hilite ("Select:");

    ch = getch (); putch (CR); putch (LF);
    erase();
    if (ch == CRTEXT) 
    {
	f_exec (EDITOR, EDITOR, nname, 0);
    }
    else if (ch == CRDIR) 
    {
	if (f_exec ("/bin/mkdir", "mkdir", nname, 0))
		getrtn ();
	else {
		hilite("Directory created. Enter new directory?");
		if (getch() == 'y')
			return enterdir (nname) | REPLOT;
		else
			return REPLOT;
	}
    }
    else if (ch == CRCOPY || ch == CRLINK || ch == CRMOVE) 
    {
	if (getfname (CNULL, oname, cr_msgs[ch-CRCOPY]) == BAD) return REPLOT;

	switch (classify (oname)) 
	{

	  case CL_NULL: 
	  case CL_PROTPLN:
	    myperror (oname); break;

	  case CL_DIR:
	    if (ch != CRMOVE) {
		hilite ("Cannot copy or link to directories");
		break;
	    }

	  default:
	    if (ch == CRCOPY)
		f_exec ("/bin/cp", "cp", oname, nname, 0);
	    else if (ch == CRMOVE)
		f_exec ("/bin/mv", "mv", oname, nname, 0);
	    else if (ch == CRLINK) 
	    {
		hilite ("Linking\r\n");
		if (link (oname, nname)) {
			myperror ("Link failed");
			break;
		}
	    }
/*
	    at (1501);
 */
	    if (!access (nname, 0)) 
	    {
		hilite("\r\n\nFile created. Do you wish to examine it?");
		if (getch() == 'y') 
		{
		    putch (CR);
		    putch (LF);
		    return enterfile (nname) | REPLOT;
		}
		return REPLOT;
	    }
	}
/*
	at (1501);
 */
	hilite ("\r\n\nFile not created.");
	getrtn ();
    }
    return REPLOT;
}

/* Getfname takes two character arrays as parameters.
   Inname is the partial pathname of a file.  If inname == CNULL,
   getfname will instead read the partial pathname from the terminal.
   The full pathname is returned in outname.
   Getfname has a return value of GOOD or BAD.
*/
getfname (inname, outname, msg) register char *inname, *outname, *msg; 
{
    char inword [STRMAX]; int ilen;
    int ffind;
    static oldfile[STRMAX];

    force = 0;
    ffind = 0;
    if (msg && *msg == '~') {
	msg++;
	ffind++;
    }
    if (inname == CNULL) 
    {
	tty_push (COOKEDMODE);
	if (msg && *msg == '!') {
		msg++;
		force++;
	}
	if (msg) {
		if (*msg == ';')
			hilite(++msg);
		else
			putmsg(msg);
	}
	ilen = getword (inword);
	tty_pop ();
	if (ilen <= 0) 
	{
	    clearmsg (0);
	    return BAD;
	}
	inname = inword;
	if (*inname == '!' && force) {
		inname++;
		ilen--;
		if (*inname == 0) {
			putmsg("No file name");
			return BAD;
		}
	}
	else
		force = 0;
	if (strcmp("!", inname) == 0)
		strcpy(inname, oldfile);
	if (inname[ilen-1] == '$') {
		if (*selectname == 0) {
			putmsg(" Missing argument");
			return BAD;
		}
		strcpy(inname+ilen-1, selectname);
		putmsg("%s%s", msg, inname);
	}
    }

    /*
     * Search for file amongst specified directories
     */
    if (ffind
	&& inname[0] != '/'
	&& (inname[0] != '.' || inname[1] != '/')
	&& srchfile(inname, outname) == GOOD) {
	strcpy(oldfile, outname);
	return GOOD;
    }
    if (pathgen(wdname, inname, outname)) {
	putmsg ("Path name too long");
	return BAD;
    }
    strcpy(oldfile, outname);
    return GOOD;
}

srchfile(inname, outname)
char *inname, *outname;
{
	register char *s, *t;
	char pbuf[STRMAX];

	s = ENTERPATH;
	while (*s) {
		if (*s == ':') {
			s++;
			t = wdname;
		}
		else {
			t = s;
			while (*s)
				if (*s++ == ':') {
					if (*s)
						s[-1] = 0;
					else
						*--s = 0;
					break;
				}
		}
		strcpy(pbuf, t);
		strcat(pbuf, SLASH);
		strcat(pbuf, inname);
		if (pathgen(wdname, pbuf, outname) == 0
			&& classify(outname) != CL_NULL) {
			if (*s)
				s[-1] = ':';
			return GOOD;
		}
		if (*s)
			s[-1] = ':';
	}
	return BAD;
}

/* If one is in dir with pathname "old", and does a chdir "change",
   one ends up in directory "new".  Exception: ".." always breaks
   through to the root.
*/
pathgen (old, change, new) char *old, *change, *new; 
{

#ifndef	NDIR
    char element [DIRSIZ + 1];
#else
    char element [MAXNAMLEN + 1];
#endif
    char chgbuf  [STRMAX];
    register char *s;
    register len;

    /* Match a user's login directory, or HOME */
    if (getuser(change, chgbuf) == 0)
	strcpy (chgbuf, change);
    s = chgbuf;
    if (*s == '/')
	strcpy(new, SLASH);
    else
	strcpy(new, old);

    while (*s) 
    {
	extract (element, s);
	if (compe (DOT, element));
	else if (compe (DOTDOT, element))
	    todotdot (new);
	else
	{
	    len = strlen (new);
	    if (len > LPLEN) return 1;
	    else if (len > 1) strcat (new, SLASH);
	    strcat (new, element);
	}
    }
    return 0;
}

extract (element, path) char *element, *path; 
{

    register char *cp;
    int eltlen;

    for (cp = path; *cp != 0 && *cp != '/'; cp++);

    eltlen = cp - path;
    if (eltlen == 0) 
    {
	strcpy (element, DOT);
    }
    else
    {
#ifndef	NDIR
	strncpy (element, path, DIRSIZ);
	element [min (eltlen, DIRSIZ)] = 0;
#else
	strncpy (element, path, MAXNAMLEN);
	element [min (eltlen, MAXNAMLEN)] = 0;
#endif
    }
    if (*cp) shift (path, eltlen + 1);
    else path [0] = 0;
}

shift (path, length) char *path; int length; 
{

    register char *cp;

    for (cp = path + length; cp [-1];) *path++ = *cp++;
}
	
/*
 * getuser(spec, pbuf)
 */
char *getuser(spec, pbuf)
char *spec, *pbuf;
{
	register char *r, *s, *t;
	struct passwd *p;
	struct passwd *getpwnam();

	s = spec;
	if (*s++ != '~')
		return 0;
	t = pbuf;
	while (*t = *s) {
		if (*s++ == '/') {
			*t = 0;
			break;
		}
		t++;
	}
	if (t == pbuf)
		t = HOME;
	else if (p = getpwnam(pbuf))
		t = p->pw_dir;
	else
		t = spec;
	r = pbuf;
	while (*r++ = *t++)
		if (r >= pbuf+LPLEN)
			return 0;
	if (*s) {
		r[-1] = '/';
		while (*r++ = *s++)
			if (r >= pbuf+STRMAX)
				return 0;
	}
	return pbuf;
}

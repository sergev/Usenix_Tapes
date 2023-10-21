#ifndef lint
static char rcsid[] = "$Header: /usr/src/local/bin/RCS/newacct.c,v 1.5 85/09/02 18:23:24 bin Exp $";
#endif

/*
 * New Account Request program
 *
 * Version 3.0, 2 Sep 1985, Chris Torek.
 *
 * Allows one to fill in an "entry form" for a new account.
 *
 % cc -O -o newacct newacct.c -lwinlib -ltermlib
 */

#include <stdio.h>
#include <local/window.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <ctype.h>

/*
 * conffile contains configuration info in the form "keyword=[ \t]*value\n".
 * It can override all the other strings given below.
 */
char	conffile[] = "/usr/adm/newacct.conf";

char	mailcommand[200] = "/usr/ucb/mail -s 'new account' account-master";
char	homedir[50] = "/u";
char	defaultgroup[20] = "misc2";
char	defaultmachine[100];
char	machinenames[300];
char	dictionary[100] = "/usr/dict/words";
char	localdict[100] = "/usr/dict/local";
char	badpasswds[100] = "/usr/adm/badpasswds";

struct conf {
    char  *c_name;
    char  *c_value;
    int    c_size;
} confitems[] = {
#define ENTRY(n,s) n, s, sizeof s
    ENTRY ("mailcmd",		mailcommand),
    ENTRY ("home",		homedir),
    ENTRY ("group",		defaultgroup),
    ENTRY ("defaultmachine",	defaultmachine),
    ENTRY ("machines",		machinenames),
    ENTRY ("dictionary",	dictionary),
    ENTRY ("localdict",		localdict),
    ENTRY ("badpasswds",	badpasswds),
    0, 0, 0
#undef ENTRY
};

#define	ROWS		15	/* Number of rows required */
#define	COLS		76	/* Number of columns required */

int     Uid,
        Gid,
        Which;
Win    *MainWin,
       *ErrorWin;
char   *Machines[30];

extern int   errno;

int     VerifyLogin (), VerifyGroup (), VerifyPassword ();
int     VerifyMachine (), VerifyOK ();

#define	NFIELDS	(sizeof Fields / sizeof *Fields)

struct entry {
    char   *e_descr;		/* Description */
    char   *e_space;		/* Ptr to space for entry */
    int     e_size;		/* Size of entry (# chars) */
    int     e_rsize;		/* Real size (for passwd, which grows) */
    int     e_noecho;		/* True if should blank-type (for passwd) */
    int   (*e_verify) ();	/* Verification function: true => good */
} Fields[] = {
#define X_LOGIN		0
	"Login name:",		0,  8,  8, 0, VerifyLogin,
#define	X_FULLNAME	1
	"Full name:",		0, 20, 20, 0, VerifyOK,
#define	X_GROUP		2
	"Group:",		0,  8,  8, 0, VerifyGroup,
#define	X_PASSWD	3
	"Password:",		0,  8, 14, 1, VerifyPassword,
#define	X_OFFICE	4
	"Office:",		0, 10, 10, 0, VerifyOK,
#define	X_OPHONE	5
	"Office phone:",	0, 12, 12, 0, VerifyOK,
#define	X_HPHONE	6
	"Home phone:",		0, 12, 12, 0, VerifyOK,
#define	X_OTHER		7
	"Other info:",		0, 20, 20, 0, VerifyOK,
#define	X_FUND		8
	"Account:",		0, 20, 20, 0, VerifyOK,
#define	X_INCHARGE	9
	"Person in charge:",	0, 20, 20, 0, VerifyOK,
#define	X_MACHINE	10
	"Machine name:",	0, 10, 10, 0, VerifyMachine,
};

FILE *popen ();
struct passwd *getpwent ();
struct group *getgrnam ();
char *malloc (), *index (), *strcpy ();

#define	CTRL(c)		((c) & 0x1f)
#define	move(y,x)	WAcursor(MainWin, y, x)
#define	refresh()	(WRCurRow=MainWin->w_cursor.row, \
			 WRCurCol=MainWin->w_cursor.col, Wrefresh(0))

main (argc, argv)
int argc;
char **argv;
{
    register int    c,
                    i;
    register struct entry *ep;
    int     rrows,
            rcols;

    if (argc > 1)
	error (1, 0, "usage: %s\n", *argv);

    Configure ();
    i = strlen (homedir);
    if (i == 0 || homedir[i - 1] != '/') {
	homedir[i] = '/';
	homedir[i + 1] = 0;
    }
    MakeMachines ();
    if (Winit (0, 0)) {
	printf ("Sorry, your terminal won't run this program.\n");
	printf ("Here is your free consolation fortune:\n\n");
	fflush (stdout);
	execl ("/usr/games/fortune", "fortune", (char *)0);
	execl ("/usr/local/bin/fortune", "fortune", (char *)0);
	error (1, errno, "So much for \"fortune\"");
    }
    Wscreensize (&rrows, &rcols);
    if (rcols < COLS || rrows < ROWS) {
	Wcleanup ();
	printf ("Sorry, but your screen is too small.\n");
	exit (1);
    }
    if ((MainWin = Wopen (0, 0, 0, COLS, ROWS, 0, 0)) == 0) {
	Wcleanup ();
	printf ("Something is wrong (window open failed)\n");
	printf ("Contact the system administrator\n");
	exit (1);
    }
    Woncursor (MainWin, 0);
    ep = Fields;
    for (i = 0; i < NFIELDS; i++) {
	register char *p = malloc (ep -> e_rsize + 1);

	if (p == 0)
	    error (1, errno, "malloc failed - consult newacct guru");
	*p = 0;
	ep -> e_space = p;
	ep++;
    }
    WSetRealCursor++;
    center ("Please fill in this form");
    ep = Fields;
    for (i = 0; i < NFIELDS; i++) {
	move (i / 2 + 2, (i & 1) ? COLS / 2 : 0);
	Wputs (ep -> e_descr, MainWin);
	ep++;
    }
    move (8, 0);
    center ("^L Redraw screen        ^D Done, send message    ");
    move (9, 0);
    center ("^M Move to next entry   ^U Move to previous entry");
    move (10, 0);
    center ("Type to change, use ESC to cancel change");
    move (13, 0);
    center ("If you don't know what to enter for something, leave it blank");
    strcpy (Fields[X_GROUP].e_space, defaultgroup);
    Which = X_GROUP;
    sel (0);
    strcpy (Fields[X_MACHINE].e_space, defaultmachine);
    Which = X_MACHINE;
    sel (0);
    Which = 0;
    sel (1);
    for (;;) {
	refresh ();
	c = getchar ();
	if (ErrorWin)
	    Whide (ErrorWin);
	switch (c) {
	    case CTRL ('l'): 
		ScreenGarbaged++;
		break;
	    case '\r': 
	    case '\n': 
	    case CTRL ('n'): 
		sel (0);
		if (++Which >= NFIELDS)
		    Which = 0;
		sel (1);
		break;
	    case CTRL ('u'): 
	    case '\b': 
		sel (0);
		if (--Which < 0)
		    Which = NFIELDS - 1;
		sel (1);
		break;
	    case CTRL ('d'): 
	    case EOF: 
		save ();
		exit (0);
	    default: 
		if (c < ' ')
		    gripe ("That key means nothing");
		else {
		    ChangeIt (c);
		    sel (1);
		}
		break;
	}
    }
}

/*
 * Center a string on the screen
 */
center (s)
register char *s;
{
    register int    nb;
    register char  *p = s;

    while (*p++);
    p--;
    nb = (COLS - (p - s)) / 2;
    while (--nb >= 0)
	Wputc (' ', MainWin);
    Wputs (s, MainWin);
}

/*
 * Change one of the strings (the variable Which tells which one)
 */
ChangeIt (firstc)
int firstc;
{
    register int    c,
                    inlen = 0;
    register char  *s = Fields[Which].e_space;
    char    instr[81],
            remember[81];
    int     noecho,
            maxlen,
            legal;

    strcpy (remember, s);
    *s = 0;
    sel (1);
    Wsetmode (MainWin, WINVERSE);
    maxlen = Fields[Which].e_size;
    noecho = Fields[Which].e_noecho;
    for (c = firstc;; c = getchar ()) {
	if (ErrorWin)
	    Whide (ErrorWin);
	if (c == '\r' || c == '\n' || c == CTRL ('u') || c == EOF
		|| c == CTRL ('d')) {
	    strcpy (s, inlen ? instr : remember);
	    legal = (*Fields[Which].e_verify) (s);
	    if (c != EOF)
		ungetc (c, stdin);
	    if (legal)
		return 1;
	    if (c != EOF)
		c = getchar ();	/* un-ungetc */
	    strcpy (s, remember);
	}
	else if (c == 033) {
	    strcpy (s, remember);
	    return 0;
	}
	else if (c == '\b') {
	    if (inlen) {
		Wputs ("\b \b", MainWin);
		instr[--inlen] = 0;
	    }
	    else
		gripe ("Can't backspace - at left margin");
	}
	else if (c >= ' ' && c <= 0177 && c != ':' && c != ',' && c != ';') {
	    if (inlen >= maxlen)
		gripe ("Can't enter any more characters");
	    else {
		Wputc (noecho ? ' ' : c, MainWin);
		instr[inlen++] = c;
		instr[inlen] = 0;
	    }
	}
	else if (c == CTRL ('l'))
	    ScreenGarbaged++;
	refresh ();
    }
}

/*
 * Mark or unmark the current string with inverse video
 */
sel (inv)
int inv;
{
    register char  *s = Fields[Which].e_space;
    register int    bl,
                    bl2;
    int     y,
            x;

    y = Which / 2 + 2;
    x = Which % 2 ? COLS / 2 + 18 : 18;
    move (y, x);
    Wsetmode (MainWin, inv ? WINVERSE : 0);
    bl = Fields[Which].e_rsize - strlen (s);
    bl2 = Fields[Which].e_rsize - Fields[Which].e_size;
    if (bl < 0)
	bl2 += bl;
    Wputs (s, MainWin);
    while (--bl >= 0)
	Wputc (' ', MainWin);
    Wsetmode (MainWin, 0);
    while (--bl2 >= 0)
	Wputc (' ', MainWin);
    move (y, x);
}

/*
 * Save the results of the session.  Actually, send mail to someone.
 */
save () {
    register struct passwd *pw;
    register struct group  *g;
    register int    i = 1;
    register char  *s;
    char    remarks[1024],
            passwdentry[300];

    Wcleanup ();

 /* Find a unique userid and the group id */
    setpwent ();
    while ((pw = getpwent ()) != NULL) {
	if (pw -> pw_uid >= i)
	    i = pw -> pw_uid + 1;
    }
    setgrent ();
    g = getgrnam (Fields[X_GROUP].e_space);

 /* Make an entry for the passwd file */
    sprintf (passwdentry,
	    "%s:%s:%d:%d:%s,%s,%s,%s,%s:%s%s:/bin/csh",
	    Fields[X_LOGIN].e_space,
	    Fields[X_PASSWD].e_space,
	    i,
	    g ? g -> gr_gid : 0,
	    Fields[X_FULLNAME].e_space,
	    Fields[X_OFFICE].e_space,
	    Fields[X_OPHONE].e_space,
	    Fields[X_HPHONE].e_space,
	    Fields[X_OTHER].e_space,
	    homedir,
	    Fields[X_LOGIN].e_space);
    printf ("\
Oh, one other thing: please type in any additional\n\
information that could be helpful (e.g., why you are\n\
requesting this account), then enter a line consisting\n\
of a \".\" (period) on a line by itself.\n");
    fflush (stdout);
    s = remarks;
    while (fgets (s, sizeof remarks - (s - remarks), stdin)) {
	if (strcmp (s, ".\n") == 0)
	    break;
	s += strlen (s);
    }
    *s = 0;
    if ((i = fork ()) == 0) {	/* Child, send mail */
	register FILE *f = popen (mailcommand, "w");

	if (f == 0)
	    error (1, errno, "popen (\"%s\", \"w\") failed", mailcommand);
	fprintf (f, "\
%s Account Request:\n\
%s\n\
(group is %s, account %s, contact is %s)\n\
Remarks: %s\n",
		Fields[X_MACHINE].e_space,
		passwdentry,
		Fields[X_GROUP].e_space,
		Fields[X_FUND].e_space,
		Fields[X_INCHARGE].e_space,
		remarks);
	pclose (f);
	_exit (0);
    }
    if (i <= 0)
	error (1, errno, "fork");
    printf ("\nYour account will be ready in a few days.\n");
}

/*
 * Field verification stuff
 */

VerifyOK () {
    return 1;			/* call it OK */
}

/*
 * We may only bother checking name-in-use for local machine names.
 * This is a mistake if you are using several machines as a single
 * distributed system.  If you really don't like this, turn the
 * disabled code back on.
 */
VerifyLogin () {
    register int    i;
#ifdef notdef
    static char thishost[sizeof defaultmachine];

    if (thishost[0] == 0)
	(void) gethostname (thishost, sizeof thishost);
    if (strcmp (Fields[X_MACHINE].e_space, thishost))
	return 1;
#endif
    i = getpwnam (Fields[X_LOGIN].e_space) == 0;
    if (i == 0)
	gripe ("That name is in use");
    return i;
}

VerifyGroup () {
    register int    i = getgrnam (Fields[X_GROUP].e_space) != 0;

    if (i == 0)
	gripe ("There is no such group");
    return i;
}

VerifyMachine (p)
register char *p;
{
    register char **l = Machines;
    register char  *s;
    char    buf[BUFSIZ];

    while ((s = *l++) != 0)
	if (*s == *p && strcmp (s, p) == 0)
	    return 1;

    strcpy (buf, "No such machine.  Try one of:");
    p = buf + strlen (buf);
    l = Machines;
    while ((s = *l++) != 0) {
	*p++ = ' ';
	while ((*p = *s++) != 0)
	    p++;
    }
    *p++ = '.';
    *p = 0;
    gripe (buf);
    return 0;
}

VerifyPassword (p)
register char *p;
{
    register int    i,
                    c;
    register char  *np,
		   *sp;
    long    salt;
    char    saltc[2];
    static char  buf[BUFSIZ];

    if (!*p) {
	gripe ("You must specify a password");
	return 0;
    }
    if (strlen (p) < 6) {
	gripe ("Must be at least 6 characters");
	return 0;
    }
    if (alldigits (p)) {
	gripe ("Must have at least one non-digit");
	return 0;
    }
 /* Try login name */
    if (try (Fields[X_LOGIN].e_space, p)) {
tooeasy:
	gripe ("Too easy to guess");
	return 0;
    }

 /* Try full name, all pieces */
    strcpy (buf, Fields[X_FULLNAME].e_space);
    for (np = buf; np && *np;) {
	if ((sp = index (np, ' ')) != 0)
	    *sp = 0;
	if (try (np, p))
	    goto tooeasy;
	np = sp ? sp + 1 : 0;
    }

 /* Try word lists */
    if (lookup (p, dictionary) || lookup (p, localdict) ||
	    lookup (p, badpasswds))
	goto tooeasy;

    time (&salt);
    salt += getpid ();
    saltc[0] = salt & 077;
    saltc[1] = (salt >> 6) & 077;
    for (i = 0; i < 2; i++) {
	c = saltc[i] + '.';
	if (c >= '9')
	    c += 7;
	if (c > 'Z')
	    c += 6;
	saltc[i] = c;
    }
    strcpy (p, crypt (p, saltc));
}

alldigits (p)
register char *p;
{
    while (isdigit (*p++));
    return (*p == 0);
}

try (guess, pwd)
register char *guess;
register char *pwd;
{
    register int    c1,
                    c2;

    while (c1 = *guess++) {
	c2 = *pwd++;
	if (isupper (c1))
	    c1 = tolower (c1);
	if (isupper (c2))
	    c2 = tolower (c2);
	if (c1 != c2)
	    return 0;
    }
    return *pwd == 0;
}

/*
 * Gripe about something the user did.
 */
gripe (s)
char *s;
{
    if (!ErrorWin) {
	ErrorWin = Wopen (1, 0, 11, COLS, 1, 0, 0);
	Woncursor (ErrorWin, 0);
	Wwrap (ErrorWin, 0);
    }
    else
	Wunhide (ErrorWin);
    Wsetmode (ErrorWin, 0);
    WAcursor (ErrorWin, 0, 0);
    Wclear (ErrorWin, 0);
    Wsetmode (ErrorWin, WINVERSE);
    Wputs (s, ErrorWin);
    Ding ();
}

/*
 * Read the configuration file.
 */
Configure () {
    register FILE *cf;
    register char *p;
    register struct conf *c;
    register char *val;
    int lineno;
    char buf[BUFSIZ];

 /* first set up the default machine name */
    (void) gethostname (defaultmachine, sizeof defaultmachine);
    if ((cf = fopen (conffile, "r")) == 0)
	return;
    lineno = 0;
    while (fgets (buf, sizeof buf, cf)) {
	lineno++;
	if (buf[0] == '#')	/* comment */
	    continue;
	if ((p = index (buf, '\n')) != 0)
	    *p = 0;
	if (buf[0] == 0)	/* ignore blank lines */
	    continue;
	if ((p = index (buf, '=')) == 0) {
	    p = "malformed line";
badconf:
	    error (0, 0, "\"%s\", line %d: bad conf file (%s)", conffile,
		    lineno, p);
	    error (1, 0, "consult newacct guru");
	    /*NOTREACHED*/
	}
	*p = 0;
	val = p + 1;
	while (isspace (*val))
	    val++;
	for (c = confitems; c -> c_name; c++)
	    if (*c -> c_name == *buf && strcmp (c -> c_name, buf) == 0)
		goto found;
	*p = '=';
	p = "unrecognized keyword";
	goto badconf;
found:
	if (strlen (val) >= c -> c_size) {
	    *p = '=';
	    p = "value too long";
	    goto badconf;
	}
	(void) strcpy (c -> c_value, val);
    }
    (void) fclose (cf);
}

/*
 * Turn the whitespace-separated list of machine names into a list
 * of pointers.
 */
MakeMachines () {
    register char *p, **l;

    l = Machines;
    p = machinenames;
    while (*p) {
	*l++ = p;
	while (*p) {		/* while there's more of this name */
	    if (isspace (*p)) {
		*p++ = 0;
		while (isspace (*p))
		    p++;
		break;		/* end there's more of this name */
	    }
	    p++;
	}
    }
    *l = 0;
}

FILE   *wordfile;

/*
 * Perform a case independent binary search for target in the (sorted)
 * word list in file filenam.
 */
lookup (target, filenam)
char *target, *filenam; {
    register int    c;
    long    high,
            low,
            mid;
    char    key[10],
	    word[80];
#define CMP(s,t) ((*s) == (*t) ? strcmp (s, t) : (*s) - (*t))

    if (wordfile != NULL)
	fclose (wordfile);
    if ((wordfile = fopen (filenam, "r")) == NULL)
	return 0;
    strcpy (key, target);
    lower (key);
    low = 0;
    fseek (wordfile, 0L, 2);
    high = ftell (wordfile);
    for (;;) {
	mid = (high + low) >> 1;
	fseek (wordfile, mid, 0);
	do {
	    mid++;
	    c = getc (wordfile);
	} while (c != EOF && c != '\n');
	if (!getword (word))
	    break;
	c = CMP (key, word);
	if (c == 0)	/* found it */
	    return 1;
	if (c < 0) {	/* too far */
	    if (high == mid)
		break;	/* stop spinning the wheels */
	    high = mid;
	}
	else		/* not far enough */
	    low = mid;
    }
 /* at this point we've narrowed the range as much as we can; now
    search until either we find the word, or we go past the key. */
    fseek (wordfile, low, 0);
    for (;;) {
	if (!getword (word))
	    return (0);
	c = CMP (key, word);
	if (c < 0)
	    return 0;
	if (c == 0)
	    return 1;
    }
#undef CMP
}

/*
 * Read a word and lowercasify it.
 */
getword (w)
char *w; {
    register char  *p = w;
    register int    c;

    while ((c = getc (wordfile)) != '\n') {
	if (c == EOF)
	    if (p == w)
		return 0;
	    else
		break;
	*p++ = c;
    }
    *p = 0;
    lower (w);
    return 1;
}

/*
 * Lowercasify a word.
 */
lower (s)
register char *s; {
    register int c;

    while ((c = *s) != 0) {
	if (isupper (c))
	    *s = tolower (c);
	s++;
    }
}

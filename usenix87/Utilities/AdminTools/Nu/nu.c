/* New User Installation Program
 *
 *	Brian Reid, Erik Hedberg, Jeff Mogul, and Fred Yankowski
 *	Stanford University
 *
 *	This program helps system administrators manage login accounts.
 *	it reflects the way we do things at Stanford, but can probably
 *	be adapted to almost any situation.

 *	It was originally written by Fred Yankowski as an MS project in 1980.
 *	Several years of experience at using it showed us lots of things that
 *	we would like it to do differently; Erik Hedberg added many changes.
 *	In summer 1984 Brian Reid tore it all apart, adapted it to 4.2BSD,
 *	modified it to use the C library as much as possible (this was Fred's
 *	first attempt at a C program), added error checking and command-line
 *	parsing, rewrote code that didn't amuse him, and changed the
 *	structure of it so that it read in a configuration file instead of
 *	having compile-time options. Jeff Mogul added the -d option.
 *	The man page is entirely by Brian Reid.
 *	
 *	This program does not contain any Unix source code, nor is it copied
 *	from any. It is completely public domain, and you are free to use,
 *	distribute, modify, or sell it, make T-shirts out of it, or do
 *	whatever you want.

 *
 */

#define CONFIGFILE "/etc/nu.cf"		/* configuration info from here */
#define	ALIASES	   "/usr/lib/aliases"	/* sendmail alias file */
#define MAXSYMBOLS 100			/* limit of configuration symbols */
#define MAXGROUPS  100			/* limit of GroupHome symbols */

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <grp.h>
#include <signal.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CR		'\n'
#define BUF_LINE	250		/* line length buffer */
#define BUF_WORD	40		/* word length buffer */
#define TRUE		1		/* return(...) codes */
#define FALSE		0
#define	ERROR		1		/* exit(...) codes */
#define	OK		0

 /* DoCommand codes ... */
#define	FATAL		1		/* Errors during System calls exit */
#define	NONFATAL	0		/* Errors just print message */
#define SAFE		1		/* Safe to execute during debugging */
#define	UNSAFE		0		/* Must not execute while debugging */

#define NOT		!
#define MAXUID		99999		/* assumed to be higher than any
					   userid */
#define MAXMODS		50		/* max. modifications in session */

char   *ctime (), *getlogin (), *crypt (), *StrV ();
int     IntV ();
struct passwd  *getpwent (), *getpwuid (), *getpwnam ();
char   *getpass ();
struct group   *getgrgid (), *getgrnam ();

struct passwd  *pwd;			/* ptr to an entry of the passwd file 
					*/

/* This is a copy of the passwd structure, values are copied into this
    structure because getpw--- returns a pointer to a STATIC area that is
    overwritten on each call. */
struct cpasswd {
    char    cpw_name[BUF_WORD];
    char    cpw_passwd[BUF_WORD];
    char    cpw_asciipw[BUF_WORD];	/* Extra field: Ascii password */
    int     cpw_uid;
    int     cpw_gid;
    char    cpw_group[BUF_WORD];	/* Extra field: Ascii group name */
    char    cpw_person[BUF_WORD];
    char    cpw_dir[BUF_WORD];
    char    cpw_linkdir[BUF_WORD];	/* Extra field: top-level link dir */
    char    cpw_shell[BUF_WORD];
};
/* This structure is used to hold the configuration statements from nu.cf
   Note that no error checking of any kind is done! */

struct Symb {
    char   *SymbName;			/* Variable name goes here */
    char   *Svalue;			/* String argument goes here OR */
    long    ivalue;			/* Integer argument goes here */
};
struct Symb Symbols[MAXSYMBOLS];
long    nsymbols = 0;

/* Definitions for using dbm (to access alias database) */
typedef struct {
    char   *dptr;
    int     dsize;
}               datum;

int     dbminit ();
datum fetch (), firstkey (), nextkey ();
#define	dbm_buf_size 1024		/* dbm(3) guarantees this limit */

/* End of dbm declarations */

char    This_host[100];			/* Holds result of gethostname() */

struct topnode {
    int     gid;			/* login group number */
    char   *topnodename;		/* what comes after /usr */
};

int     numtopnodes = 0;
struct topnode  topnode[MAXGROUPS];	/* to be filled in from nu.cf */

int     incritsect;			/* Set to true when the program is in
					   the section involved with updating
					   the files and creating new files
					   and, hence, should not be
					   interruptable by ^C. */

int     wasinterrupted;			/* Set to true if interrupt occurs
					   during a call to 'System' */

char    cmd[BUF_LINE];			/* Buffer to construct commands in */

char    editor[BUF_WORD];		/* Who is running this program */

FILE * logf;				/* File to log actions in */
int     whichmail;			/* record which kind of mail the user
					   wants */
struct stat statbuf;			/* for stat and lstat calls */

ncpy (to, from, len)
char   *to, *from;
int     len;
{
    register int    i;
    for (i = 1; i <= len; i++)
	*to++ = *from++;
}

/* YesNo
 *	Gets either a yes or a no answer. "Yy\n" for yes, "Nn" for no.
 */
int     YesNo (defaultans)
char    defaultans;
{
    char    temp[BUF_WORD], ans;

    for (;;) {
	fgets (temp, BUF_WORD, stdin);
	MapLowerCase (temp);
	ans = temp[0];
	if (ans == '\n')
	    ans = defaultans;
	if (ans == 'y')
	    return (TRUE);
	if (ans == 'n')
	    return (FALSE);
	printf ("Answer y or n. [return means %c] ", defaultans);
    }
}

/* UseDefault
 *	returns TRUE if NULL entered (meaning use default) or
 *	FALSE after reading text to use in place of default
 */
UseDefault (str, def)
char   *str, *def;
{
    char    temp[BUF_WORD];
    printf (" [%s] ", def);
    gets (temp);
    if (temp[0] == NULL) {
	if (str != def)
	    strcpy (str, def);
	return (TRUE);
    }
    else {
	strcpy (str, temp);
	return (FALSE);
    }
}

/* GetUserID
 *	returns the userid for the new user.  The routine scans the
 *	password file to find the highest userid so far assigned.
 *	The new userid is then set to be the one more than this value.
 *	This calculated default can be overridden, but the password
 *	file is searched to insure that the chosen userid will be
 *	unique.
 */
GetUserID () {
    int     maxuid, newuid;
    char    defaultID[BUF_WORD], buf[BUF_WORD];

 /* scan the passwd file for the highest userid */
    setpwent ();			/* 'rewinds' passwd file search ptr */
    maxuid = 0;
    while ((pwd = getpwent ()) != NULL) {
	if (pwd->pw_uid > maxuid)
	    maxuid = pwd->pw_uid;
    }
    newuid = maxuid + 1;
    sprintf (defaultID, "%d", newuid);

    for (;;) {
	printf ("User id number? (small integer) ");
	UseDefault (buf, defaultID);
	newuid = atoi (buf);
	if (newuid <= 0) {
	    printf ("Userid must be >0, and should be >10.\n");
	    continue;
	}
	setpwent ();			/* rewind password file search */
	if ((pwd = getpwuid (newuid)) == NULL)
	    return (newuid);
	else
	    printf ("User id %d is already assigned to %s (%s)\n",
		    pwd->pw_uid, pwd->pw_name, pwd->pw_gecos);
    }
}

/* GetGroup
 *	writes the group name in grpname and returns the numeric gid.
 *	A groupid can be entered as a number or the name for a
 *	group (as defined in /etc/group).  The getgr-- functions are
 *	used to peruse the group file.  Legal symbolic names are
 *	mapped into the corresponding groupid.  Input numeric
 *	groupid's are mapped into the corresponding symbolic name
 *	(if such exists) for verification.
 */
GetGroupID (group)
char   *group;
{
    struct group   *agrp;
    int     gid;
    char    buf[BUF_WORD], def[BUF_WORD];

 /* Get group name for IntV("DefaultGroup") */
    setgrent ();
    if (agrp = getgrgid (IntV ("DefaultGroup")))
	strcpy (def, agrp->gr_name);
    else
	strcpy (def, "unknown");

    for (;;) {
	printf ("Which user group? (name or number; ? for list)");
	UseDefault (buf, def);
	if ((strcmp (buf, "?") == 0 || (strcmp (buf, "help") == 0))) {
	    char    hbuf[BUF_LINE], *hptr;
	    int     gcount;
	    setgrent ();
	    hptr = hbuf;
	    gcount = 0;
	    sprintf (hptr, "Available groups are:");
	    while ((agrp = getgrent ()) != 0) {
		if ((gcount++) == 7) {
		    strcat (hptr, "\n\t\t     ");
		    gcount = 0;
		}
		strcat (hptr, " ");
		strcat (hptr, agrp->gr_name);
	    }
	    endgrent ();
	    puts (hptr);
	    continue;
	}

	if (isdigit (*buf)) {
    /* presumably, a numeric groupid has been entered */
	    gid = atoi (buf);
	    setgrent ();
	    if (agrp = getgrgid (gid))
		strcpy (buf, agrp->gr_name);
	    else
		strcpy (buf, "unknown");
	    printf ("Selected groupid is %d (%s), OK? (y or n) [y]", gid, buf);
	    if (YesNo ('y')) {
		strcpy (group, buf);
		return (gid);
	    }
	}
	else {
    /* some symbolic group name has been entered */
	    setgrent ();
	    if (agrp = getgrnam (buf)) {
		strcpy (group, agrp->gr_name);
		return (agrp->gr_gid);
	    }
	    else
		printf ("Sorry, %s is not a registered group name.\nIf you want a list of groups, type '?'\n", buf);
	}
    }
}

/* MapLowerCase
 *	maps the given string into lower-case.
 */
MapLowerCase (b)
char   *b;
{
    while (*b) {if (isupper (*b)) *b++ += 'a'-'A'; else b++;}
}

HasBadChars (b)
char   *b;
{
    while (*b) {
	if ((NOT isalpha (*b)) && (NOT isdigit (*b)) && (*b != '_'))
	    return (TRUE);
	b++;
    }
    return (FALSE);
}

/* GetLoginName
 * 	Prompts for a login name and checks to make sure it is legal.
 *	The login name is returned, null-terminated, in "buf".
 */
GetLoginName (buf)
char   *buf;
{
    int     done = FALSE, i, j;
    char   *aptr, aname[dbm_buf_size];
    datum aliaskey, aliasname;

    while (NOT done) {
	printf ("Login name? (1-8 alphanumerics) [no default] ");
	gets (buf);
	if (buf[0] == NULL)
	    continue;

	MapLowerCase (buf);
	if (HasBadChars (buf)) {
	    printf ("Sorry, the login name can contain only alphanumerics or '_'\n");
	    continue;
	}

	if (strlen (buf) > IntV ("MaxNameLength")) {
	    printf ("Sorry, login names must not contain more than ");
	    printf ("%d characters\n", IntV ("MaxNameLength"));
	    buf[IntV ("MaxNameLength")] = 0;
	    printf ("Should it be truncated to '%s'? ", buf);
	    if (NOT YesNo ('y'))
		continue;		/* start over again */
	}

 /* check to see that the login is unique */
	setpwent ();
	if (pwd = getpwnam (buf)) {
	    printf ("Sorry, the login '%s' is already in use (id %d, %s).\n",
		    buf, pwd->pw_uid, pwd->pw_gecos);
	    continue;
	}
 /* check to make sure that the login does not conflict with an alias. This
    whole section of code is ridiculous overkill, but I was in the mood
    (BKR). */
	aliaskey.dptr = buf;
	aliaskey.dsize = strlen (buf) + 1;
					/* char count includes the null!! */
	aliasname = fetch (aliaskey);
	if (aliasname.dptr != NULL) {
	    printf ("Sorry, the name '%s' is already in use as a mail alias\n(aliased to '", buf);
	    if (aliasname.dsize > dbm_buf_size-2)
		aliasname.dsize = dbm_buf_size-2;

	    aptr = aliasname.dptr;
	    for (i = 0; (aptr[i] & 0177) == ' ' || (aptr[i] & 0177) == '\t'; i++);

	    if ((aptr[i] & 0177) == '"' && (aptr[aliasname.dsize-2] == '"')) {
		i++;			/* Unquote quoted names */
		aptr[aliasname.dsize-2] = 0;
		aliasname.dsize--;
	    }

	    for (j = 0; i < aliasname.dsize; i++, j++) {
		aname[j] = aptr[i] & 0177;
		if (i < 60 - strlen (buf))
		    putchar (aname[j]);
	    }
	    putchar ('\'');
	    aname[j] = 0;
	    if (aliasname.dsize >= 60 - strlen (buf))
		printf ("...");
	    puts (")");
	    printf ("\nBecause some mail aliases are critical to system operation, you must\n");
	    printf ("resolve that conflict before you can create an account named '%s'.\n\n", buf);

    /* 
     Try to figure out what the alias entry is. We do this because the person
     running nu is often an administrative assistant and not a programmer.
     */
	    if (aname[0] == '|') {	/* It's a program */
		printf ("The current alias is a program, automatically run whenever mail is sent\n");
		printf ("to '%s'. It is almost certainly a bad idea to delete this alias,\n",buf);
		printf ("but check with a systems programmer to be sure.\n");
	    }
	    else
		if (aname[0] == '/') {	/* A log file */
		    printf ("The current alias is a log file. All mail sent to '%s' is auto-\n", buf);
		    printf ("matically added to the end of %s. You can probably negotiate\n", aname);
		    printf ("with its owner to change from '%s' to a new name.\n", buf);
		}
		else {			/* alias or list */
		    if (index (aname, ',')) {/* mailing list */
			printf ("The current alias is a mailing list, enabling people to send mail\n");
			printf ("to '%s@%s' and have it reach a group of people. To know whether\n", buf, This_host);
			printf ("it is safe to delete or rename that list, you need to learn who uses it (the\n");
			printf ("users are not necessarily on this machine). Please investigate before you\nchange anything.\n");
		    }
		    else {		/* alias entry */
			if (index (aname, '@') ||
			    index (aname, '!')) {/* network alias */
			    printf ("The current definition is a network mail alias for a user who does not have\n");
			    printf ("a login on %s. When somebody sends mail to %s@%s,\nit is forwarded to %s.\n\n",
				    This_host, buf, This_host, aname);
			    printf ("However, if the %s login '%s' that you are currently trying\n", This_host, buf);
			    printf ("to create is in fact for %s, you probably want to leave this\nforwarding entry in place and create the account anyhow.\n\n",
				    aname);
			    printf ("Do you want to go ahead and create the account '%s', knowing that\nits mail will be forwarded to %s? (y or n) [y] ",
				    buf, aname);
			    if (YesNo ('y')) {
				done = TRUE;
				break;
			    }
			}
			else {		/* local alias */
			    printf ("The current definition is a local nickname or spelling correction\n");
			    printf ("for %s. It is most likely OK to delete the nickname, but there\n", aname);
			    printf ("might be people who are accustomed to sending mail to '%s' instead of\nto '%s'. Please investigate before you change anything.\n",
				    buf, aname);
			}
		    }
		}
	    puts ("\n");		/* puts will add a second newline */
	}
	else
	    done = TRUE;
    }
}

/* GetPassword
 *	read password (null is still allowed)
 */
char   *GetPassword (ascpw, cryptpw)
char   *ascpw, *cryptpw;
{
    char    saltc[2], c;
    long    salt;
    register int    i;
    char    pw1[40], pw2[40];

    do {
	strcpy (pw1, getpass ("Enter password: "));
	strcpy (pw2, getpass ("Retype password, please: "));
	if (!strcmp (pw1, pw2))
	    break;
	printf ("They don't match. Please try again.\n");
    } while (TRUE);

    strcpy (ascpw, pw1);
    if (strlen (ascpw)) {
	time (&salt);
	salt += getpid ();
	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i = 0; i < 2; i++) {
	    c = saltc[i] + '.';
	    if (c > '9')
		c += 7;
	    if (c > 'Z')
		c += 6;
	    saltc[i] = c;
	}
	strcpy (cryptpw, (crypt (ascpw, saltc)));
    }
    else
	strcpy (cryptpw, "");
}

/* GetRealName
 *	read the new user's actual name.
 */
GetRealName (buf)
char   *buf;
{
    int     done;
    done = FALSE;
    while (NOT done) {
	printf ("Enter actual user name: ");
	gets (buf);
	if (index (buf, ':'))
	    printf ("Sorry, the name must not contain ':'\n");
	else
	    done = TRUE;
    }
}

/* GetLoginDir
 *	Writes the new user's login directory in the cpw_dir field.
 */
int     GetLoginDir (np)
struct cpasswd *np;
{
    int     DontClobberDir;
    register int    i, done;
    char    defdir[BUF_WORD];		/* default login directory */

    strcpy (defdir, StrV ("DefaultHome"));
    strcat (defdir, "/");

    for (i = 0; topnode[i].gid; i++) {
	if (np->cpw_gid == topnode[i].gid) {
	    strcpy (defdir, topnode[i].topnodename);
	    strcat (defdir, "/");
	    break;
	}
    }
    strcat (defdir, np->cpw_name);

    done = FALSE;
    while (NOT done) {
	printf ("Login directory? ");
	UseDefault (np->cpw_dir, defdir);
	if (index (np->cpw_dir, ':')) {
	    printf ("Sorry, the name must not contain ':'\n");
	    continue;
	}
	DontClobberDir = 0;
	if (stat (np->cpw_dir, &statbuf) == 0) {
	    printf ("%s already exists. Do you want to clobber it? (y or n) [n] ", np->cpw_dir);
	    if (YesNo ('n'))
		break;
	    printf ("Do you want to use %s, but not touch its contents? (y or n) [y] ", np->cpw_dir);
	    if (NOT YesNo ('y'))
		continue;
	    DontClobberDir = 1;
	    break;
	}
	break;
    }
    return (DontClobberDir);
}

/* GetLoginSH
 *	returns the new user's login shell directory.  The default,
 *	which may be overridden, is StrV("DefaultShell").
 */
GetLoginSH (buf)
char   *buf;
{
    FILE * shellf;
    int     done;

    done = FALSE;
    while (NOT done) {
	printf ("Enter shell");
	UseDefault (buf, StrV ("DefaultShell"));
	shellf = fopen (buf, "r");
	if (shellf == NULL) {
	    printf ("I trust you realize that there is no such file.\n");
	    printf ("Is that ok? (y or n) [y] ");
	    if (NOT YesNo ('y'))
		continue;
	}
	if (index (buf, ':'))
	    printf ("Sorry, shell file name must not contain ':'\n");
	else
	    done = TRUE;
    }
}

/* System
 *	
 *	Like the regular "system", but does the right thing with ^C
 */
System (cmdstring)
char   *cmdstring;
{
    int     status, pid, waitstat;

    if (IntV("Debug"))
    	printf("System(%s)\n", cmdstring);
    wasinterrupted = FALSE;

    fflush(stdout);
    fflush(stderr);

    if (!(pid = vfork ())) {
	execl ("/bin/sh", "sh", "-c", cmdstring, 0);
	_exit (127);
    }
    while ((waitstat = wait (&status)) != pid && waitstat != -1);
    if (waitstat == -1)
	status = -1;
    return (status);
}
/* CallSys
 *	repeats a System call until the call returns without error,
 *	or until it returns with an error but without the flag being
 *	set indicating that an interrupt occurred during the call.
 *	This flag, 'wasinterrupted', is set FALSE just before a
 *	System call, and is set to TRUE only if the Catch routine is
 *	called during the critical (uninterruptable) section of the
 *	program.
 */
CallSys (cmd)
char   *cmd;
{
    register int    status;

    while (status = System (cmd))
	if (NOT wasinterrupted)		/* regular system error */
	    return (status);		/* system call bombed */
    return (status);			/* executed without problem */
}

/* DoCommand
 *	calls 'Callsys' to execute the string 'cmd', and supplies
 *	some messages.  Exits if 'fatal' is TRUE.
 */
DoCommand (cmd, fatal, safe)
char   *cmd;
int     fatal, safe;
{
    register int    status;

    if (IntV ("Debug"))
	printf ("%s", cmd);
    if (IntV ("Debug") == 0 || safe) {
	if (status = CallSys (cmd)) {
	    printf ("nu: '%s' failed, status %d\n", cmd, status);
	    perror ("nu");
	    if (fatal) {
		unlink (StrV ("Linkfile"));
		exit (ERROR);
	    }
	}
    }
    else
	printf ("Unsafe command, skipped in Debug mode.\n");
}

/* AddToPasswd
 *	takes buf to hold the new entry for the passwd file, and
 *	inserts this new entry at the end of the file.
 */
AddToPasswd (buf)
char   *buf;
{
    FILE * pwfile;

    printf ("\nEntry to passwd file looks like:\n%s\n", buf);

    if ((pwfile = fopen (StrV ("PasswdFile"), "a")) == NULL) {
	printf ("Error: not able to open %s\n", StrV ("PasswdFile"));
	unlink (StrV ("Linkfile"));
	exit (ERROR);
    }
    else {
	fprintf (pwfile, "%s", buf);
	printf ("Written to %s\n", StrV ("PasswdFile"));
    }
    fclose (pwfile);
}

/* LogAddition
 *	creates an entry for a file that holds information about
 *	newuser's, corresponding to the information in the passwd
 *	file.  This file is used only informally, to keep track
 *	recent additions.
 */
LogAddition (buf)
char   *buf;
{
    FILE * logf;
    long    clock;

    if (logf = fopen (StrV ("Logfile"), "a")) {
	clock = time (0);
	fprintf (logf, "%s\tadded by %s on %s\n", buf, editor, ctime (&clock));
	fclose (logf);
    }
    else
	fprintf (stderr, "Can't make log file entry\n");
}

/* CreateDir
 *	calls a shell script that creates a new directory, to be
 *	the new user's login dir.  This new user becomes the owner
 *	of the directory.
 */
CreateDir (np, clobber)
struct cpasswd *np;
{
    sprintf (cmd, "%s %d %d %s %s %d %d\n",
	    StrV ("CreateDir"),
	    np->cpw_uid,
	    np->cpw_gid,
	    np->cpw_dir,
	    np->cpw_linkdir,
	    clobber,
	    IntV ("Debug")
	);
    DoCommand (cmd, FATAL, SAFE);
}

/* InstallFiles
 *	Call the shell script that puts files in the new directory
 *	and makes sure they have the right ownership.
 */
InstallFiles (np)
struct cpasswd *np;
{
    sprintf (cmd, "%s %s %d %d %d %d\n",
	    StrV ("CreateFiles"),
	    np->cpw_dir,
	    np->cpw_uid,
	    np->cpw_gid,
	    (whichmail == 'm') & IntV ("WantMHsetup"),
	    IntV ("Debug")
	);
    DoCommand (cmd, NONFATAL, SAFE);

}
/* PwPrint
 *	Print the fields of a passwd structure.
 */
PwPrint (cpw)
struct cpasswd *cpw;
{
    printf ("   1)  login ...... %s\n", cpw->cpw_name);
    printf ("   2)  password ... ");
    if (cpw->cpw_passwd[0])
	printf ("%s (encrypted)\n", cpw->cpw_passwd);
    else
	printf ("(none)\n");
    printf ("   3)  name ....... %s\n", cpw->cpw_person);
    printf ("   4)  userid ..... %d\n", cpw->cpw_uid);
    printf ("   5)  groupid .... %d (%s)\n", cpw->cpw_gid, cpw->cpw_group);
    printf ("   6)  login dir .. %s\n", cpw->cpw_dir);
    printf ("   7)  login sh ... %s\n", cpw->cpw_shell[0] ? cpw->cpw_shell
	    : "/bin/sh (null default)");
}

/* Verified
 *	print the current account data, ask if it is Ok, and return
 *	TRUE if it is Ok, false otherwise.	
 */
Verified (np)
struct cpasswd *np;
{
/*
 *  Clear all waiting input and output chars.
 *  Actually we just want to clear any waiting input chars so
 *  we have a chance to see the values before confirming them.
 *  We have to sleep a second to let waiting output chars print.
 */
    sleep (1);
    ioctl (0, TIOCFLUSH, 0);

    PwPrint (np);

    printf ("Are these values OK? (y or n)  [y] ");
    fflush (stdout);
    if (YesNo ('y'))
	return (TRUE);
    else {
	printf ("Do you want to continue? (y or n) [y] ");
	if (YesNo ('y'))
	    return (FALSE);
	else {
	    unlink (StrV ("Linkfile"));
	    exit (OK);
	}
    }
}

/* PasswdLocked
 *	attempts to get exclusive access to the passwd file.  By
 *	convention, any program that wants to write to the passwd
 *	file will try to create a link with the name StrV("Linkfile").  If
 *	such a link already exists, link() returns -1, indicating
 *	that someone else is currently writing to the passwd file.
 */
PasswdLocked () {
    creat (StrV ("Dummyfile"), 0);

    if (IntV ("Debug")) {
	return (0);
    }
    else
	return (link (StrV ("Dummyfile"), StrV ("Linkfile")));
}

/* Catch
 *	is called whenever ^C is typed at the terminal.  If the
 *	critical section flag is set, the program should not be
 *	aborted, and so the routine just returns.  If the flag is not
 *	set, the terminate routine is called to halt the program.
 */
Catch () {
    signal (SIGINT, SIG_IGN);		/* ignore ^C's for a short time */

    if (incritsect) {
	printf ("\nSorry, 'nu' is in a critical section and should not terminate here.\n");
	printf ("Please try again later. The safest thing to do is to let it finish,\n");
	printf ("then go back and kill the new account with 'nu -k name'.\n");
	wasinterrupted = TRUE;
	signal (SIGINT, Catch);
 /* further ^C's trapped to 'Catch()' */
	return (TRUE);
    }
    else {
	printf ("\nProgram aborted.\n");
	unlink (StrV ("Linkfile"));
	exit (ERROR);
    }
}

/* Additions
 *	This is the driver routine for adding new users. It is called from
 *	main() if the "-a" option is found.
 */
Additions () {
    int     done, noclobber;
    char    buffer[BUF_LINE], access[BUF_WORD];
    struct cpasswd  new;

    dbminit (ALIASES);
    done = FALSE;			/* becomes TRUE when the user is
					   satisfied with */
    while (NOT done) {			/*   the data, as it has been set up. 
					*/

	GetLoginName (new.cpw_name);
	GetPassword (new.cpw_asciipw, new.cpw_passwd);
	GetRealName (new.cpw_person);
	new.cpw_uid = GetUserID ();
	new.cpw_gid = GetGroupID (new.cpw_group);
	noclobber = GetLoginDir (&new);
	if (IntV ("WantSymbolicLinks") != 0) {
	    strcpy (new.cpw_linkdir, StrV ("SymbolicLinkDir"));
	    strcat (new.cpw_linkdir, "/");
	    strcat (new.cpw_linkdir, new.cpw_name);
	}
	else
	    strcpy (new.cpw_linkdir, new.cpw_dir);
	GetLoginSH (new.cpw_shell);

	if (noclobber == 0 
	    && IntV("WhatMHsetup")) {
	    printf ("Do you want an initialized ~/Mail for MH? (y or n) [y] ");
	    if (YesNo ('y'))
		whichmail = 'm';
	    else
		whichmail = 'u';
	}

	if (Verified (&new)) {
	    sprintf (buffer, "%s:%s:%d:%d:%s:%s:%s\n",
		    new.cpw_name, new.cpw_passwd, new.cpw_uid, new.cpw_gid,
		    new.cpw_person, new.cpw_linkdir, new.cpw_shell);

	    incritsect = TRUE;		/* should not be interrupted */
	    AddToPasswd (buffer);
	    if (noclobber) {
		CreateDir (&new, 0);
	    }
	    else {
		CreateDir (&new, 1);
		InstallFiles (&new);
	    }
	    LogAddition (buffer);
	    incritsect = FALSE;

	    printf ("\nDo you wish to add more new users? (y or n)  [y] ");
	    done = NOT YesNo ('y');
	}
    }
}


/* Xfer
 *	copies the values from a 'passwd' structure (which
 *	is static in a system 'getpw---' routine) into a
 *	'cpasswd' structure.  This is done so that multiple
 *	'passwd' entries can be saved.
 */
Xfer (pwd, cpw)
struct passwd  *pwd;
struct cpasswd *cpw;
{
    struct group   *agrp;

    strcpy (cpw->cpw_name, pwd->pw_name);
    strcpy (cpw->cpw_passwd, pwd->pw_passwd);
    cpw->cpw_asciipw[0] = 0;
    cpw->cpw_uid = pwd->pw_uid;
    cpw->cpw_gid = pwd->pw_gid;
    if (agrp = getgrgid (pwd->pw_gid))
	strcpy (cpw->cpw_group, agrp->gr_name);
    else
	strcpy (cpw->cpw_group, "unknown");
    strcpy (cpw->cpw_person, pwd->pw_gecos);
    strcpy (cpw->cpw_dir, pwd->pw_dir);
    strcpy (cpw->cpw_shell, pwd->pw_shell);
}

/* PromptForID
 *	queries the user interactively for the identifier of
 *	an entry in /etc/passwd.  If the ID is numeric, it
 *	is assumed to be the userid; otherwise, it is assumed
 *	to be the login.  A pointer to a structure holding
 *	the 'passwd' entry is returned.  The routine will not
 *	terminate until a valid entry is found.
 */
struct passwd  *PromptForID () {
    char    resp[BUF_WORD];
    int     theuid, done;
    struct passwd  *pwd;

    done = FALSE;
    while (NOT done) {
	printf ("\nEnter user identifier (login or uid): ");
	gets (resp);
	if (isdigit (*resp)) {
    /* presumably, a uid has been entered */
	    theuid = atoi (resp);
    /* search passwd for entry with uid = theuid */
	    setpwent ();
	    if ((pwd = getpwuid (theuid)) == NULL)
		printf ("Sorry, that uid is not in use\n");
	    else
		done = TRUE;
	}
	else {
    /* if the entry is sensible, it is a login-name */
	    setpwent ();
	    if ((pwd = getpwnam (resp)) == NULL)
		printf ("Sorry, that login is not in use\n");
	    else
		done = TRUE;
	}
    }
    return (pwd);
}

/* AlreadyStacked
 *	returns TRUE iff the stack of modified entries contains an
 *	entry with uid equal to 'auid'.
 */
AlreadyStacked (auid, stack, size)
int     auid, size;
struct cpasswd *stack;
{
    int     i;
    for (i = 0; i < size; i++)
	if (stack[i].cpw_uid == auid)
	    return (TRUE);
    return (FALSE);
}

/* GetMod
 *	asks user to identify a particular passwd entry, prints
 *	the entry, prompts for changes to the entry, and leaves
 *	'ent' pointing to an appropriately modified copy of the
 *	entry.
 */
GetMod (ent, stack, size)
struct cpasswd *ent;
struct cpasswd *stack;			/* the stack of modifications */
int     size;				/* size of the above stack */
{
    struct passwd  *pwd;		/* struct used by system 'getpw--'
					   routines */
    char    reply[BUF_WORD];
    int     morechgs = TRUE,
            needID = TRUE;

    while (morechgs) {
	if (needID) {
	    pwd = PromptForID ();
	    Xfer (pwd, ent);		/* Copy to local storage */
	    needID = FALSE;
	}
	printf ("Entry is now:\n");
	PwPrint (ent);
	if (AlreadyStacked (ent->cpw_uid, stack, size)) {
	    printf ("\nWARNING: This entry has already been modified\n");
	    printf ("	 in this session.  The entry above is\n");
	    printf ("	 the unmodified version.  If you modify\n");
	    printf ("	 it, the previous changes will be lost.\n");
	}
	printf ("\nSelect field to be modified ");
	printf ("(1-7, q (discard changes), or e (make changes): ");
	gets (reply);
	switch (*reply) {
	    case '1': 			/* get new login */
		GetLoginName (ent->cpw_name);
		break;
	    case '2': 			/* get new password */
		GetPassword (ent->cpw_asciipw, ent->cpw_passwd);
		break;
	    case '3': 			/* get new name */
		GetRealName (ent->cpw_person);
		break;
	    case '5': 			/* get groupid */
		ent->cpw_gid = GetGroupID (ent->cpw_group);
		break;
	    case '6': 			/* get login directory */
		GetLoginDir (ent);
		break;
	    case '7': 			/* get login shell */
		GetLoginSH (ent->cpw_shell);
		break;
	    case 'q': 			/* another entry */
		needID = TRUE;
		continue;
	    case 'e': 			/* done */
		morechgs = FALSE;
		continue;		/* fall out of while loop */
	    default: 
		printf ("Sorry, invalid selection: %s\n", reply);
		continue;
	}
    }
}

/* SortStack
 *	sorts the stack of structures holding modified passwd
 *	entries by userid, so that it can be merged with the
 *	current /etc/passwd file to produce the updated version.
 *	It is a straight selection-sort, a la Wirth.
 */
SortStack (stk, size)
struct cpasswd *stk;
int     size;
{
    struct cpasswd  ptemp;
    int     i, j, k, limit, cpsize;

    cpsize = sizeof (ptemp);
    limit = size - 1;

    for (i = 0; i < limit; i++) {
	k = i;
	ncpy (&ptemp, &stk[i], cpsize);	/* ptemp := stk[i] */
	for (j = i + 1; j <= limit; j++) {
	    if (stk[j].cpw_uid < ptemp.cpw_uid) {
		k = j;
		ncpy (&ptemp, &stk[j], cpsize);/* ptemp := stk[j]  */
	    }
	}
	ncpy (&stk[k], &stk[i], cpsize);/* stk[k] := stk[i] */
	ncpy (&stk[i], &ptemp, cpsize);	/* stk[i] := ptemp  */
    }
}

/* Linearize
 *	takes a 'cpasswd' structure and converts it into the proper
 *	form for insertion into the passwd file.
 */
char   *Linearize (p)
struct cpasswd *p;
{
    static char buff[BUF_LINE];
    sprintf (buff, "%s:%s:%d:%d:%s:%s:%s\n", p->cpw_name,
	    p->cpw_passwd, p->cpw_uid, p->cpw_gid, p->cpw_person,
	    p->cpw_dir, p->cpw_shell);
    return (buff);
}

/* SwapFiles
 *	copies the current passwd file onto the backup file.  The
 *	modified version of the passwd file is then copied over
 *	the current version.  The temporary modified version is
 *	then removed.
 */
SwapFiles (FromWhat)
char   *FromWhat;
{
    sprintf (cmd, "cp %s %s\n", FromWhat, StrV ("Backupfile"));
    DoCommand (cmd, FATAL, SAFE);

    sprintf (cmd, "cp %s %s\n", StrV ("Tempfile"), FromWhat);
    DoCommand (cmd, FATAL, SAFE);

    sprintf (cmd, "rm %s\n", StrV ("Tempfile"));
    DoCommand (cmd, FATAL, SAFE);
}

/* UpdtPasswd
 *	merges the stack of modified passwd entries with the current
 *	version of the passwd file to create the new version.  The
 *	stack and the file are presumed to be sorted by userid.  An
 *	entry in the stack replaces a file entry with the same
 *	userid.  Changes to the file are logged in a separate file.
 */
UpdatePasswd (stk, size)
struct cpasswd *stk;
int     size;
{
    FILE * tempf;
    char   *newrec;
    struct passwd  *pwd;
    struct cpasswd  cpw_buf;
    int     top;
    long    clock;

    tempf = fopen (StrV ("Tempfile"), "w");
    logf = fopen (StrV ("Logfile"), "a");

    setpwent ();			/* rewind passwd file search */
    top = 0;				/* top of stack of modified entries */
    stk[size].cpw_uid = MAXUID;		/* put sentinel on top of stack */
    while (pwd = getpwent ()) {
	Xfer (pwd, &cpw_buf);
	if (cpw_buf.cpw_uid == stk[top].cpw_uid) {
    /* Stacked entry should be substituted for original */
    /* If more than one stacked entry has the same userid, only the last one
       should be used.  If a stable sort has been used, this will correspond
       to the last attempt at changing a given entry. */
	    while (stk[top].cpw_uid == stk[top + 1].cpw_uid)
		++top;

    /* substitute modified version of record in file */
	    newrec = Linearize (&stk[top]);
	    fprintf (tempf, "%s", newrec);

    /* make an entry in the log file */
	    fprintf (logf, "%s\tchanged to\n", Linearize (&cpw_buf));
	    fprintf (logf, "%s", Linearize (&stk[top]));
	    clock = time (0);
	    fprintf (logf, "\tby %s on %s\n", editor, ctime (&clock));
	    ++top;
	}
	else {
    /* rewrite old unmodified passwd entry */
	    newrec = Linearize (&cpw_buf);
	    fprintf (tempf, "%s", newrec);
	}
    }
    fclose (tempf);
    fclose (logf);
    endpwent ();

 /* Save the old passwd file and replace it with the new version */
    SwapFiles (StrV ("PasswdFile"));
}

/* Modify
 *	is the main routine when NU is called in modify mode.  It
 *	prompts the user for modifications to the passwd file;
 *	stacks up to MAXMODS of these modified entries; sorts the
 *	stack by userid; and merges the stack with the current
 *	passwd file, creating the updated version of the file.
 */
Modify () {
    struct cpasswd  mods[MAXMODS + 1];
    int     modtop;			/* next opening in 'mods' stack */
    int     done;

    printf ("\n\t\t>>> Modify mode <<<\n");
    modtop = 0;
    done = FALSE;
    dbminit (ALIASES);

    while (NOT done) {
	GetMod (&mods[modtop], mods, modtop);
	if (++modtop >= MAXMODS) {
	    printf ("\nYou have now made %d modifications, ", MAXMODS);
	    printf ("you must rerun nu to make more.\n\n");
	    done = TRUE;
	}
	else {
	    printf ("\nDo you want to modify any more /etc/passwd entries? (y or n) [y]");
	    done = NOT YesNo ('y');
	}
    }
    printf ("\n%d modified entries stacked. Sorting...", modtop);
    fflush(stdout);
    if (modtop == 0) {
	unlink (StrV ("Linkfile"));
	exit (OK);
    }

    SortStack (mods, modtop);
    printf ("merging...");fflush(stdout);

    incritsect = TRUE;
    UpdatePasswd (mods, modtop);
    incritsect = FALSE;
    printf ("done.\n");
}

/*
 * Kill various accounts
 */
KillUser (argc, argv)
int     argc;
char   *argv[];
{
    struct passwd  *pwd;
    char    egrepstr[BUF_LINE], logindir[BUF_LINE];
    int     cc;
    register int    i;

    for (i = 2; i < argc; i++)
	if (pwd = getpwnam (argv[i])) {
	    cc = readlink (pwd->pw_dir, logindir, BUF_LINE);
	    if (cc == -1)
		strcpy (logindir, pwd->pw_dir);
	    else
		logindir[cc] = 0;

	    sprintf (cmd, "%s %s %s %s %s %d\n",
		    StrV ("DestroyAccts"),
		    pwd->pw_name,
		    logindir,
		    pwd->pw_dir,
		    StrV ("Logfile"),
		    IntV ("Debug")
		);
	    DoCommand (cmd, FATAL, SAFE);
	}
	else
	    printf ("nu: no such user as %s\n", argv[i]);
}

/*
 * Delete accounts: Do what KillUser() does but don't erase
 * /etc/passwd entries, so accounting information is available.
 * Also, work interactively; structurally similar to Modify(),
 * except that it doesn't postpone updates, so it can bomb out
 * if an error occurs.
 */
DeleteAccounts()
{
	struct cpasswd  del[2];
	int     done;

	printf ("\n\t\t>>> Deletion mode <<<\n");
	done = FALSE;
	dbminit (ALIASES);

	while (NOT done) {
	    if (GetDel (del, del, 0)) {
		printf ("merging...");fflush(stdout);
		incritsect = TRUE;
		UpdatePasswd (del, 1);
		incritsect = FALSE;
		printf ("done.\n");
	    }
	    printf("\nDo you want to delete any more users? (y or n) [y]");
	    done = NOT YesNo ('y');
	}
}

/* GetDel
 *	asks user to identify a particular passwd entry, prints
 *	the entry, prompts to check if entry should be deleted,
 *	modifies the password field if so, and leaves
 *	'ent' pointing to an appropriately modified copy of the
 *	entry.  Returns false if no deletion is wanted
 */
GetDel (ent, stack, size)
struct cpasswd *ent;
struct cpasswd *stack;			/* the stack of deletions */
int     size;				/* size of the above stack */
{
	struct passwd  *pwd;		/* struct used by system 'getpw--'
					   routines */
	char logindir[BUF_LINE];
	char	delcmd[BUF_LINE];
	int cc;

	pwd = PromptForID ();
	Xfer (pwd, ent);		/* Copy to local storage */
	printf ("Entry is now:\n");
	PwPrint (ent);
	if (AlreadyStacked(ent->cpw_uid, stack, size)) {
	    printf ("\nWARNING: This entry has already been deleted\n");
	    printf ("	 in this session.\n");
	}
	if (ent->cpw_passwd[0] == '*') {
	    printf("\nThis account is already disabled.\n");
	    return(0);
	}
	printf ("\nDo you want to delete this entry? (y or n) [y] ");
	if (NOT YesNo('y')) {
	    return(0);	/* no deletion */
	}
	cc = readlink(ent->cpw_dir, logindir, BUF_LINE);
	if (cc == -1)
	    strcpy(logindir, ent->cpw_dir);
	else
	    logindir[cc] = 0;
	sprintf(delcmd, "%s %s %s %s %s %d\n",
		StrV("DeleteAccts"),
		ent->cpw_name,
		logindir,
		ent->cpw_dir,
		StrV("Logfile"),
		IntV("Debug")
		);
	DoCommand(delcmd, FATAL, SAFE);
	strcpy(ent->cpw_asciipw, "[untypeable password]");
	strcpy(ent->cpw_passwd, "*");	/* cannot match a typed password */
	strcpy(ent->cpw_shell, "/bin/noshell");
	strcpy(ent->cpw_dir, "/nosuchdir");

	return(1);
}

/* 
  This procedure reads in the nu.cf configuration file and uses its contents
  to initialize various tables and configuration variables.
*/
ReadCf () {
    FILE * cfile;
    char    lbuf[BUF_LINE];
    char    c, *cp, *op, *name, *sv;
    long    iv;
    int     i, istat;
    int     DefaultGroupHasHome = 0;
    if ((cfile = fopen (CONFIGFILE, "r")) == NULL) {
	fprintf (stderr, "nu: Unable to open configuration file \"%s\".\n",
		CONFIGFILE);
	exit (ERROR);
    }
    while (fgets (cp = lbuf, BUF_LINE, cfile) != NULL) {
	sv = name = (char *) 0;
	iv = 0;
	op = (char *) 0;
	for (cp = lbuf; *cp != 0; cp++) {
	    switch (*cp) {
		case '\t': 
		case ' ': 
		    *cp = 0;
		    continue;
		case ';': 
		    goto exitforloop;
		case '=': 
		    name = (char *) malloc (cp - op + 2);
		    *cp = 0;
		    strcpy (name, op);
		    cp++;
		    while (*cp == ' ' || *cp == '\t')
			cp++;
		    if (strcmp (name, "GroupHome") == 0) {
			iv = atol (cp);
			for (; *cp != '"' && *cp != 0; cp++);
			cp++;
			for (op = cp; *cp != '"' && *cp != 0; cp++);
			sv = (char *) malloc (cp - op + 2);
			*cp = 0;
			strcpy (sv, op);
			if (numtopnodes + 1 < MAXGROUPS) {
			    topnode[numtopnodes].gid = (int) iv;
			    topnode[numtopnodes].topnodename = sv;
			    topnode[numtopnodes + 1].gid = 0;
			}
			numtopnodes++;
			if (iv == IntV ("DefaultGroup"))
			    DefaultGroupHasHome = 1;
		    }
		    else {
			if (*cp == '"') {
			    cp++;
			    for (op = cp; *cp != '"' && *cp != 0; cp++);
			    sv = (char *) malloc (cp - op + 2);
			    *cp = 0;
			    strcpy (sv, op);
			}
			else {
			    iv = atol (cp);
			}
			if (nsymbols < MAXSYMBOLS) {
			    Symbols[nsymbols].SymbName = name;
			    Symbols[nsymbols].Svalue = sv;
			    Symbols[nsymbols].ivalue = iv;
			}
			nsymbols++;
		    }
		    goto exitforloop;
		default: 
		    if ((int) op == 0)
			op = cp;
		    continue;
	    }
	}
exitforloop: continue;
    }
    if (numtopnodes == 0) {
	fprintf (stderr, "nu: no GroupHome info in %s\n", CONFIGFILE);
        unlink (StrV ("Linkfile"));
	exit (ERROR);
    }
    if (numtopnodes >= MAXGROUPS) {
	fprintf (stderr, "nu: %s defines %d GroupHomes; limit is %d.\n",
		CONFIGFILE, numtopnodes, MAXGROUPS - 1);
	unlink (StrV ("Linkfile"));
	exit (ERROR);
    }
    if (nsymbols > MAXSYMBOLS) {
	fprintf (stderr, "nu: %s defines %d symbols; limit is %d.\n",
		CONFIGFILE, nsymbols, MAXSYMBOLS);
	unlink (StrV ("Linkfile"));
	exit (ERROR);
    }
    for (i = 0; topnode[i].gid; i++) {
	istat = stat (topnode[i].topnodename, &statbuf);
	if (istat != 0) {
	    fprintf (stderr, "nu: a GroupHome declaration names %s as the home\n    directory for group %d, but it does not exist. Please fix.\n",
		    topnode[i].topnodename, topnode[i].gid);
	    unlink (StrV ("Linkfile"));
	    exit (ERROR);
	}
	if (!((statbuf.st_mode) & S_IFDIR)) {
	    fprintf (stderr, "nu: a GroupHome declaration names %s as the home dir for group %d,\n    but it is not a directory. Please fix.\n",
		    topnode[i].topnodename, topnode[i].gid);
	    unlink (StrV ("Linkfile"));
	    exit (ERROR);
	}
    }

    if (!DefaultGroupHasHome) {
	fprintf (stderr, "nu: %s defines DefaultGroup=%d, but there is no\n GroupHome declaration for group %d. Be careful.\n",
		CONFIGFILE, IntV ("DefaultGroup"), IntV ("DefaultGroup"));
    }
}

/* IntV and StrV return integer and string values that were defined in
   the configuration file CONFIGFILE. */
int
        IntV (name)
char   *name;
{
    int     j;
    for (j = 0; j <= nsymbols; j++) {
	if (strcmp (Symbols[j].SymbName, name) == 0)
	    return (int) (Symbols[j].ivalue);
    }
    fprintf (stderr, "nu: no definition of %s in nu.cf; cannot continue.\n", name);
    unlink (StrV ("Linkfile"));
    exit (ERROR);
}

char
       *StrV (name)
char   *name;
{
    int     j;
    for (j = 0; j <= nsymbols; j++) {
	if (strcmp (Symbols[j].SymbName, name) == 0)
	    return (char *) (Symbols[j].Svalue);
    }
    fprintf (stderr, "nu: no definition of %s in nu.cf; cannot continue.\n", name);
    unlink (StrV ("Linkfile"));
    exit (ERROR);
}


/*
 *	M A I N   P R O G R A M
 */

main (argc, argv)
int     argc;
char   *argv[];
{
    char   *p;
    struct passwd  *pwd;

    if (argc == 1)
	goto uusage;
    incritsect = FALSE;
    signal (SIGINT, Catch);		/* catch ^C's */
    gethostname (This_host, 100);
    printf ("nu 3.3 [10 Oct 1984] (%s:%s)\n", This_host, CONFIGFILE);
    ReadCf ();

    if (IntV ("Debug"))
	printf (">>>In debugging mode (no dangerous system calls)<<<\n");
    else {
	if (geteuid ()) {		/* not a super-user */
	    printf ("Sorry, you must have superuser status to run nu without debug mode.\n");
	    unlink (StrV ("Linkfile"));
	    exit (ERROR);
	}
    }

    if (PasswdLocked ()) {
	printf ("\nPassword file is locked (see vipw(8))\n");
	exit (ERROR);
    }
    if (p = getlogin ())
	strcpy (editor, p);
    else {
	pwd = getpwuid (getuid ());
	if (pwd)
	    strcpy (editor, pwd->pw_name);
	else
	    strcpy (editor, "UNKNOWN!");
    }
    if (argc == 1)
	goto uusage;
    else {
	if (argv[1][0] == '-')
	    switch (argv[1][1]) {
		case 'a': 
		    Additions ();
		    break;
		case 'm': 
		    Modify ();
		    break;
		case 'd':
		    DeleteAccounts();
		    break;
		case 'k': 
		    if (argc < 3) {
			fprintf (stderr, "usage: nu -k user ...\n");
		    }
		    else {
			KillUser (argc, argv);
		    }
		    break;
		default: 
		    goto uusage;
	    }
	else
	    goto uusage;
    }
    unlink (StrV ("Linkfile"));
    exit (OK);
uusage: 
    fprintf (stderr,
	    "usage:	nu -a			add new accounts\n");
    fprintf (stderr, "	nu -m			modify existing accounts\n");
    fprintf (stderr, "	nu -d			delete existing accounts\n");
    fprintf (stderr, "	nu -k user1 user2 ...	kill old accounts\n");

    unlink (StrV ("Linkfile"));
    exit (ERROR);
}

/*
 *	chown, chgrp, chto, chusr, chmine, chsame - improved chown/chgrp
 *
 *	    This is a more flexible version of the chown/chgrp commands.
 *	If REASONABLE is not #defined, chown and chgrp should be identical
 *	to the originals (except for slight differences in the usage:
 *	error message).  Otherwise, two subtle changes will occur:
 *	suid and sgid bits will be preserved for non-superusers (if possible),
 *	and 'changing' a file to be what it already is (like "chown root /")
 *	will always succeed.  These slight changes were made to prevent
 *	accidentally turning off suid/guid bits with global commands like
 *	"chmine -s *" for non-superusers.  Of course, bozoid stuff like
 *	"chown root myprog" will still disable any suid bit (since the
 *	chmod call to restore the suid bit will fail).
 *
 *	Since all the programs are linked, the total size is about 40%
 *	smaller than the original (separate) chown & chgrp.
 *
 *	The six flavors are:
 *
 *	chown  [-s] owner files ...
 *	chgrp  [-s] group files ...
 *	chto   [-s] owner group files ...
 *	chusr  [-s] username files ...
 *	chmine [-s] files ...
 *	chsame [-s] template files ...
 *
 *	    chown & chgrp act as they always have.  chto changes both owner
 *	and group of the files (MUST be in this order, since "chto 1 2 foo"
 *	would otherwise be ambiguous).  chusr changes the owner & group to
 *	match the given user's login uid and gid (chusr must be given a login
 *	name, not a number).  chmine changes the files to match your current
 *	uid and gid.  chsame changes the files to match the first file given,
 *	as in "chsame . *"
 *
 *	    The -s option will not change the uid [gid] of files that have
 *	the suid [sgid] bit set.  This will avoid problems with:
 *		cd /bin
 *		chusr -s bin *
 *	...which would disable su, ps, etc. without the -s option.
 *
 *	Flames to: ...ihnp4!umn-cs!rosevax!rose3!merlyn!brian (or root)
 *		a.k.a Merlyn Leroy (back on the air!)
 *
 */

#include	<stdio.h>
#include	<ctype.h>	/* for isdigit */
#include	<pwd.h>		/* for getpwnam */
#include	<grp.h>		/* for getgrnam */
#include	<sys/types.h>	/* for stat */
#include	<sys/stat.h>	/* for stat */

/* #define REASONABLE if you want a more reasonable chown/grp/etc */
/* don't define it if you want slavish compatability with the old chown/grp */

#define	REASONABLE

#define	when		break;case	/* for convenience */
#define	otherwise	break;default

#define	errchk(eval)	if (eval) { perror(*argv); exitval = 1; continue; }

#define	SET_UID	1
#define	SET_GID	2
#define	SET_ALL	(SET_UID | SET_GID)

#define	OWN	0
#define	GRP	1
#define	TO	2
#define	USR	3
#define	MINE	4
#define	SAME	5

#define	FIRST	OWN	/* FIRST is also the default function */
#define	LAST	SAME	/* if invoked under an unknown name */

extern chown();
extern chmod();
extern stat();
extern struct passwd *getpwnam();
extern struct group *getgrnam();

struct passwd *pw;	/* struct returned by getpwnam() */
struct group *gr;	/* struct returned by getgrnam() */
char *progname;		/* what is my name? */
short ami;		/* what am i? (set to OWN, GRP, TO, etc) */

struct {
	char *prog;		/* list of recognized program names */
	short n,set;		/* minimum # of params needed; id's to set */
} what[] =     {{ "chown", 2, SET_UID},
		{ "chgrp", 2, SET_GID},
		{ "chto",  3, SET_ALL},
		{ "chusr", 2, SET_ALL},
		{ "chmine",1, SET_ALL},
		{ "chsame",2, SET_ALL}};

/* to avoid the silly strrchr vs. rindex problem, here is strrchr */

char *strrchr(s,ch)
register char *s,ch;
{
    register char *p;

    p = NULL;
    do {
	if (*s == ch) p = s;
    } while (*s++);

    return (p);
}

main(argc,argv)
int argc;
register char *argv[];
{
    struct stat status;		/* to hold the stat() structure returned */
    int uid,gid;		/* uid and gid wanted */
    int chuid,chgid;		/* uid and gid to change */
    short nargs,toset;		/* minimum # of params; id's to set */
    char saveset;		/* boolean option */
    int exitval;		/* return value */

    if ((progname = strrchr(*argv,'/')) != NULL) progname++;
	else progname = *argv;				/* find my name */
    what[FIRST].prog = progname;			/* assign as default */

    ami = LAST+1;
    while (strcmp(progname,what[--ami].prog));		/* find name in list */
    nargs = what[ami].n;
    toset = what[ami].set;

    if (saveset = !strcmp(*++argv,"-s")) argv++, argc--;

    if (argc <= nargs) {
	fprintf(stderr,"usage: %s [-s] ",progname);
	switch (ami) {
	when  USR: fprintf(stderr,"username ");
	when SAME: fprintf(stderr,"template ");
	when MINE:		/* no parameters */
	otherwise: if (toset & SET_UID) fprintf(stderr,"owner ");
		   if (toset & SET_GID) fprintf(stderr,"group ");
	}
	fprintf(stderr,"files ...\n");
	exit(4);
    }

    switch (ami) {
    when  OWN: uid = finduid(*argv++);
    when  GRP: gid = findgid(*argv++);
    when   TO: uid = finduid(*argv++);	gid = findgid(*argv++);
    when  USR: uid = finduid(*argv++);	gid = pw->pw_gid; /* from getpwnam */
    when MINE: uid = getuid();		gid = getgid();
    when SAME: if (stat(*argv,&status)) {
		    perror(*argv);
		    exit(4);
		}
		uid = status.st_uid;	gid = status.st_gid;
		argv++;
    }

    exitval = 0;
    for (argc -= nargs; argc--; argv++) {
	errchk(stat(*argv,&status));			/* stat the file */
	switch (toset) {
	when SET_UID: gid = status.st_gid;		/* don't change gid */
	when SET_GID: uid = status.st_uid;		/* don't change uid */
	}
	chuid = (saveset && (status.st_mode & S_ISUID)) ? status.st_uid : uid;
	chgid = (saveset && (status.st_mode & S_ISGID)) ? status.st_gid : gid;

#ifdef REASONABLE
	/*
	 * Only change files that need it, to be nice to non-superusers.
	 * Otherwise, suid & sgid bits may be removed accidentally.
	 */
	if (chuid != status.st_uid || chgid != status.st_gid) {
	    errchk(chown(*argv,chuid,chgid));
	    if (status.st_mode & (S_ISUID | S_ISGID))
		chmod(*argv,status.st_mode);  /* try to preserve suid & sgid */
	}
#else
	errchk(chown(*argv,chuid,chgid));
#endif /* REASONABLE */
    }
    return (exitval);
}

/* find uid of user, or literal uid value */
/* chusr must find username only */
finduid(name)
char *name;
{
    if ((pw = getpwnam(name)) == NULL)
	if (nondigit(name)) {
	    fprintf(stderr,"%s: unknown user id %s\n",progname,name);
	    exit(4);
	}
	else if (ami == USR) {
	    fprintf(stderr,
	    "usage: %s [-s] username (NOT uid number) files ...\n",progname);
	    exit(4);
	}
	else return (atoi(name));
    else return (pw->pw_uid);
}

/* find gid of group, or literal gid value */
findgid(name)
char *name;
{
    if ((gr = getgrnam(name)) == NULL)
	if (nondigit(name)) {
	    fprintf(stderr,"%s: unknown group id %s\n",progname,name);
	    exit(4);
	}
	else return (atoi(name));
    else return (gr->gr_gid);
}

/* return true (nonzero) if string contains a non-digit */
nondigit(s)
register char *s;
{
    while (*s && isdigit(*s)) s++;
    return *s;
}

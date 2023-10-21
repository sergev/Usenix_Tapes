#include "hd.h"
#include "mydir.h"
#ifdef	NDIR
#include <errno.h>
#endif
#define	curmtime	wd_stb.st_mtime

/* See mydir.h for information about these variables */
int tfiles, tpages, cpage;
int pageoff = 0, column = 1, colfield = 19;

struct stat wd_stb;

int wdfile;
char wdname [MPLEN + 1];
/* dyt */
char wdname0 [MPLEN + 1];
static int oldpage = 1;

char selectname[STRMAX];
int selectcmd;
int selectpage;
int selecttype;

int owindow = 0;

#ifndef	NDIR
struct direct *dirbuf;
char **d_namep;
#else
struct direct **d_dirp;
#endif

int dir_status;
struct stat scr_stb;

char *dirmsg [] = 
{
    "", "(System Error)  ", "(Too big)  ", 
	"(Unreadable)  "
} ;

/*  Enterdir -- Enter into new directory.
    The parameter newdir is the file to change to.  Pass a single
    element file name or a full path name. "file", "..", or "."
    are OK.  "../..", "/a/b/..", "a/", or "a/." must be passed
    to the file procedure instead.

    On success, there is a new current directory, and wdname has been
    modified to reflect the new path name.  ENTERDIR | REPLOT is the
    return value.  On failure, NOREPLOT is returned.  Nothing is changed.
*/

enterdir (newdir)
register char *newdir;
{
    char wdname1 [MPLEN + 1];
    register int entermode;	/* Type of entry */
#define	FOREWARD	0	/* Deaper into dir */
#define	BACKWARD	1	/* Previous dir (..) */
#define	STOP		2	/* Stay in same place (.) */
#define LOAD		3	/* Load new path name */

    if (compe (newdir, DOT)) entermode = STOP;
    else if (compe (newdir, DOTDOT)) entermode = BACKWARD;
    else if (newdir[0] == '/') entermode = LOAD;
    else entermode = FOREWARD;

    if (entermode == FOREWARD && strlen (wdname) > LPLEN) 
    {
	putmsg ("Cannot chdir -- Pathname too long");
	return NOREPLOT;
    }

    if (chdir (newdir)) 
    {
	myperror (newdir);
	return NOREPLOT;
    }

	/* dyt */
	*wdname1 = 0;
	if (strcmp(wdname, newdir) &&
		(strcmp(DOTDOT, newdir) || strcmp(SLASH, wdname)))
		strcpy(wdname1, wdname);

    if (entermode == STOP);	/* change to "." */
    else if (entermode == BACKWARD) 
    {	/* change to ".." */
	todotdot (wdname);
/* This fails for filesystems not mounted directly off root
	if (ISROOT (wdname)) chdir (SLASH);
 */	if (chdir(wdname)) {
		myperror(wdname);
		return NOREPLOT;
	}
    }
    else if (entermode == LOAD) strcpy (wdname, newdir);

    else
    {
	if (!ISROOT (wdname)) strcat (wdname, SLASH);
	strcat (wdname, newdir);	/* go deeper into dir */
    }

	/* dyt - try to go to old page, save previous directory name */
	entermode = cpage;
	cpage = strcmp(wdname0, wdname) ? 1 : oldpage;
	pageoff = 0;
	oldpage = entermode;
	if (*wdname1)
		strcpy(wdname0, wdname1);

#ifndef	NDIR
    close (wdfile); wdfile = open (DOT, 0);
#endif
    dir_status = unloaded;
    *selectname = 0;

    return ENTERDIR | REPLOT;
}

/* loaddir loads dir and sets assoc parameters */
loaddir () 
{
    register char *s;
    register unsigned i;
    char *malloc();
    static int first=1;

    if (!first) {
#ifndef	NDIR
	if (dirbuf != NULL)
		free (dirbuf);
	if (d_namep != NULL)
		free (d_namep);
#else
	freedir (d_dirp);
	d_dirp = NULL;
#endif
    }
    else
	first = 0;
    tpages = tfiles = 0;

#ifdef	NDIR

    dir_status = unloaded;
    if (sortdir())
	dir_status = loaded;

#else

    if (wdfile < 0) 
    {
	dir_status = protected; return;
    }
    else dir_status = unloaded;

    fstat (wdfile, &wd_stb);

/* dyt */
#ifdef	V6
#define	st_size	st_size1
#endif

    i = (unsigned)wd_stb.st_size;
    /* Allocate one extra byte for a null (in case last entry is dirsize) */
    s = malloc(i + 1);
    if ((dirbuf = (struct direct *) s) == NULL)
    {
	dir_status = toobig;
	return;
    }
    s[i] = 0;
    lseek (wdfile, 0L, 0);	/* read in entire file */
    if (read (wdfile, dirbuf, (unsigned) wd_stb.st_size)
	!= (unsigned) wd_stb.st_size)
	return;

	/*
	 * dyt - Alloc name pointer array
	 *	Note that this array is larger than necessary since there
	 *	will be empty slots in the directory file. To economize
	 *	on this array size, one could put this code in sortdir()
	 *	after it is determined how many files there are. The trade-off
	 *	is that sortdir() would have to make two passes through
	 *	dirbuf. The wager here is that most (especially large)
	 *	directories don't have too many empty slots anyways.
	 */
    d_namep = malloc(((unsigned) wd_stb.st_size/dirsize)*sizeof d_namep[0]);
    if (d_namep == NULL) {
	printf(" Out of memory");
	dir_status = toobig;
	return;
    }

    sortdir ();
    dir_status = loaded;

#endif

    tpages = tfiles / nfpp + ((tfiles % nfpp) > 0);
    return;
}

#ifndef	NDIR
/* sortdir sorts the directory entries.  when done, the following is true:
    1)  tfiles contains the number of files available
    2)  the d_namep array will contain pointers to the files.
	these will be sorted assending.
*/
sortdir () 
{
    register struct direct *maxent, *dirp;
    int dircmp ();

    tfiles = 0;
    maxent = & dirbuf [(unsigned)wd_stb.st_size / dirsize];
    for (dirp = dirbuf; dirp < maxent; dirp++) 
    {
	if (dirp->d_ino) {
		d_namep [tfiles++] = dirp->d_name;
	/* Note: this has the effect of providing a null byte at the end */
		dirp->d_ino = 0;
	}
    }
    qsort (d_namep, tfiles, sizeof d_namep [0], dircmp);
}

dircmp (a, b) char **a, **b;
{
    return strcmp (*a, *b);
}

#else

/*
 * For the NDIR version, we set a pointer d_dirp to an array of pointers
 *	to sorted directory entries. The number of entries is also placed in
 *	tfiles
 */
sortdir()
{
	extern int errno;
	extern alphasort();

	tfiles = scandir(DOT, &d_dirp, NULL, alphasort);
	if (tfiles == -1) {
		dir_status = protected;
		if (errno == ENOMEM)
			dir_status = toobig;
		tfiles = 0;
		d_dirp = NULL;
		return NULL;
	}
	return 1;
}

/*
 * Free memory associated with the sorted directory entry array
 */
freedir(p)
struct direct **p;
{
	register int i;

	if (p == NULL)
		return;
	i = tfiles;
	while (i-- > 0)
		free (p[i]);
	free (p);
}
#endif

/*  Dispdir displays a page of the directory.
    W A R N I N G.  Dispdir modifies global data.  If the dir is not
    loaded, or is out of date, dispdir will call on loaddir.
    Cpage can be adjusted to conform to the current dir.
    An out of date dir is reloaded only if reload is true.

    In general, the goal of dispdir is to make sure the internal
    representation of the directory is consistent with the real
    directory, and what is displayed is consistent with the internal
    directory.
*/

dispdir (reload) int reload;
{
    register int dirx;		/* index into dirbuf */
    register int dirchar;	/* char to select file assoc. with dirx */
    register int mode;
    int pe, po;
    extern char username[];
    struct stat tmpsbuf;
    extern int didlong;		/* from longlist() */

    long lastmtime;	/* last time dir was modified */

    bufout (); clearmsg (-1);

    dirx = 0;
    didlong = 0;
    if (window != owindow) {
	owindow = window;
	dirx++;
    }
    if (reload) 
    {
	lastmtime = curmtime;
#ifndef	NDIR
	fstat (wdfile, &wd_stb);
#else
	stat(DOT, &wd_stb);
#endif
	if (dirx || (lastmtime != curmtime) || dir_status) loaddir ();
    }
    else if (dirx)
	loaddir();

    cpage = max (1, min (cpage, tpages));

    erase ();
    hilite (1);
    printf ("Directory: %s  %sUser: %s", wdname, dirmsg [dir_status],
	username);

    if (VSHMODE == SELECTMODE) {
	printf("  (Select mode)");
	mode = 15;
    }
    else
	mode = 0;

    if (tfiles == 0) 
    {
	hilite(0);
	unbufout (); return;
    }

    if (tpages > 1) 
    {
	atxy (66+xoff,
		(strlen(wdname)+strlen(username)+mode+strlen(dirmsg[dir_status])
		> 45+xoff) ? 2: 1);
	if (column > 1 && cpage != tpages) {
		po = cpage+column-1;
		if (po > tpages)
			po = tpages;
		printf ("Page %d-%d / %d", cpage, po, tpages);
	}
	else
		printf ("Page %d / %d", cpage, tpages);
    }

    hilite(0);
    po = pageoff;
    pageoff = 0;

	do {
		pe = pgend();
		for(dirx=0, dirchar='a'; dirx < pe; dirx++, dirchar++) {
			if (pageoff == 0) {
				atfile (dirx, 1);
				printf("%c   ", dirchar);
			}
			else
				atfile (dirx, OFFFILE+(pageoff*colfield));
			printf("%s", filename(dirx));
			stat(filename(dirx),&tmpsbuf);
			mode = tmpsbuf.st_mode;
			if ((mode&S_IFMT) == S_IFDIR)
				printf("/");
			else if (mode&S_IEXEC)
				printf("*");
		}
		pageoff++;
	} while (pageoff < column && (cpage+pageoff) <= tpages);
	pageoff = po;

    if (pageoff >= column)
	pageoff = column - 1;
    if (cpage+pageoff > tpages)
	pageoff = tpages - cpage;
    if (reload && *selectname
	&& (selectcmd >= pgend()
	|| strcmp(filename(selectcmd), selectname)))
	*selectname = 0;
    if (*selectname) {
	atfile(selectcmd, OFFARROW+colfield*pageoff);
	printf(arrow);
    }
    if (column > 1)
	colprompt();

    if (*HELPFILE) {
	mode = 0;
	atfile (mode, 24+xoff);
	for (dirx = 0; dirchar = helptext[dirx]; dirx++) {
		if (dirchar == '\r')
			continue;
		else if (dirchar == '\n')
			atfile(++mode, 24+xoff);
		else
			putch(dirchar);
	}
    }

    unbufout ();
}

/*	Change dir to father of dir  */
todotdot (dir) char *dir;
{

    register char *cp;

    for (cp = dir; *cp; cp++);	/* Scan to end of name */
    while (*--cp != '/');	/* Scan back to a slash */
    if (cp == dir) cp++;	/* Must handle root specially */
    *cp = 0;
}

/* Calculate end of current page including offset */
pgend()
{
	register int pe;

	pe = tfiles % nfpp;
	if ((cpage+pageoff) != tpages || pe == 0)
		pe = nfpp;
	return pe;
}

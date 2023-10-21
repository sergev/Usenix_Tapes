
#include "hd.h"
#include "mydir.h"
#include "command.h"
#include <signal.h>

#define	ISDIR	((scr_stb.st_mode & S_IFMT) == S_IFDIR)
#define	paction(arg)	{ \
	atfile (file, OFFARROW+colfield*pageoff);printf (action [cpage+pageoff][file] ? "//" : "  ");}

/* User first indicates which files to remove by selecting them
   with a-t keypresses.  Selected files are stored in the action
   array.  When ready, the user presses R, and the files are gone.
*/

char quitcmds[] = 
{
    CR, LF, EOT, 0
} ;

/* ^L, ^F, ^U, ^I */
char dircmds [] = 
{
    "0123456789+-L\014\006\025\011\002\026"
} ;

static char *rmmsg[] = 
{
    "\tSelect the files you wish to remove, then press -R-",
	"\tPress -Return- to leave with out removing files. -?- for more help"
} ;

static int rcount;	/* Count of number of files removed */
static int rlast;	/* Last file removed */
extern int nointer;	/* SIGINT flag */

remove () 
{			/* remove (unlink) files */

    register cmd, file; register char *cp;
    int pe;
    short starmode;
#define	OFF	0
#define	ON	1
#define	UNDEF	2

    char (*action)[maxnfpp];
    int (*oldsig)();
    extern catch();
    extern int didlong;

    if (access (DOT, 3)) 
    {
	myperror ("Cannot remove files");
	return NOREPLOT;
    }

    *selectname = 0;

    /* Allocate memory for action flags */
    file = malloc(cmd = (sizeof action[0]) * tfiles);
    action = (char **) file;
    if (action == NULL) {
	putmsg("Out of memory");
	return NOREPLOT;
    }

    for (cp = (char *) file; cp < cmd + (char *) file; *cp++ = OFF);
    rcount = 0;

    dispaction (action);
    disprmmsg ();

    for (;;) 
    {
	atxy (80, window-1);
	cmd = getch ();
	file = cmd - 'a';
	if (cmd == (PAGECHAR-'@'))
		cmd = '+';
	if (cmd == (QUITCHAR-'@') || any(cmd, quitcmds) != NULL) 
	    break;
	if (didlong)
	    rmdispdir(action);
	if (any(cmd, dircmds) != NULL) 
	{
	    if (command(cmd, DIRCMD) & REPLOT) rmdispdir (action);
	    else if (cmd == '.') disprmmsg();
	}
	else if (cmd == '*') 
	{
	    starmode = UNDEF;
	    pe = pgend();
	    for (file = 0; file < pe; file++) 
	    {
		cp = &action [cpage+pageoff][file];

		if (compe (filename (file), DOT));
		else if (compe (filename (file), DOTDOT));

		else if (starmode == UNDEF)
		    *cp = starmode = !*cp;
		else *cp = starmode;
	    }
	    dispaction (action);
	}

	else if (cmd == 'R') break;
	else if (cmd == '?') 
	{
	    help (RMHELP);
	    rmdispdir (action);
	}
	else if (cmd >= 'a' && cmd <= 'z' && file < pgend()) 
	{
	    if (compe (filename (file), DOT));
	    else if (compe (filename (file), DOTDOT));
	    else
	    {
		action [cpage+pageoff][file] ^= 1;
		paction (file);
	    }
	}
    }
    if (cmd == 'R') {
	nointer = 1;
	oldsig = signal(SIGINT, catch);
	act (action);
	signal(SIGINT, oldsig);
    }
    free(action);
	if (rcount) {
		rlast = 1 + ((rlast-rcount)/nfpp);
		if (rlast < cpage || rlast > cpage+column-1) {
			cpage = rlast;
			pageoff = 0;
		}
	}
	clearmsg(1);
	hilite(" (%d file(s) removed)", rcount);
	sleep(1);
    return REPLOT;
}

dispaction (action) char action [][maxnfpp]; 
{
    register file;
    register int pe;
    int opageoff;

    bufout ();
    opageoff = pageoff;
    for (pageoff = 0; pageoff < column; pageoff++) {
	pe = pgend();
	for (file = 0; file < pe; file++)
		paction (file);
	if (pe < nfpp)
		break;
    }
    pageoff = opageoff;
    unbufout ();
}

rmdispdir (action) char action [][maxnfpp]; 
{
    dispdir (0);
    dispaction (action);
    disprmmsg ();
}

disprmmsg () 
{
    clearmsg (2);
    bufout ();
    hilite ("%s\r\n%s", rmmsg [0], rmmsg [1]);
    unbufout ();
}

/* This does the actual removing */
act (action) char action [][maxnfpp]; 
{
    register page, file;

    clearmsg (1);
    for (page = 1; page <= tpages; page ++)
	for (file = 0; file < nfpp; file++)
	if (!nointer)
		return;
	else if (action [page][file]) {
		if (page < cpage || page > cpage+column-1) {
			pageoff = 0;
			cpage = page; dispdir (0);
			dispaction (action); clearmsg (1);
		}
		else
			pageoff = page - cpage;
		atfile (file, OFFARROW+colfield*pageoff); printf (arrow);
		atxy (1, window-1);
#ifdef	SYMLINK
		if (lstat (filename (file), &scr_stb)) {
#else
		if (stat (filename (file), &scr_stb)) {
#endif
			myperror (filename (file));
	    		if (rmerror ()) return;
		}
		else if(ISDIR && f_exec("/bin/rmdir", "+", filename(file), 0)) {
			if (rmerror ()) return;
		}
		else if (!ISDIR && unlink (filename (file))) {
			myperror (filename (file));
			if (rmerror ()) return;
		}
		else {
			bufout();
			atfile (file, OFFARROW+colfield*pageoff);
			if (column > 1)
				for (rlast = 0; rlast < colfield; rlast++)
					putch(' ');
			else
				clearline();
			unbufout();
			rcount++;
			rlast = ((page-1)*nfpp) + file;
		}
	}
    return;
}

rmerror () 
{		/* Error in removing file */
    register ch;

    atxy (65, window-1);
    hilite ("Press -Return-");
    ch = getch ();
    clearmsg (0);
    return (ch == EOT);
}

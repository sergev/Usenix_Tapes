/*
 * ut -	window version of 
 *	tail -f /usr/spool/uucp/.Log/{uucico,uux,uuxqt,uucp}/nodename
 *
 *	usage: ut nodename
 *
 *	david stone
 *	arete systems, inc
 *	3/86
 *
 *	written and ran under V.2 release 2
 *
 */

#include <stdio.h>
#include <errno.h>
#include <curses.h>
#include <signal.h>
#include <fcntl.h>

extern int	errno;
extern char	*sys_errlist[];
extern void	exit();
extern long	lseek();
extern unsigned sleep();

char	*basedir = BASEDIR;
char	*usage = "usage: %s nodename\n";

char	*node;
char	*progname;

typedef struct _log {
	WINDOW	*win;		/* pointer to window */
	int	numlines;	/* number of lines in win */
	int	start;		/* starting y coordinate for window */
	WINDOW	*lwin;		/* pointer to label window */
	int	fd;		/* file descriptor */
	char	*logname;	/* file name */
} NODE;

NODE	logfiles[] = {
	{ NULL, 11, 0, NULL, NULL, "uucico" },
	{ NULL, 3, 12, NULL, NULL, "uux"  },
	{ NULL, 3, 16, NULL, NULL, "uuxqt" },
	{ NULL, 3, 20, NULL, NULL, "uucp" },
	{ NULL }
};

main(argc,argv)
int	argc;
char	*argv[];
{
	int	cleanup();

	progname = argv[0];

	if ( argc != 2 ) {
		printf(usage, progname);
		exit(1);
	}

	node = argv[1];

	if ( chdir(basedir) ) {
		printf("%s: cd %s failed [%d] (%s)\n",
		  progname, basedir, errno, sys_errlist[errno]);
		exit(2);
	}

	openfiles();

	signal (SIGINT, cleanup);
	signal (SIGQUIT, cleanup);
	signal (SIGTERM, cleanup);

	initscr();		/* ask curses to please get ready */
	initwindows();

	readem();		/* this never returns */

}

initwindows()
{
	register NODE	*np;

	for ( np = logfiles; np->logname; np++ )  {

		/*
		 * label windows are 78 columns wide,
		 * to avoid magic cookie trouble
		 */ 

		np->win = newwin(np->numlines, COLS - 2, np->start, 0);
		scrollok(np->win, 1);
		np->lwin = newwin(1, COLS - 2, np->start + np->numlines, 0);
		wattrset(np->lwin, A_REVERSE);
		if ( np->fd != -1 )
			wprintw(np->lwin, "    %s/%s%.*s", np->logname, node,
			  73 - strlen(np->logname) - strlen(node),
"                                                                      ");
		else
			wprintw(np->lwin, "    %s/%s (no such file)%.*s",
			  np->logname, node,
			  57 - strlen(np->logname) - strlen(node),
"                                                                      ");
		wrefresh(np->lwin);
	}
}

cleanup()
{
	register NODE	*np;

	werase(logfiles[3].lwin);
	wrefresh(logfiles[3].lwin);
	endwin();

	for ( np = logfiles; np->logname; np++ )
		if ( np->fd )
			close(np->fd);		/* close any open files */


	exit(0);
}

openfiles()
{
	register NODE	*np;

	for ( np = logfiles; np->logname; np++ )
		if ( openone(np, 0) )  
			if ( errno == EACCES ) {
				printf("%s: can't open %s/%s [%d] (%s)\n",
				  progname, basedir, node, errno,
				  sys_errlist[errno]);
				exit(errno);
			}
}
/*
 * openone - 	attempt to open basedir/np->logname
 *
 *		if flag is true, then print message into label window
 */

openone(np, flag)
NODE	*np;
int	flag;
{
	char		filename[512];

	sprintf(filename, "%s/%s", np->logname, node);

	if ( (np->fd = open(filename, O_RDONLY|O_NDELAY)) == -1 )
		return(-1);

	if ( flag && np->lwin ) {
		werase(np->lwin);
		wrefresh(np->lwin);
		wprintw(np->lwin, "    %s/%s%.*s", np->logname, node,
		  73 - strlen(np->logname) - strlen(node),
"                                                                      ");
		touchwin(np->lwin);
		wrefresh(np->lwin);
	}
	else
		lseek(np->fd, 0, 2);	/* seek to end of file */

	return(0);

}


readem()
{
register NODE	*np;
register long	count;

infamous_point_of_unstructuredness:

	for ( count = 0, np = logfiles; np->logname; count++, np++ ) {
		
		if ( np->fd == -1 )
			openone(np, 1);
		else
			readone(np);

	}
	sleep(1);
	goto infamous_point_of_unstructuredness;
}

readone(np)
register NODE	*np;
{
	int	retcode;
	char	buf[BUFSIZ];

	switch ( retcode = read(np->fd, buf, BUFSIZ) ) {
	case	0:
		break;
	case	-1:
		;		/* maybe later */
		break;
	default:
		buf[retcode] = NULL;
		wprintw(np->win, buf);
		wrefresh(np->win);
	}
}

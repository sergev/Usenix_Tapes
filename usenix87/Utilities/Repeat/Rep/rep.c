/******************************************************************************
 *									      *
 * rep -- run a program repeatedly using 'curses'.			      *
 *									      *
 *	Usage: rep [-n] command						      *
 *									      *
 *	Permits the user to watch a program like 'w' or 'finger' change with  *
 *	time.  Given an argument '-x' will cause a repetion every x seconds.  *
 *									      *
 * ORIGINAL								      *
 * Ray Lubinsky University of Virginia, Dept. of Computer Science             *
 *	     uucp: decvax!mcnc!ncsu!uvacs!rwl                                 *
 * MODIFIED 3/85							      *
 * Bruce Karsh, Univ. Wisconsin, Madison.  Dept. of Geophysics.               *
 *           uucp: uwvax\!geowhiz\!karsh				      *
 ******************************************************************************/

#include <curses.h>
#include <signal.h>

#define	NextLine	fgets(buf,sizeof buf,input)

FILE	*input;				/* Pipe from 'source' */
char	source[512];			/* Source program to run repeatedly */
char	*progname;			/* Name of this program (for errors) */
int	interval = 1;			/* Seconds between repitions */
int	deathflag = 0;			/* Set by interupt trap routine */

main(argc,argv)

	int	argc;
	char	**argv;
{
	int	endrep();		/* Mop up routine on break */
	FILE	*popen();

	char	buf[BUFSIZ];		/* Buffer holds a line from 'source' */
	int	i;

	progname = *argv;
        source[0]=0;
	if (argc == 1)
		badargs();
	if (**(argv+1) == '-')		/* User specified interval */
		if ((--argc == 1) || ((interval = atoi(*(++argv)+1)) == 0))
			badargs();
	while (--argc > 0) {		/* Argument becomes source program */
		strcat(source,*++argv);
		strcat(source," ");
	}
	signal(SIGINT,endrep);
	if(deathflag == 1)goto die;
	initscr();
	if(deathflag == 1)goto die;
	crmode();
	if(deathflag == 1)goto die;
	nonl();
	if(deathflag == 1)goto die;
	clear();
	if(deathflag == 1)goto die;
	for (;;) {
		if(deathflag == 1)goto die;
		if ((input = popen(source,"r")) == NULL) {
			fprintf(stderr,"%s: can't run %s\n",progname,source);
			goto die;
		}
		for (i = 0; (i < LINES) && (NextLine != NULL); i++) {
			mvaddstr(i,0,buf);
			clrtoeol();
			if(deathflag == 1)goto die;
		}
		if(deathflag == 1)goto die;
		pclose(input);
		if(deathflag == 1)goto die;
		clrtobot();
		if(deathflag == 1)goto die;
		refresh();
		if(deathflag == 1)goto die;
		sleep(interval);
		if(deathflag == 1)goto die;
	}
die:
signal(SIGINT,SIG_IGN);
if (input != NULL)
	pclose(input);
clear();
refresh();
endwin();
exit(0);
}

endrep()
{
deathflag=1;
}

badargs()
{
	fprintf(stderr,"Usage: %s [-n] command\n",progname);
	exit(1);
}

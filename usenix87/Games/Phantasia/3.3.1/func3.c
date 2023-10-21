/*
 * func3.c	Phantasia support routines
 */
#include "phant.h"

jmp_buf btimeout;
int catchalarm();

int getans(choices, def)	/* get a one character option from keyboard */
char	*choices;
bool	def;
{
int ch, loop, oldx, oldy;
char	*cptr;

    getyx(stdscr, oldy, oldx);
    alarm(0);

    for (loop = 3; loop; --loop)
	if (setjmp(btimeout))
	    if (def)
		return(*choices);
	    else if (loop > 1)
		goto YELL;
	    else
		break;
	else
	    {
	    clrtoeol();
	    refresh();
#ifdef BSD41
	    sigset(SIGALRM, catchalarm);
#else
	    signal(SIGALRM, catchalarm);
#endif
	    if (timeout)
		alarm(7);
	    else
		alarm(600);
	    ch = getch();
	    alarm(0);
	    if ((cptr = strchr(choices, toupper(ch))) != NULL)
		return(*cptr);
	    else if (def)
		return(toupper(ch));
	    else if (!def && loop > 1)
		{
YELL:		mvprintw(oldy+1,0,"Please choose one of : [%s]\n", choices);
		move(oldy, oldx);
		clrtoeol();
		continue;
		}
	    else
		break;
	    }
    return(*choices);
}

int catchalarm()
{
    longjmp(btimeout, 1);
}

void	error(whichfile)
char	*whichfile;
{
extern int errno;

    clear();
    printw("An unrecoverable error has occurred reading %s.  (errno = %d)\n", whichfile, errno);
    addstr("Please run 'setup' to determine the problem.\n");
    exit1();
    /*NOTREACHED*/
}


void	mesg()
{
FILE	*fp;
char	s[100];

    move(3,0);
    clrtoeol();
    if ((fp = fopen(messfile, "r")) != NULL)
	{
	if (fgets(s,100,fp))
	    addstr(s);
	fclose(fp);
	}
}

double	dist(x1, x2, y1, y2)
double	x1, x2, y1, y2;
{
double	deltax, deltay;

    deltax = x1 - x2;
    deltay = y1 - y2;
    return(sqrt(deltax * deltax + deltay * deltay));
}

void scoreboard(stat)		    /* enter login into scoreboard */
reg struct stats *stat;
{
static	struct	sb_ent sbuf;
FILE	*fp;
reg int loc = 0;
bool	found = FALSE;

    if ((fp = fopen(sbfile, "r")) != NULL)
	{
	while (fread((char *) &sbuf, sizeof(sbuf), 1, fp))
	    if (!strcmp(stat->login, sbuf.login))
		{
		found = TRUE;
		break;
		}
	    else
		++loc;
	fclose(fp);
	}

    /*
     * At this point, 'loc' will either indicate a point beyond
     * the end of file, or the place where the previous entry
     * was found.
     */

    if ((!found) || stat->lvl > sbuf.level)
	{
	strcpy(sbuf.login, stat->login);
	strcpy(sbuf.name, stat->name);
	sbuf.level = stat->lvl;
	sbuf.type = stat->typ;
	}
    if ((fp = fopen(sbfile, ACCESS)) == NULL)
	{
	error(sbfile);
	/*NOTREACHED*/
	}
    fseek(fp, (long) (loc * sizeof(sbuf)), 0);
    fwrite((char *) &sbuf, sizeof(sbuf), 1, fp);
    fclose(fp);
}


void show_sb()			/* printout the scoreboard */
{
static	struct	sb_ent	sbuf;
FILE	*fp;

    if ((fp = fopen(sbfile, "r")) != NULL)
	while (fread((char *) &sbuf, sizeof(sbuf), 1, fp))
	    printf("%-20s   Login: %-9s  Level: %6d  Type: %3d\n",
		sbuf.name, sbuf.login, sbuf.level, sbuf.type);
}

int ill_sig(whichsig)		/* come here on bad signals */
int whichsig;
{
    clear();
    printw("Error: caught signal # %d.\n", whichsig);
    exit1();
    /*NOTREACHED*/
}

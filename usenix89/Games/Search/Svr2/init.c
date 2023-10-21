#ifndef lint
static char rcsid[] = "$Header: init.c,v 2.2 85/08/06 22:29:44 matt Exp $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Rewrite by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to initialize the search databases and to
 * open needed files
 *
 * Copyright (c) 1979
 *
 * $Log:	init.c,v $
 * Revision 2.2  85/08/06  22:29:44  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 * Revision 2.1  85/04/10  17:31:03  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.2  85/02/11  12:43:47  matt
 * added GUTS mode
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>

#include "defines.h"
#include "structs.h"

static struct message message;
extern sock;
/*
 * initialize all the databases for search
 */

void init() {
	extern	time_t	time();
	extern	int	trap(),
			core_dump(),
			srand(),
			rand();
	extern	long	lseek();
	extern	t_player player[NPLAYER];
	extern	t_alien alien[NALIEN];
	extern	t_sbase sbase[NBASE];
	extern	t_planet planet;
	extern	int pfd,
		nplayer,
		sfflag,
		errfile,
		sfcount;
	extern	char ppx,
		ppy;
	void	initplayer();
	register t_alien *pa;
	register t_sbase *ps;
	register int i;
	time_t	tvec;
	char	buf[100];
	extern childdeath(), quit();

	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGALRM, SIG_IGN);
	(void) signal(SIGPIPE, SIG_IGN);
	(void) signal(SIGTERM, quit);
	(void) signal(SIGCLD, childdeath);
	/*
	 * clear everything out
	 */
	bzero((char *)player, sizeof(t_player) * NPLAYER);
	pfd = open(POINTFILE, O_RDWR, 0);
	if (pfd < 0) {
		perror(POINTFILE);
		exit(1);
	}
	/*
	 * no big deal if we can't open the error log
	 */
	errfile = open(ERRLOG, O_RDWR|O_CREAT, 0666);
	if (errfile >= 0) {
		lseek(errfile, 0L, 2);
		sprintf(buf, "starting daemon, pid %d\n", getpid());
		errlog(buf);
	}
	tvec = time((time_t *)0);
	srand(tvec);
	nplayer = 0;
	/*
	 * Setup aliens with random locations and random directions.
	 */
	for (pa = alien; pa < &alien[NALIEN]; pa++) {
		pa->cx = (rand()%80)+20;
		pa->cy = (rand()%50)+25;
		pa->cix = (rand()%3)-1;
		pa->ciy = (rand()%3)-1;
		if (pa->ciy == 0 && pa->cix == 0)
			pa->cix = pa->ciy = 1;
		pa->aname = NAMEAL;
		pa->type = NORM;
	}
	/*
	 * Add in the shankers.
	 */
	for (pa = alien; pa < &alien[NSHANK]; pa++) {
		pa->type = SHANK;
		pa->count = (rand()%30)+15;	/* shanker alarm clock */
	}
	/*
	 * Some others are the wandering type.
	 */
	for (; pa < &alien[NSHANK+NWAND]; pa++) {
		pa->type = WANDER;
		pa->aname = NAMEWD;
		pa->count = (rand()%32)+16;
	}
	/*
	 * Star bases get random places also -- but spread out.
	 */
	for (ps = sbase, i = 0; i != NBASE - 1; i++, ps++) {
		ps->xpos = ((rand()%50)+(25*(i+1))%120)+3;
		ps->ypos = ((rand()%50)+(25*(i+1))%120)+3;
	}
	/*
	 * Build quartone.
	 */
	planet.planetname = "Quartone";
	planet.places[0][0] = -((rand()%30)+30);
	planet.places[0][1] = -((rand()%30)+30);
	for (i = 0; i != 5; i++) {
		planet.places[i][1] = planet.places[0][1];
		planet.places[i][0] = planet.places[0][0]+i;
	}
	for (i = 5; i != 12; i++) {
		planet.places[i][1] = planet.places[0][1]-1;
		planet.places[i][0] = planet.places[0][0]+i-6;
	}
	for (i = 12; i != 17; i++) {
		planet.places[i][1] = planet.places[0][1]-2;
		planet.places[i][0] = planet.places[0][0]+i-12;
	}
	ppx = planet.places[8][0];
	ppy = planet.places[8][1];

	sbase[NBASE-1].xpos = ppx-(rand()%20)-4;
	sbase[NBASE-1].ypos = ppy-(rand()%20)-4;
	/*
	 * Solar flare alarm clock....
	 */
	sfcount = 800+(rand()%100);
	sfflag = OFF;
}

struct initlist {
	char	x;
	char	y;
	char	*output;
} initlist[] = {
	{ XAXISTITLE, "--------------- ---------------" },
	{ POSTITLE, "POSITION" },
	{ EGYTITLE, "ENERGY" },
	{ HRTITLE, "HOMING RADAR" },
	{ H1TITLE, "1) " },
	{ H2TITLE, "2) " },
	{ H3TITLE, "3) " },
	{ PTTITLE, "POINTS" },
	{ STTITLE, "STATUS: " },
	{ MSTITLE, "MESSAGE: " },
	{ INTITLE, "INVISIBILTY" },
	{ VLTITLE, "VELOCITY" },
	{ TMTITLE, "TIME" },
	{ MFTITLE, "M-FACTOR" },
	{ PLTITLE, "----- PLAYER MAP ----" },
	{ 0, 0, 0 }
};

void initplayer(p)
register t_player *p;
{
	extern	void	bflush(),
			clear(),
			cput(),
			initpdisplay(),
			pldisplay();
	register int j;
	register struct initlist *pi = initlist;

	p->eoq = p->outputq;
	*p->eoq = NULL;
	clear(p);
	for (j = CENTY; j < XWIND; j++)
		cput(YAXISTITLE, j, p, "|");
	while (pi->x) {
		cput(pi->x, pi->y, p, pi->output);
		pi++;
	}
	cput(EGYDATA, p, "%d", p->energy);
	cput(PTDATA, p, "%d", p->points);
	cput(INDATA, p, p->status.invis == TRUE ? "on " : "off");
	cput(MFDATA, p, "%c", p->mflg+'0');
	initpdisplay(p);
	pldisplay(p, "a");
	p->plstp = 0;
	bflush(p);
}

/*
 * initializes player stuff
 */
t_player *startup(fileform)
t_file *fileform;
{
	extern	void	makescab();
	extern	char	*strncpy();
	extern	t_player player[NPLAYER];
	extern	char ppx, ppy;
	extern	long ptime;
	extern	int  gutsflag;
	register t_player *p;

	for (p = player; p < &player[NPLAYER]; p++)
		if (p->status.alive == FALSE)
			break;
	/*
	 * too many players in the game.
	 */
	if (p >= &player[NPLAYER]) {
		message.mtype = fileform->uid+1;
		message.text[0] = 0;
		msgsnd(sock, &message, 4+4+1, 0);
		return(NULL);
	}
	/*
	 * clear out everything.
	 */
	bzero((char *)p, sizeof(*p));
	if (!ioinit (p, fileform)) {
		p->status.alive = FALSE;
		errlog("STARTUP: ioinit failed.\n");
		return (NULL);
	}
	p->status.alive = TRUE;
	p->status.begin = TRUE;
	p->status.guts  = gutsflag;
	p->curx = ppx+(rand()%10)+5;
	p->cury = ppy+(rand()%10)+5;
	p->mflg = 1;
	p->pltime = ptime;
	p->maxmaxe = p->maxe = p->energy = 250;
	p->uid = fileform->uid;
	(void) strncpy(p->plname, fileform->p_name, sizeof(p->plname)-1);
	return(p);
}

bzero(p, n)
register char *p;
register n;
{
    while (n--) *p++ = 0;
}

childdeath() {
    signal(SIGCLD, childdeath);

    errlog("My child died!\n");
    errlog("Return status = %d\n", wait(0));
}

quit() {
    register t_player *p;
    FILE *f = fopen("/usr/src/local/search/lib/apa", "w");
    fprintf(f, "Got a quit signal\n"); fflush(f);
    errlog("Got a quit signal\n");
    for (p = player; p < &player[NPLAYER]; p++) {
	if (p->status.alive == FALSE) continue;
	fprintf(f, "name: %s, uid %d, energy %d\n",
	    p->plname, p->uid, p->energy); fflush(f);
	errlog("name: %s, uid %d, energy %d\n",
	    p->plname, p->uid, p->energy);
	fprintf(f, "    outputchars:\n%s\n", p->outputq); fflush(f);
	errlog("    outputchars:\n%s\n", p->outputq);
    }
    fclose(f);
    core_dump();
}

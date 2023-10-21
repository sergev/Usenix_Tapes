#ifndef lint
static char rcsid[] = "$Header: main.c,v 1.1 84/08/25 17:11:21 chris Exp $";
#endif

#ifdef PLUS5
#include <signal.h>
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/time.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/data.h"

int keyok;		/* Used to control shot speed */
int bitmaze[16*32];	/* Used for quick position look up */

main(argc, argv)
	int argc;
	char *argv[];
{
	int setkeyok();
	struct user player;
	register int i;
	char *beep_string, *act_string, *getenv();

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		(*argv)++;
		while (**argv)
		    switch (*(*argv)++) {
#ifdef DEBUG
			case 'd':		/* Debug Mode */
			    debug = 1;
			    break;
#endif
			case 'n':
			    nodaemon = 1;	/* Don't contact daemon */
			    break;
			case 's':		/* Use suncore */
#ifdef sun
			    ascii = 0;
			    onsun = 1;
#else
			    fprintf(stderr, "Sun not compiled in\n");
#endif
			    break;
			case 'a':		/* Use curses */
			    ascii = 1;
			    onsun = 0;
			    break;
			default:
			    fprintf(stderr, "Unknown flag %c\n", *(*argv)--);
			    exit(1);
		}
		argc--, argv++;
	}

	(void) srandom(getpid());

	/* Open the windows on the output device and draw maze */

	if (ascii) {
		init_screen();
		draw_maze();
	}
#ifdef sun
	else if (onsun) {
		if (sun_init_screen() < 0)
			exit(1);
		screen_redraw();
		draw_maze();
	}
#endif

	initplayer(&player);

	/*
	 *  Set up the key mappings.
	 *  If the environment variable MAZEWAR_BEEP is set
	 *  and is yes, we set the default key mappings to beep.
	 */
	if ((beep_string = getenv("MAZEWAR_BEEP")) &&
	    (*beep_string == 'y' || *beep_string == 'Y'))
		set_default_acts(1);
	else
		set_default_acts(0);

	if (act_string = getenv("MAZEWAR_KEYS")) {
		if (parse_act_string(act_string)) {
			fprintf(stderr, "Can't parse act format string.\n");
			exit(1);
		}
	}

	initsigs();
	setioctls();
	/* The alarm is used to control user's use of the shoot key */
	ualarm((unsigned long) RMMKEYOK,setkeyok);
	/*
	** If we are running without a daemon (to test output on 
	** screens usually)  Set up my stats to this player to the
	** initialized structure.
	*/
	if (nodaemon) {
	    player.u_slot = 0;
#ifdef	NOASGNSTRUCT
	    bcopy(&player, &players[0], sizeof (struct user));
#else
	    players[0] = player;
#endif
	    me = &players[0];
	}
	else {
	    /* Connect to daemon */
	    ear = getsocket();
/*	    initearsin(DEFAULT_HOST, DEFAULT_PORT);	RMM*/
	    initearsin(DEF_HOST, DEF_PORT);
/*	    calldaemon(&player, DEFAULT_HOST);		RMM*/
	    calldaemon(&player, DEF_HOST);
	}
	/*
	** Put each current player in the bitmaze and draw them in
	** the status area.  Draw status will force a redraw.
	*/
	for (i = 0; i < MAXPLAYER; i++) {
	    if (players[i].u_flag & U_ALIVE)
		addplayer(i, players[i].u_x, players[i].u_y);
	    draw_status(i);
	}
	/* redraw the screen */
	srefresh();
	work();
}

initsigs()
{
	register int sig;
	int quit(), takill();

#ifndef DEBUG
        for (sig = 1; sig < NSIG; sig++) {
		if(sig==SIGALRM) continue;
		signal(sig, SIG_IGN);
	}
#endif

	/* 
	 * We want to interrupt to a screen updating routine every interval.
	 */
	(void) signal(SIGHUP, quit);

	/* Interrupt character flushes type-ahead */
	(void) signal(SIGINT, takill);
	(void) signal(SIGQUIT, quit);
}

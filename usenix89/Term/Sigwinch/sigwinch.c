
/*
 * vis: continuously run a program updating it's output to a window
 * (the screen) giving it an animated effect (or some other description).
 *
 * Written by Dan Heller (argv@sri-spam.arpa)
 * Updates by John Gilmore (hoptoad!gnu)
 *
 * compile: -lcurses -ltermlib
 */
#include <curses.h>
#include <signal.h>
#include <ctype.h>

#define min(a,b) ((a) > (b) ? (b) : (a))

int maxlines;
extern int errno;
extern char *sys_errlist[];
int delay = 0;

main(argc, argv)
char **argv;
{
    register lines, iterations = 0;
    register char c;
    char *cmd, *do_cmd();
    int die();
    FILE *fp;

    if (argc > 1 && isdigit(argv[1][0])) {
	delay = atoi(argv[1]);
	argv++, argc--;
    }

    if (argc < 2)
	printf("Usage: %s [delay] command [args]\n", argv[0]), exit(1);

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)    /* for non-job-ctl & */
        signal(SIGINT, die);
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)    /* for nohup */
        signal(SIGHUP, die);
    signal(SIGTERM, die);

    initscr(); /* initialize curses package */
    clear();
    cmd = do_cmd(argc,argv);
    mvprintw(0,0," Command: %s\n", cmd);
    noecho();  /* don't let typing ruin our painting */
    refresh();	/* Do something now to show we're alive */
    while(1) {
	if (!(fp = popen(cmd, "r")))
	    die(-1);
	mvprintw(0, COLS-15, "exec #%d", ++iterations);
	move(lines = 1, 0);
	while ((c = getc(fp)) != EOF) { /* read the command's output */
	    if (c == '\n')
		clrtoeol(), move(++lines, 0);
	    else addch(c);
	    if (lines == LINES) /* no need to process anymore */
		break;
	}
	pclose(fp);
	/* we've found the end of file, thus, the end of exec */
	getyx(stdscr, maxlines, lines);
	    /* don't need lines..maxlines is where to go on die() */
	clrtobot(); /* clear to bottom of screen */
        move (min (maxlines, LINES - 1), 0); /* for aesthetics */
	refresh();   /* update screen NOW -- other curses calls don't update */
	if (delay)	/* The pause that leaves some CPU for others */
		sleep(delay);
    }
}

die(sig)
{
    if (sig == -1) { /* leave room for perror */
	mvprintw(maxlines = LINES - 2, 0, "error: %s", sys_errlist[errno]);
	clrtoeol();
    }
    move(min(maxlines,LINES - 1), 0);
    clrtoeol(), echo(), refresh();
    exit(sig);
}

char *
do_cmd(argc,argv) /* cat all the args together to one string */
char **argv;
{
    int count = 0;
    static char string[800];
    char *p = string;
    while(++count < argc)
	sprintf(p, "%s ", argv[count]), p += strlen(p);
    return string;
}

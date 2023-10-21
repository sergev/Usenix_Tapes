
/* vis: continuously run a program updating it's output to a window
 * (the screen) giving it an animated effect (or some other description).
 *
 * Written by Dan Heller (argv@sri-spam.arpa)
 *
 * compile: -lcurses -ltermlib
 */
#include <curses.h>
#include <signal.h>
#include <sys/time.h>

#define min(a,b)	((a) > (b) ? (b) : (a))
#define input 		fildes[0]
#define output 		fildes[1]

int maxlines;
extern int errno;
extern char *sys_errlist[];

main(argc, argv)
char **argv;
{
    register lines, iterations = 0;
    register char c;
    char *cmd = argv[0], *do_cmd();
    int die();
    long wait_time = 0;
    FILE *fp;

    signal(SIGQUIT,die);
    signal(SIGINT, die);

    if (argc > 3 && !strcmp(argv[1], "-t")) {
	wait_time = atoi(argv[2]);
	argv += 2, argc -= 2;
    }
    if (argc < 2)
	printf("Usage: %s [-t wait_time] command [args]\n", cmd), exit(1);
    initscr(); /* initialize curses package */
    clear();
    cmd = do_cmd(argc,argv);
    mvprintw(0,0," Command: %s", cmd);
    if (wait_time)
	mvprintw(0, strlen(cmd)+20, "time delay: %d secs.", wait_time);
    noecho();  /* don't let typing ruin our painting */
    while(1) {
	int fildes[2], pid, status;
	refresh();   /* update screen NOW -- other curses calls don't update */
	if (wait_time) {
	    register struct itimerval tmr;
	    timerclear(&(tmr.it_interval));
	    timerclear(&(tmr.it_value));

	    /* get the remaining time */
	    getitimer(ITIMER_REAL, &tmr);
	    if (tmr.it_value.tv_sec > 1000)
		sleep(tmr.it_value.tv_sec - 1000);
	    /* DON'T let the timer expire or the signal will kill you */
	    tmr.it_value.tv_sec = 1000 + wait_time; /* big + argument */
	    setitimer(ITIMER_REAL, &tmr, NULL);
	}
	pipe(fildes);
	if(!(pid = vfork())) { /* fork and exec redirecting output to curses */
	    signal(SIGPIPE, SIG_IGN);   /* if output exceeds screen size */
	    dup2(output, 1); /* stdout will now go thru the pipe */
	    close(input);
	    execvp(argv[1], argv+1); /* do the call */
	    mvprintw(LINES-2, 0, "%s: %s.", argv[1], sys_errlist[errno]);
	    maxlines = LINES - 1;
	    clrtoeol();
	    _exit(-1);	/* use _exit with vfork() */
	}
	if (pid == -1) die(-1);
	if (!(fp = fdopen(input, "r"))) die(-1);
	close(output);
	mvprintw(0, COLS-15, "exec #%d", ++iterations);
	move(lines = 1, 0);
	/* read the command's output */
	while ((c = getc(fp)) != EOF && lines < LINES - 1)
	    if (c == '\n')
		clrtoeol(), move(++lines, 0);
	    else addch(c);
	fclose(fp);
	/* we've found the end of file, thus, the end of exec */
	wait(&status); /* wait for child to die */
	if (status)
	    die(status); /* if child didn't work, status will != 0 */
	getyx(stdscr, maxlines, lines);
	    /* don't need lines..maxlines is where to go on die() */
	clrtobot(); /* clear to bottom of screen */
	move(min(maxlines, LINES-1), 0);
    }
}

die(sig)
{
    move(min(maxlines,LINES - 1), 0);
    clrtoeol(), echo(), refresh(), endwin();
    exit(sig);
}

char *
do_cmd(argc,argv) /* cat all the args together to one string */
char **argv;
{
    int count = 0;
    static char string[80];
    char *p = string;
    while(++count < argc)
	sprintf(p, "%s ", argv[count]), p += strlen(p);
    return string;
}

/* display.c - repeatedly display command through curses
 * vix 18apr86 [written]
 * vix 15dec86 [major overhaul]
 */


#include <curses.h>
#include <signal.h>
#include <ctype.h>


#define		DEFAULT_DELAY	5


static	char	*Command;
static	int	Delay;


main(argc, argv)
int	argc;
char	*argv[];
{
	extern	void	parse_args(),
			die(),
			display();

	parse_args(argc, argv);

	signal(SIGHUP, die);
	signal(SIGINT, die);
	signal(SIGQUIT, die);
	signal(SIGTERM, die);

	initscr();
	clear();

	while (TRUE) {
		display();
		sleep(Delay);
	}
}


static void die()
{
	move(LINES-1, 0);
	clrtoeol();
	refresh();
	endwin();
	exit(0);
}


static void display()
{
	auto	FILE	*fp, *popen();
	auto	char	ch;

	if (!(fp = popen(Command, "r"))) {
		perror("popen");
		exit(1);
	}
	move(0, 0);
	while (EOF != (ch = fgetc(fp)))
	{
		if (ch == '\n')
			clrtoeol();
		addch(ch);
	}
	clrtoeol();
	refresh();
	pclose(fp);
}


static void parse_args(argc, argv)
int	argc;
char	*argv[];
{
	extern	void	usage();
	auto	int	argn,
			delay_found;

	Command = NULL;
	Delay = DEFAULT_DELAY;
	delay_found = FALSE;
	for (argn = 1;  argn < argc;  argn++)
	{
		if (argv[argn][0] == '-')
			if (delay_found)
				usage();	/* already got this once */
			else if (!isdigit(argv[argn][1]))
				usage();	/* not a numeric */
			else
			{
				Delay = atoi(&argv[argn][1]);
				delay_found = TRUE;
			}
		else
			if (Command != NULL)
				usage();	/* already got this once */
			else
				Command = argv[1];
	}
	if (Command == NULL)
		usage();			/* no Command on line */
}


static void usage()
{
	extern	char	*getenv();
	auto	char	*shell = getenv("SHELL");

	fprintf(stderr, "\
usage:  display [-<delay>] <command>\n\
        <delay>   = # of seconds between displays, default=%d\n\
        <command> = command to display, quoted if it contains blanks\n",
		DEFAULT_DELAY);

	if (strcmp(shell, "/bin/sh"))
		fprintf(stderr, "\
	NOTE:  /bin/sh will be used to process the command, not SHELL (%s)\n",
			shell);
	
	exit(1);
}

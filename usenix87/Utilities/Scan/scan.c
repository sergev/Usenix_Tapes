/*
 * scan: 'real time' screen display - by jeffrey greenberg
 * Copyright 1984 Jeffrey Greenberg * ALL RIGHTS RESERVED
 * Source given for non-profit use only! (Please give me credit &
 *	mail me changes!)
 *
 * compile: cc scan.c -lcurses
 * Should run on system 5...
 */
#include <stdio.h>
#include <curses.h>

int seconds;

main(argc, argv)
int argc; char *argv[];
{
	FILE *str;
	int intrp();
	int c;
	int minarg;
	char *execstr, *getopts();
	char *copyright;
	copyright = "Jeffrey Greenberg 1984/85 - ALL RIGHTS RESERVED";

	execstr = getopts(argc,argv);

	initscr();
	signal(2,intrp);
	nonl(); cbreak(); noecho();
	clear();
	refresh();

loop:
	if( (str = popen( execstr, "r")) == NULL )
		panic( execstr );

	move(0,0);
	while( (c=fgetc( str )) != EOF )
		addch(c);
	clrtobot();
	refresh();

	pclose( str );

	if(seconds )
		sleep( seconds );

	goto loop;
}

panic( str )
char *str;
{
	perror( str );
	cleanup();
	exit(1);
}

intrp()
{
	cleanup();
	exit(0);
}

cleanup()
{
	echo(); nl(); nocbreak();
	endwin();
}

char *
getopts( argc, argv )
int argc;
char **argv;
{
	int option, badopt = 0;
	extern int optind;
	extern char *optarg;
	static char exec_default[] = {"ps -af"};
	char *execstr;

	seconds = 4;	/* default sleep time in seconds */

	while( (option=getopt(argc,argv,"s:")) != EOF )
		if( option == 's' ) {
			if( (seconds = atoi(optarg)) < 0)
				++badopt;
		}
		else {
			++badopt;
		}
	if( badopt ) {
		fprintf(stderr,"Usage: scan [-s seconds] [quoted_command_string]\n");
		exit(1);
	}

	if( (argc - 1) < optind )
		execstr = exec_default;
	else
		execstr = argv[optind];

	return execstr;
}

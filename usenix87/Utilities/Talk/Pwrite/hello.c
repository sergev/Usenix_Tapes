
#include "header.h"

int	DEF_LENGTH, DEF_WIDTH;
int	NUM_DOWN, NUM_ACROSS;

hello()
{
	int	a;
	int	y, x;

	/*
	printf("# of lines: "); LINES = atoi(gets(STRING));
	printf("# of cols : "); COLS  = atoi(gets(STRING));
	*/

	initscr();
	crmode(); noecho();

	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT, goodbye);
	/* signal(SIGPIPE, _pipe_); */
	signal(SIGSEGV,_death_);
	signal(SIGBUS, _death_);

	if (*CM && LINES >= 8 && COLS >= 32)
		SMART = 1;
	else
		SMART = 0;

	TIMEOUT.tv_sec =  5;	/* 5 seconds			*/
	TIMEOUT.tv_usec = 0;	/* 0 microseconds		*/

	SCREEN = newwin(LINES,COLS,0,0);
	clearok(SCREEN,0);
	LINE = newwin(1,COLS,LINES-1,0);

	DEF_LENGTH = 8;
	DEF_WIDTH  = (COLS - 8) / 2;
	if (DEF_WIDTH > 32)
		DEF_WIDTH = 32;
	NUM_DOWN   = LINES / DEF_LENGTH;
	NUM_ACROSS = (COLS - 8) / DEF_WIDTH;

	for (a=0; a < MAX_USERS; a++)
	{
		D[a].D_ysize = DEF_LENGTH;
		D[a].D_xsize = DEF_WIDTH;
		D[a].D_y = LINES - DEF_LENGTH;
		D[a].D_x = COLS  - DEF_WIDTH;
	}

	for (x=a=0; x < NUM_ACROSS; x++)
	for (y=0; y < NUM_DOWN; y++,a++)
	{
		D[a].D_y = y * DEF_LENGTH;
		D[a].D_x = x * DEF_WIDTH + 8;
	}

	p_setup_me();
}


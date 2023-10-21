
#include "header.h"

panic(s,n)
char *s;
int n;
{
	if (SMART)
	{
		do_line(s);
	}
	else
	{
		printf("\n%s\n",s);
	}
	fflush(stdout);

	if (n)
		{ sleep(n); }
	else
		{ sleep(5); goodbye(); }
}

do_line(s)
char *s;
{
	int a, l;

	if (SMART)
	{
		wclear(LINE);
		if ((l = strlen(s)) < COLS)
		{
			waddstr(LINE,s);
		}
		else
		{
			for (a=16; a > 0; a--)
				waddch(LINE,s[l-a]);
		}
		if (!*s)
			touchwin(LINE);
		wrefresh(LINE);
	}
	else
	{
		printf("\n%s\n",s);
	}
}

_pipe_()
{
	signal(SIGINT, SIG_IGN);
	if (SMART)
		mvcur(0,COLS-1,LINES-9,0);

	printf("\n");
	printf(".----------------------.\n");
	printf("| THIS UNIVERSE SEEMS  |\n");
	printf("| TO BE DEFECTIVE!     |\n");
	printf("|                      |\n");
	printf("| PLEASE TRY ANOTHER.  |\n");
	printf("|         Press RETURN |\n");
	printf("`----------------------'\n");
	gets(STRING);
	endwin(); exit(0);
}

_death_()
{
	signal(SIGINT, SIG_IGN);
	if (SMART)
		mvcur(0,COLS-1,LINES-9,0);

	printf("\n");
	printf(".----------------------.\n");
	printf("|    .-----* A serious |\n");
	printf("|   _|_   system error |\n");
	printf("|  /   \\  has occurred |\n");
	printf("| | TNT |              |\n");
	printf("|  \\___/  Press RETURN |\n");
	printf("`----------------------'\n");
	gets(STRING);
	endwin(); exit(0);
}

char *enter_line(s)
char *s;
{
	char	e_line[BUFSIZ], c;
	int	a, no_echo;

	if (!SMART)
	{
		printf("\n%s",s);
		echo(); nocrmode();
		gets(e_line);
		noecho(); crmode();
		return(e_line);
	}

	no_echo = 0;
	if (*s == '^')
		{ s++; no_echo = 1; }

	do_line(s);

	a = 0;
	for (a=e_line[0]=0; (c=getchar()) != RET && c != ESC; )
	{
		switch(c)
		{
			case 127:
			case   8:	if (a > 0)
						e_line[--a] = NULL;
					break;
			case  21:	a = 0; e_line[a] = NULL;
					break;
			default:	if (a < BUFSIZ && c >= ' ')
					{
						e_line[a++] = c;
						e_line[a] = NULL;
					}
					break;
		}
		if (!no_echo)
			do_line(e_line);
	}

	if (c == ESC)
		strcpy(e_line,"");

	return(e_line);
}

aslstr(s,n)
char	*s;
int	n;
{
	int	a;

	for (a=0; s[a] = s[a+n]; a++);
}


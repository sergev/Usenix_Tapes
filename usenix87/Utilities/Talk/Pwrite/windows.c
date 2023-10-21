
#include "header.h"

WINDOW	*W_WINDOW;
WINDOW	*H_WINDOW;

w_new_user(n)
int	n;
{
	int	a, b, c, x, y;

	T[n].__win1 = newwin(D[n].D_ysize,D[n].D_xsize,D[n].D_y,D[n].D_x);
	T[n].__win2 = newwin(D[n].D_ysize-3,D[n].D_xsize-3,D[n].D_y+2,D[n].D_x+2);
	if (T[n].__connected)
	{
		box(T[n].__win1,'|','-');
	}
	else
	{
		for (y=0; y < D[n].D_ysize; y++)
		{
			mvwaddch(T[n].__win1,y,0,'.');
			mvwaddch(T[n].__win1,y,D[n].D_xsize-1,'.');
		}

		for (x=0; x < D[n].D_xsize; x+=2)
		{
			mvwaddch(T[n].__win1,D[n].D_ysize-1,x,'-');
			mvwaddch(T[n].__win1,0,x+1,'-');
		}
	}

	mvwaddch(T[n].__win1,0,0,'.');
	mvwaddch(T[n].__win1,D[n].D_ysize-1,0,'`');
	mvwaddch(T[n].__win1,0,D[n].D_xsize-1,'.');
	mvwaddch(T[n].__win1,D[n].D_ysize-1,D[n].D_xsize-1,'\'');

	sprintf(STRING,"%s  %s",T[n].__tty,T[n].__username);
	for (a=0; STRING[a] && a+3 < T[n].__win1->_maxx; a++)
		mvwaddch(T[n].__win1,0,a+2,STRING[a]);

	scrollok(T[n].__win2,1);
	w_setcorners(n);

	if (SMART)
	{
		wrefresh(T[n].__win1);
		wrefresh(T[n].__win2);
	}
	else
	{
		printf("\nUser '%s' on 'tty%s' has joined.%c\n",T[n].__username,T[n].__tty,7);
	}
}

w_setcorners(n)
int	n;
{
	CURRENT_WINDOW = n;
	C[0].C_y = C[1].C_y = D[n].D_y;
	C[0].C_x = C[2].C_x = D[n].D_x;
	C[2].C_y = C[3].C_y = D[n].D_y + D[n].D_ysize-1;
	C[1].C_x = C[3].C_x = D[n].D_x + D[n].D_xsize-1;
}

w_addch(n,c)
int	n, c;
{
	switch(c)
	{
		case BEGIN_SECRET:
				T[n].__secret = 1;
				return(0);
				break;
		case END_SECRET:
				T[n].__secret = 0;
				return(0);
				break;
		case 23:
				p_secretcommand(n,c);
				return(0);
				break;
	}

	if (T[n].__secret)
	{
		if (SNOOPER < 0)
			return(0);
		else
			n = SNOOPER;
	}

	if (SMART)
	{
		if (c == 8)
		{
			w_backspace(n);
		}
		else if (T[n].__win2->_curx == T[n].__win2->_maxx-1)
		{
			if (c == RET || c == SPC)
				waddch(T[n].__win2,RET);
			else
			{
				w_wordwrap(n);
				waddch(T[n].__win2,c);
			}
		}
		else
		{
			waddch(T[n].__win2,c);
		}

		if (n == SNOOPER)
			wrefresh(T[SNOOPER].__win2);
	}
	else
	{
		putchar(c);
	}
}

w_writech(n,c)
int	n, c;
{
	w_addch(n,c);
	if (SMART)
		wrefresh(T[n].__win2);
	else
		fflush(stdout);
}

w_writestr(n,s)
int	n;
char	*s;
{
	for (; *s; w_addch(n,*s++));
	if (SMART)
		wrefresh(T[n].__win2);
	else
		fflush(stdout);
}

w_wordwrap(n)
int	n;
{
	int	c, x, y, s;

	waddch(T[n].__win2,RET);

	y = T[n].__win2->_cury;
	x = T[n].__win2->_maxx - 2;

	for ( ; x > 0 && T[n].__win2->_y[y-1][x] != SPC; x--);

	s = x + 1;

	if (x)
	{
		for (x=0; s < T[n].__win2->_maxx - 1; x++,s++)
		{
			wmove(T[n].__win2,y-1,s);
			c = winch(T[n].__win2);
			waddch(T[n].__win2,SPC);
			mvwaddch(T[n].__win2,y,x,c);
		}
	}
}

w_backspace(n)
int	n;
{
	int	y, x;

	if (!T[n].__win2->_curx && (y = T[n].__win2->_cury))
	{
		wmove(T[n].__win2,y-1,T[n].__win2->_maxx-1);
		for (x=T[n].__win2->_maxx-2; x >= 0; x--)
		{
			wmove(T[n].__win2,y-1,x);
			if (winch(T[n].__win2) != SPC)
				break;
		}
		wmove(T[n].__win2,y-1,x+1);
	}
	else
		waddch(T[n].__win2,8);
}

w_who()
{
	int	a, i, j, x, y;

	p_who();
	y = LINES - 4;
	x = NUM_W / y;
	if (NUM_W % y)
		x++;
	if (NUM_W < y)
		y = NUM_W;
	x *= 16;

	if (SMART)
		W_WINDOW = newwin(y+3,x+4,LINES-(y+4),0);
	else
		W_WINDOW = newwin(y+3,x+4,0,0);

	box(W_WINDOW,'|','-');
	mvwaddch(W_WINDOW,0,0,'.');
	mvwaddch(W_WINDOW,y+2,0,'`');
	mvwaddch(W_WINDOW,0,x+3,'.');
	mvwaddch(W_WINDOW,y+2,x+3,'\'');
	mvwaddstr(W_WINDOW,0,2,"who");

	for (i=j=a=0; a < NUM_W; a++)
	{
		if (p_check(W[a].W_tty,0) != -1)
		{
			wstandout(W_WINDOW);
			mvwaddstr(W_WINDOW,j+2,(i*16)+2,"              <");
		}

		mvwaddstr(W_WINDOW,j+2,(i*16)+2,W[a].W_tty);
		mvwaddstr(W_WINDOW,j+2,(i*16)+6,W[a].W_username);

		wstandend(W_WINDOW);

		j++;
		if (j >= y)
		{
			j = 0;
			i++;
		}
	}

	wrefresh(W_WINDOW);
	delwin(W_WINDOW);
}

w_redraw(n)
int	n;
{
	int	a;

	if (SMART)
	{
		werase(SCREEN);
		for (a=0; a < NUM_T; a++)
		{
			if (!T[a].__dead)
			{
				overwrite(T[a].__win1,SCREEN);
				overwrite(T[a].__win2,SCREEN);
			}
		}

		if (n)
			touchwin(SCREEN);

		wrefresh(SCREEN);
	}
}

w_help()
{
	if (SMART)
		H_WINDOW = newwin(14,40,LINES-15,COLS-40);
	else
		H_WINDOW = newwin(14,40,0,0);

	wprintw(H_WINDOW,".-help---------------------------------.");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"| Talk-mode commands:                  |");
	wprintw(H_WINDOW,"| ===================                  |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|   ^L   - redraw screen               |");
	wprintw(H_WINDOW,"|   ^D   - exit pwrite                 |");
	wprintw(H_WINDOW,"|   ESC  - enter Command Mode          |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"`-Press SPACE for more-----------------'");
	wrefresh(H_WINDOW);
	while (getchar() != SPC);

	wclear(H_WINDOW);
	wprintw(H_WINDOW,".-help---------------------------------.");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"| Command Mode commands:               |");
	wprintw(H_WINDOW,"| ======================               |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|   a    - add a user                  |");
	wprintw(H_WINDOW,"|   d    - delete a user               |");
	wprintw(H_WINDOW,"|   w    - see who is on the system    |");
	wprintw(H_WINDOW,"|   r    - ring all unanswered users   |");
	wprintw(H_WINDOW,"|   ^L   - redraw screen               |");
	wprintw(H_WINDOW,"|   ESC  - return to Talk Mode         |");
	wprintw(H_WINDOW,"|   ?    - show these help pages       |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"`-Press SPACE for more-----------------'");
	wrefresh(H_WINDOW);
	while (getchar() != SPC);

	wclear(H_WINDOW);
	wprintw(H_WINDOW,".-help---------------------------------.");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|   Moving windows around:             |");
	wprintw(H_WINDOW,"|   ----------------------             |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"|   h j k l - move the four border     |");
	wprintw(H_WINDOW,"|             corners left, down, up,  |");
	wprintw(H_WINDOW,"|             or right.                |");
	wprintw(H_WINDOW,"|   H J K L - change size.             |");
	wprintw(H_WINDOW,"|   SPACE   - select another window.   |");
	wprintw(H_WINDOW,"|   @       - move window to fit       |");
	wprintw(H_WINDOW,"|             within the four corners. |");
	wprintw(H_WINDOW,"|                                      |");
	wprintw(H_WINDOW,"`-Press SPACE--------------------------'");
	wrefresh(H_WINDOW);
	while (getchar() != SPC);
	delwin(H_WINDOW);
	w_redraw(1);
}

w_corners(n)
int	n;
{
	int	a;

	if (!SMART)
		return(0);

	if (n)
	{
		for (a=0; a < 4; a++)
		{
			wmove(curscr,C[a].C_y,C[a].C_x);
			C[a].C_under = winch(curscr);
			wmove(SCREEN,C[a].C_y,C[a].C_x);
			wstandout(SCREEN);
			if (a % 2)
				waddch(SCREEN,']');
			else
				waddch(SCREEN,'[');
			wstandend(SCREEN);
		}
	}
	else
	{
		for (a=0; a < 4; a++)
		{
			wmove(SCREEN,C[a].C_y,C[a].C_x);
			waddch(SCREEN,C[a].C_under);
		}
	}

	wrefresh(SCREEN);
}

w_mvcorners(y,x,j,i)
int	y, x, j, i;
{
	if (!SMART)
		return(0);

	C[0].C_y += y; C[0].C_x += x;
	C[1].C_y += y; C[1].C_x += i;
	C[2].C_y += j; C[2].C_x += x;
	C[3].C_y += j; C[3].C_x += i;

	if (C[0].C_y < 0 || C[0].C_y >= (LINES - (C[2].C_y - C[0].C_y)))
	{
		C[0].C_y -= y; C[1].C_y -= y;
	}
	if (C[0].C_x < 0 || C[0].C_x >= (COLS  - (C[1].C_x - C[0].C_x)))
	{
		C[0].C_x -= x; C[2].C_x -= x;
	}
	if (C[3].C_y < (C[0].C_y+4) || C[3].C_y >= LINES)
	{
		C[2].C_y -= j; C[3].C_y -= j;
	}
	if (C[3].C_x < (C[0].C_x+7) || C[3].C_x >= COLS)
	{
		C[1].C_x -= i; C[3].C_x -= i;
	}
}

w_change()
{
	if (!SMART)
		return(0);

	delwin(T[CURRENT_WINDOW].__win1);
	delwin(T[CURRENT_WINDOW].__win2);
	D[CURRENT_WINDOW].D_ysize = C[2].C_y - C[0].C_y + 1;
	D[CURRENT_WINDOW].D_xsize = C[1].C_x - C[0].C_x + 1;
	D[CURRENT_WINDOW].D_y = C[0].C_y;
	D[CURRENT_WINDOW].D_x = C[0].C_x;
	w_new_user(CURRENT_WINDOW);
	w_redraw(1);
}

w_nextcorners()
{
	if (!SMART)
		return(0);

	do
	{
		if (++CURRENT_WINDOW >= NUM_T)
			CURRENT_WINDOW = 0;
	}
	while (T[CURRENT_WINDOW].__dead);
	w_setcorners(CURRENT_WINDOW);
}


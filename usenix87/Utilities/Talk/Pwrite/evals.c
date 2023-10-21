
#include "header.h"

eval_key(c)
char	c;
{
	int	a, k;

	switch(c)
	{
		case   4:
				goodbye();
				break;
		case  12:
				wrefresh(curscr);
				break;
		case  18:
				w_redraw(0);
				break;
		case  23:
				if (SNOOPER >= 0)
					p_sendch(c);
				break;
		case ESC:
				eval_esc();
				break;
		case 127:
		case   8:
				w_writech(0,'\b');
				p_sendch('\b');
				break;
		default:
				if (c >= ' ' || c == RET)
				{
					w_writech(0,c);
					p_sendch(c);
				}
				break;
	}
}

eval_esc()
{
	int	c;

	overwrite(curscr,SCREEN);
	w_setcorners(CURRENT_WINDOW);
	sprintf(STRING,"%c\n\"%s\" --> COMM-MODE.%c",BEGIN_SECRET,T[0].__tty,END_SECRET);
	p_sendstr(STRING);
	w_corners(1);
	do_line("Command Mode. Press ESC to return to Talk Mode.");

	while ((c = getchar()) != ESC)
	{
		w_corners(0);
		switch(c)
		{
			case  12:
					wrefresh(curscr);
					break;
			case  18:
					w_redraw(0);
					break;
			case 'w':
					w_who();
					break;
			case 'a':
					e_call();
					break;
			case 'd':
					e_delete();
					break;
			case 'r':
					e_ring();
					break;
			case '?':
					w_help();
					break;
			case 'h':
					w_mvcorners( 0,-2, 0,-2);
					break;
			case 'j':
					w_mvcorners( 1, 0, 1, 0);
					break;
			case 'k':
					w_mvcorners(-1, 0,-1, 0);
					break;
			case 'l':
					w_mvcorners( 0, 2, 0, 2);
					break;
			case 'H':
					w_mvcorners( 0, 0, 0,-2);
					break;
			case 'J':
					w_mvcorners( 0, 0, 1, 0);
					break;
			case 'K':
					w_mvcorners( 0, 0,-1, 0);
					break;
			case 'L':
					w_mvcorners( 0, 0, 0, 2);
					break;
			case SPC:
					w_nextcorners();
					break;
			case '@':
					w_change();
					break;
		}
		w_corners(1);
		do_line("Command Mode. Press ESC to return to Talk Mode.");
	}

	w_corners(0);
	w_redraw(1);
	sprintf(STRING,"%c\n\"%s\" <-- COMM-MODE.%c",BEGIN_SECRET,T[0].__tty,END_SECRET);
	p_sendstr(STRING);
	sock_io();
}

e_call()
{
	int	a;

	w_who();
	strcpy(STRING,enter_line("Which user do you want? "));
	if (*STRING)
	{
		if (!strcmp(STRING,"**"))
		{
			if (p_check(STRING,0) < 0)
			{
				SNOOPER = p_add("**","Super Snooper",-1,0);
				w_new_user(SNOOPER);
				sprintf(STRING,"%c\n\"%s\" ADD \"**\" %c",BEGIN_SECRET,T[0].__tty,END_SECRET);
				p_sendstr(STRING);
			}
			else
			{
				panic("How many Super Snoopers do you want?!?!?",2);
			}
		}
		else if ((a = p_check(STRING,1)) < 0)
		{
			panic("That user is not on the system.",2);
		}
		else
		{
			if (p_check(STRING,0) < 0)
			{
				a = p_add(W[a].W_tty,W[a].W_username,-1,0);
				w_new_user(a);
				sprintf(STRING,"%c\n\"%s\" ADD \"%s\".%c",BEGIN_SECRET,T[0].__tty,T[a].__tty,END_SECRET);
				p_sendstr(STRING);
			}
			else
			{
				panic("That user is already on pwrite.",2);
			}
		}
	}
	w_redraw(0);
}

e_delete()
{
	int	a;

	w_who();
	strcpy(STRING,enter_line("Which user do you want to delete? "));
	if (*STRING)
	{
		if ((a = p_check(STRING,0)) < 0)
		{
			panic("That user is not on pwrite.",2);
		}
		else
		{
			sprintf(STRING,"%c\n\"%s\" DEL \"%s\".%c",BEGIN_SECRET,T[0].__tty,T[a].__tty,END_SECRET);
			p_delete(a);
			p_sendstr(STRING);
		}
	}

	w_redraw(0);
}

e_ring()
{
	int  a;

	for (a=1; a < NUM_T; a++)
	{
		if (!T[a].__dead && !T[a].__connected)
		{
			sprintf(STRING,"/dev/tty%s",T[a].__tty);
			if ((fp = fopen(STRING,"w")) != NULL)
			{
				fprintf(fp,"\n\n\n%c",7);
				fprintf(fp,"pwrite: User '%s' on 'tty%s' wishes to talk to you using 'pwrite'.\n",T[0].__username,T[0].__tty);
				fprintf(fp,"pwrite: Please respond using: '~puchyr/com/pwrite' and wait 10 seconds.\n");
				fprintf(fp,"\n\n%c",7);
				fflush(fp);
				fclose(fp);
				w_writestr(a,"[RING]  ");
			}
			else
			{
				w_writestr(a,"[NO RING]  ");
			}
		}
	}
}


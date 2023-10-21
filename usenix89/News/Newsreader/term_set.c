#include <stdio.h>
#include "tty.h"
#include "vn.h"

extern int L_allow, C_allow;

static outc (c)
char c;
{
	putchar (c);
}

/*
	term_set controls terminal through termcap
	START sets global parameters related to terminal also,
	as well as allocating display buffer which depends on
	terminal lines, and allocating escape strings.
*/

/*
** Escape strings.
*/
static char *Cm,*Cl,*So,*Se,*Te,*Bc,*Ce;
static int Backspace;		/* backspace works */
static int Overstrike;

static t_setup()
{
	int i;
	char *tgetstr(), *getenv(), *str_store();
	char *c, tc_buf[2048],optstr[2048];

	c = optstr;
	if (tgetent(tc_buf,getenv("TERM")) != 1)
		printex ("%s - unknown terminal",getenv("TERM"));

	/* get needed capabilities */
	Cm = str_store(tgetstr("cm",&c));
	Cl = str_store(tgetstr("cl",&c));
	So = str_store(tgetstr("so",&c));
	Se = str_store(tgetstr("se",&c));
	Te = str_store(tgetstr("te",&c));
	Bc = str_store(tgetstr("bc",&c));
	Ce = str_store(tgetstr("ce",&c));
	Backspace = tgetflag("bs");
	Overstrike = tgetflag("os");
	if ( *Cm == '\0' || *Cl == '\0')
	{
		printex ("cursor control and erase capability needed");
	}
	if (Overstrike)
		fgprintf ("WARNING: terminal overstrikes - can't update display without erase\n");
	i = RECBIAS+1 < HHLINES+2 ? HHLINES+2 : RECBIAS+1;
	if ((L_allow = tgetnum("li")) < i)
	{
		if (L_allow < 0)
			printex ("can't determine number of lines on terminal");
		printex ("too few lines for display - %d needed", i);
	}

	/*
	** C_allow set so as to not use extreme right column.
	** Avoids "bad wraparound" problems - we're deciding it's best
	** to ALWAYS assume no automargin, and take care of it ourselves
	*/
	if((C_allow = tgetnum("co")) > MAX_C)
		C_allow = MAX_C;
	else
		--C_allow;
	if (C_allow < MIN_C)
	{
		if (C_allow < 0)
			printex("can't determine number of columns on terminal.");
		printex ("too few columns for display - %d needed",MIN_C);
	}

	L_allow -= RECBIAS;
	page_alloc();
	tputs(tgetstr("ti",&c),1,outc);
}

/* VARARGS */
term_set(cmd,x,y)
int cmd,x,y;
{
	char *tgoto();
	int i;
	switch (cmd)
	{
	case MOVE:
		tputs (tgoto(Cm,x,y),1,outc);
		break;
	case ERASE:
		tputs(Cl,1,outc);
		break;
	case ONREVERSE:
		tputs(So,1,outc);
		break;
	case OFFREVERSE:
		tputs(Se,1,outc);
		break;
	case START:
		t_setup();
		break;
	case STOP:
		term_set (MOVE,0,L_allow+RECBIAS-1);
		printf ("\n");
		tputs(Te,1,outc);
		break;
	case RUBSEQ:
		if (Overstrike)
		{
			/* space overprint is futile */
			if (Backspace)
				putchar('\010');
			else
				tputs(Bc,1,outc);
			break;
		}
		if (Backspace)
			printf("%c %c",'\010','\010');
		else
		{
			tputs(Bc,1,outc);  
			putchar(' ');  
			tputs(Bc,1,outc);
		}
		break;
	case ZAP:
		if (Ce != NULL && *Ce != NULL)
			tputs(Ce,1,outc);
		else
		{
			if (Overstrike)
				break;		/* punt */
			for (i=x; i < y; ++i)
				putchar(' ');
			if (Backspace)
			{
				for (i=x; i < y; ++i)
					putchar('\010');
			}
			else
			{
				for (i=x; i < y; ++i)
					tputs(Bc,1,outc);
			}
		}
		break;
	default:
		printex ("term_set unknown code (%d)",cmd);
		break;
	}
	return (0);
}

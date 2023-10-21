/*
** vn news reader.
**
** term_set.c - terminal control, hides termcap interface
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include "tty.h"
#include "vn.h"

extern int L_allow, C_allow;
extern char *Ku, *Kd, *Kl, *Kr;	

static outc (c)
char c;
{
	putchar (c);
}

/*
	term_set controls terminal through termcap
	START sets global parameters related to terminal also,
	as well as allocating display buffer which depends on
	terminal lines, and allocating escape strings.  RESTART
	simply re-issues the initialization - used following system
	calls that could have goofed up the terminal state.
*/

/*
** Escape strings.
*/

static char *Cm,*Cl,*So,*Se,*Te,*Bc,*Ce,*Ti,*Ks,*Ke;
#ifdef USEVS
static char *Vs,*Ve;
#endif

static int Backspace;		/* backspace works */
static int Overstrike;		/* terminal overstrikes */

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
	Ti = str_store(tgetstr("ti",&c));
	Bc = str_store(tgetstr("bc",&c));
	Ce = str_store(tgetstr("ce",&c));
	Kd = str_store(tgetstr("kd",&c));
	Ke = str_store(tgetstr("ke",&c));
	Kl = str_store(tgetstr("kl",&c));
	Kr = str_store(tgetstr("kr",&c));
	Ks = str_store(tgetstr("ks",&c));
	Ku = str_store(tgetstr("ku",&c));
#ifdef USEVS
	Vs = str_store(tgetstr("vs",&c));
	Ve = str_store(tgetstr("ve",&c));
#endif
	Backspace = tgetflag("bs");
	Overstrike = tgetflag("os");

	if ( *Cm == '\0' || *Cl == '\0')
	{
		printex ("cursor control and erase capability needed");
	}

	/*
	** Checks for arrow keys which don't issue something beginning
	** with <ESC>.  This is more paranoid than we need to be, strictly
	** speaking - we could get away with any string which didn't
	** conflict with controls used for commands.  However, that would
	** be a maintenance headache - we will simply reserve <ESC> as the
	** only char not to be used for commands, and punt on terminals
	** which don't send reasonable arrow keys.  It would be confusing
	** to have keys work partially, also.  I know of no terminal with
	** one arrow key beginning with an escape, and another beginning
	** with something else, but let's be safe.  This also insists on
	** definitions for all 4 arrows, which seems reasonable.
	*/

	if ((*Ku != '\0' && *Ku != '\033') || *Kl != *Ku || *Kr != *Ku || *Kd != *Ku)
	{
		fgprintf("WARNING: arrow keys will not work for this terminal");
		Ku = Kd = Kl = Kr = Kd = Ke = "";
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
	tputs(Ti,1,outc);
	tputs(Ks,1,outc);
#ifdef USEVS
	tputs(Vs,1,outc);
#endif
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
	case RESTART:
		tputs(Ti,1,outc);
		tputs(Ks,1,outc);
#ifdef USEVS
		tputs(Vs,1,outc);
#endif
		break;
	case STOP:
		term_set (MOVE,0,L_allow+RECBIAS-1);
		printf ("\n");
		tputs(Ke,1,outc);
		tputs(Te,1,outc);
#ifdef USEVS
		tputs(Ve,1,outc);
#endif
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
		if (Ce != NULL && *Ce != '\0')
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

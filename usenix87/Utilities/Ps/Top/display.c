/*
 *  Top - a top users display for Berkeley Unix
 *
 *  This file contains the routines that display information on the screen.
 *  Each section of the screen has two routines:  one for initially writing
 *  all constant and dynamic text, and one for only updating the text that
 *  changes.  The prefix "i_" is used on all the "initial" routines and the
 *  prefix "u_" is used for all the "updating" routines.  NOTE:  it is
 *  assumed that none of the "i_" routines use any of the termcap
 *  capabilities.  In this way, those routines can be safely used on
 *  terminals that have minimal (or nonexistant) terminal capabilities.
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/dk.h>
#include "screen.h"		/* interface to screen package */
#include "layout.h"		/* defines for screen position layout */
#include "top.h"
#include "boolean.h"

static int lmpid = 0;
static struct user u;

char *printable();

/* Verbose process state names */

char *state_name[] =
{
    "", "sleeping", "ABANDONED", "running", "starting", "zombie", "stopped"
};

/* process state names for the "STATE" column of the display */

char *state_abbrev[] =
{
    "", "sleep", "WAIT", "run", "start", "zomb", "stop"
};

/* cpu state names for percentages */

char *cpu_state[] =
{
    "user", "nice", "system", "idle"
};

/* screen positions for cpustate figures */
char x_cpustates[] = { 12, 24, 36, 50 };

i_loadave(mpid, avenrun)

int mpid;
#ifdef sun
long *avenrun;
#else
double *avenrun;
#endif sun

{
    register int i;

    printf("last pid: %5d;  load averages", mpid);

    for (i = 0; i < 3; i++)
    {
	printf("%c %4.2f",
	    i == 0 ? ':' : ',',
#ifdef sun
	    (double)avenrun[i] / FSCALE);
#else
	    avenrun[i]);
#endif
    }
    lmpid = mpid;
}

u_loadave(mpid, avenrun)

int mpid;
#ifdef sun
long *avenrun;
#else
double *avenrun;
#endif sun

{
    register int i;

    if (mpid != lmpid);
    {
	Move_to(x_lastpid, y_lastpid);
	printf("%5d", mpid);
	lmpid = mpid;
    }

    Move_to(x_loadave, y_loadave);
    for (i = 0; i < 3; i++)
    {
	printf("%s%4.2f",
	    i == 0 ? "" : ", ",
#ifdef sun
	    (double)avenrun[i] / FSCALE);
#else
	    avenrun[i]);
#endif
    }
}

static int ltotal = 0;
static int lbrkdn[7];

i_procstates(total, brkdn)

int total;
int *brkdn;

{
    register int i;

    printf("%2d processes", total);	/* ??? */
    ltotal = total;
    for (i = 1; i < 7; i++)
    {
	if (brkdn[i] != 0)
	{
	    printf("%c %d %s%s",
		    i == 1 ? ':' : ',',
		    brkdn[i],
		    state_name[i],
		    (i == SZOMB) && (brkdn[i] > 1) ? "s" : "");
	}
    }
    bcopy(brkdn, lbrkdn, sizeof(lbrkdn));
}

u_procstates(total, brkdn)

int total;
int *brkdn;

{
    register int i;

    if (ltotal != total)
    {
	Move_to(x_procstate, y_procstate);
	printf("%d ", total);
	ltotal = total;
    }
    else if (bcmp(brkdn, lbrkdn, sizeof(lbrkdn)) == 0)
    {
	return;
    }

    Move_to(x_brkdn, y_brkdn);
    for (i = 1; i < 7; i++)
    {
	if (brkdn[i] != 0)
	{
	    printf("%s%d %s%s",
		    i == 1 ? "" : ", ",
		    brkdn[i],
		    state_name[i],
		    (i == SZOMB) && (brkdn[i] > 1) ? "s" : "");
	}
    }
    putcap(clear_line);
    bcopy(brkdn, lbrkdn, sizeof(lbrkdn));
}

i_cpustates(changes, total)

int *changes;
int total;

{
    register int i;

    printf("\nCpu states: ");
    for (i = 0; i < CPUSTATES; i++)
    {
	printf("%s%4.1f%% %s",
		i == 0 ? "" : ", ",
		((float)changes[i] / (float)total) * 100.0,
		cpu_state[i]);
    }
    printf("\n");
}

u_cpustates(changes, total)

int *changes;
int total;

{
    register int i;

    for (i = 0; i < CPUSTATES; i++)
    {
	Move_to(x_cpustates[i], y_cpustates);
	printf("%4.1f",
		((float)changes[i] / (float)total) * 100.0);
    }
}

z_cpustates()

{
    register int i;

    printf("\nCpu states: ");
    for (i = 0; i < CPUSTATES; i++)
    {
	printf("%s    %% %s", i == 0 ? "" : ", ", cpu_state[i]);
    }
    printf("\n");
}

i_memory(i1, i2, i3, i4, i5)

int i1, i2, i3, i4, i5;

{
    printf("Memory: %4dK (%4dK) real, %4dK (%4dK) virtual, %4dK free",
	i1, i2, i3, i4, i5);
}

u_memory(i1, i2, i3, i4, i5)

int i1, i2, i3, i4, i5;

{
    Move_to(x_realmem, y_mem);
    printf("%4dK (%4d", i1, i2);
    Move_to(x_virtmem, y_mem);
    printf("%4dK (%4d", i3, i4);
    Move_to(x_free, y_mem);
    printf("%4d", i5);
}

i_header(f2)

char *f2;

{
    printf(
      "\n\n  PID %s PRI NICE   SIZE   RES STATE   TIME   WCPU    CPU COMMAND", 
      f2);
}

u_header()

{
    Move_to(0, y_header);
}

#ifdef sun
#define percent_cpu(pp) ((double)(pp)->p_pctcpu / FSCALE)
#else
#define percent_cpu(pp) ((pp)->p_pctcpu)
#endif

#define weighted_cpu(pct, pp) ((pp)->p_time == 0 ? 0.0 : \
			 ((pct) / (1.0 - exp((pp)->p_time * logcpu))))

#define Proc_format "%5d %-8.8s %3d %4d%6dK %4dK %-5s%4d:%02d %5.2f%% %5.2f%% %.14s"

#ifdef DEBUG
FILE *debug;
#endif

i_process(line, pp, get_userid)

int line;
struct proc *pp;
char *(*get_userid)();

{
    register long cputime;
    register double pctcpu;
    register char *thisline;
    int len;

#ifdef DEBUG
    debug = fopen("debug", "w");
#endif
    /* calculate a pointer to the buffer for this line */
    thisline = screenbuf[line];

    /* get the cpu usage and calculate the cpu percentages */
    cputime = get_ucpu(pp);
    pctcpu = percent_cpu(pp);

    /* format the line */
    sprintf(thisline, Proc_format,
	pp->p_pid,
	(*get_userid)(pp->p_uid),
	pp->p_pri - PZERO,
	pp->p_nice - NZERO,
#ifdef pyr
	pagetok(pp->p_tsize + pp->p_dsize + pp->p_cssize + pp->p_ussize),
#else
	pagetok(pp->p_tsize + pp->p_dsize + pp->p_ssize),
#endif
	pagetok(pp->p_rssize),
	state_abbrev[pp->p_stat],
	cputime / 60l,
	cputime % 60l,
	100.0 * weighted_cpu(pctcpu, pp),
	100.0 * pctcpu,
	printable(u.u_comm));

    /* write the line out */
    putchar('\n');
    fputs(thisline, stdout);

    /* zero fill the rest of it */
    len = strlen(thisline);
    bzero(thisline + len, Display_width - len);
}

static int lastline = 0;

u_process(line, pp, get_userid)

int line;
struct proc *pp;
char *(*get_userid)();

{
    register char *optr;
    register char *nptr;
    register int ch;
    register int diff;
    register int newcol = 1;
    register int lastcol = 0;
    register long cputime;
    register double pctcpu;
    char cursor_on_line = No;
    char *thisline;
    int screen_line = line + Header_lines;
    static char newline[Display_width];

    /* get a pointer to the old text for this line */
    optr = thisline = screenbuf[line];

    /* get the cpu usage and calculate the cpu percentages */
    cputime = get_ucpu(pp);
    pctcpu = percent_cpu(pp);

    /* format the line */
    sprintf(newline, Proc_format,
	pp->p_pid,
	(*get_userid)(pp->p_uid),
	pp->p_pri - PZERO,
	pp->p_nice - NZERO,
#ifdef pyr
	pagetok(pp->p_tsize + pp->p_dsize + pp->p_cssize + pp->p_ussize),
#else
	pagetok(pp->p_tsize + pp->p_dsize + pp->p_ssize),
#endif
	pagetok(pp->p_rssize),
	state_abbrev[pp->p_stat],
	cputime / 60l,
	cputime % 60l,
	100.0 * weighted_cpu(pctcpu, pp),
	100.0 * pctcpu,
	printable(u.u_comm));

    /* compare the two strings and only rewrite what has changed */
    nptr = newline;
#ifdef DEBUG
    fputs(optr, debug);
    fputc('\n', debug);
    fputs(nptr, debug);
    fputs("\n-\n", debug);
#endif

    /* start things off on the right foot		    */
    /* this is to make sure the invariants get set up right */
    if ((ch = *nptr++) != *optr)
    {
	if (screen_line - lastline == 1)
	{
	    putchar('\n');
	}
	else
	{
	    Move_to(0, screen_line);
	}
	cursor_on_line = Yes;
	putchar(ch);
	*optr = ch;
	lastcol = 1;
    }
    optr++;

    /*
     *  main loop -- check each character.  If the old and new aren't the
     *	same, then update the display.  When the distance from the current
     *	cursor position to the new change is small enough, the characters
     *	that belong there are written to move the cursor over.
     *
     *	Invariants:
     *	    lastcol is the column where the cursor currently is sitting
     *		(always one beyond the end of the last mismatch).
     */
    do		/* yes, a do...while */
    {
	if ((ch = *nptr++) != *optr)
	{
	    /* new character is different from old	  */
	    /* put the new character in the screen buffer */
	    *optr = ch;

	    /* make sure the cursor is on top of this character */
	    diff = newcol - lastcol;
	    if (diff > 0)
	    {
		/* some motion is required--figure out which is shorter */
		if (diff < 6 && cursor_on_line)
		{
		    /* overwrite old stuff--get it out of the screen buffer */
		    printf("%.*s", diff, &thisline[lastcol]);
		}
		else
		{
		    /* use cursor addressing */
		    Move_to(newcol, screen_line);
		    cursor_on_line = Yes;
		}
		/* remember where the cursor is */
		lastcol = newcol + 1;
	    }
	    else
	    {
		/* already there, update position */
		lastcol++;
	    }

	    /* write what we need to */
	    if (ch == '\0')
	    {
		/* at the end--terminate with a clear-to-end-of-line */
		putcap(clear_line);
	    }
	    else
	    {
		/* write the new character */
		putchar(ch);
	    }
	}

	/* update working column and screen buffer pointer */
	newcol++;
	optr++;

    } while (ch != '\0');

    /* zero out the rest of the line buffer -- MUST BE DONE! */
    bzero(optr, Display_width - newcol);

    /* remember where the current line is */
    if (cursor_on_line)
    {
	lastline = screen_line;
    }
}

static int last_hi = 0;

u_endscreen(hi)

register int hi;

{
    register int screen_line = hi + Header_lines;

    if (smart_terminal)
    {
	if (hi < last_hi)
	{
	    if (hi == 0)
	    {
		putchar('\n');
		putchar('\n');
		putcap(clear_line);
		putchar('\n');
	    }
	    else if (screen_line - lastline == 1)
	    {
		putchar('\n');
	    }
	    else
	    {
		Move_to(0, screen_line);
	    }
    
	    while (--last_hi > hi)
	    {
		putcap(clear_line);
		putchar('\n');
	    }
	    putcap(clear_line);
	}
	else
	{
	    last_hi = hi;
	}

	/* move the cursor to a pleasant place */
	Move_to(x_idlecursor, y_idlecursor);
    }
    else
    {
	/* separate this display from the next with some vertical room */
	fputs("\n\n", stdout);
    }
}

/*
 *  get_ucpu(pp) - retrieve the user structure associated with the proc
 *	structure pointed to by pp and return the cpu usage.  The user
 *	structure is stored in the global structure "u" for later use.
 */

get_ucpu(pp)

struct proc *pp;

{
    if (getu(pp, &u) == -1)
    {
	strcpy(u.u_comm, "<swapped>");
	return(0);
    }
    else
    {
	/* set u_comm for system processes */
	if (u.u_comm[0] == '\0')
	{
	    if (pp->p_pid == 0)
	    {
		strcpy(u.u_comm, "Swapper");
	    }
	    else if (pp->p_pid == 2)
	    {
		strcpy(u.u_comm, "Pager");
	    }
	}

#ifdef FOUR_ONE
	return((int)((float)(u.u_vm.vm_utime + u.u_vm.vm_stime)/hz));
#else
	return(u.u_ru.ru_utime.tv_sec + u.u_ru.ru_stime.tv_sec);
#endif
    }
}

/*
 *  printable(str) - make the string pointed to by "str" into one that is
 *	printable (i.e.: all ascii), by converting all non-printable
 *	characters into '?'.  Replacements are done in place and a pointer
 *	to the original buffer is returned.
 */

char *printable(str)

char *str;

{
    register char *ptr;
    register char ch;

    ptr = str;
    while ((ch = *ptr) != '\0')
    {
	if (!isprint(ch))
	{
	    *ptr = '?';
	}
	ptr++;
    }
    return(str);
}

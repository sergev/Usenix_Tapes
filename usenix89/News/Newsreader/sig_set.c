#include <stdio.h>
#include <sys/signal.h>
#include <sgtty.h>
#include <setjmp.h>
#include "tty.h"
#include "vn.h"

extern int L_allow;
extern char *Version;

static int Sigflag=BRK_INIT;	/* phase of interaction */
static FILE **Fpseek;		/* article reading file pointer pointer */
static int Foreground;
static jmp_buf Jumploc;		/* for BRK_SESS phase */
static char *Cur_scn;		/* current group name being scanned */

/*
	interrupt handler - unusual termination (longjmp and printex aborts)
	if not abort, remember to reset signal trap
	CAUTION - the passing of a jump buffer is a little dicey - assumes
	type jump_buf is an array.

	sigcatch and sig_set control a lot of i/o on stderr also, since
	it is so intimately related to signal interaction.  Note that the
	SIGTSTP action causes a "stopped on tty output" if raw terminal
	mode is restored by tty_set(RESTORE).  We don't get it if we were
	already cooked since tty_set avoids calling ioctl if it doesn't
	have to.
*/
static sigcatch (sig)
int sig;
{
	char buf [MAX_C+1];
	int pgrp;

	/* disable signal while processing it */
	signal (sig,SIG_IGN);

	switch (sig)
	{
	case SIGINT:
	case SIGQUIT:
		break;
	case SIGTSTP:
		/* ignore SIGTTOU so we don't get stopped if [kc]sh grabs the tty */
		signal(SIGTTOU, SIG_IGN);
		tty_set (SAVEMODE);
		term_set (MOVE,0,L_allow+RECBIAS-1);
		printf ("\n");
		Foreground = 0;
		fflush (stdout);
		fflush (stderr);
		signal(SIGTTOU, SIG_DFL);

		/* Send the TSTP signal to suspend our process group */
		signal(SIGTSTP, SIG_DFL);
		sigsetmask(0);
		kill (0, SIGTSTP);

		/* WE ARE NOW STOPPED */

		/*
				WELCOME BACK!
				if terminals process group is ours, we are foregrounded again
				and can turn newsgroup name printing back on
			*/
		tty_set (RESTORE);
		switch (Sigflag)
		{
		case BRK_SESS:
			signal (SIGTSTP,sigcatch);
			longjmp (Jumploc,1);
		case BRK_IN:
			ioctl (1,TIOCGPGRP,&pgrp);
			if (pgrp == getpgrp(0))
			{
				Foreground = 1;
				if (Cur_scn != NULL)
					fgprintf ("    %s\n",Cur_scn);
			}
			break;
		default:
			break;
		}
		signal (SIGTSTP,sigcatch);
		return;
	default:
		printex (BRK_MSG,sig);
	}

	/* QUIT and INTERRUPT signals */
	switch (Sigflag)
	{
	case BRK_SESS:
		/* if in session, ask if really a quit, do longjump if not */
		term_set (ERASE);
		tty_set (RAWMODE);
		user_str (buf, BRK_PR);
		if (buf[0] == 'y')
			printex (BRK_MSG,sig);
		signal (sig,sigcatch);
		longjmp (Jumploc,1);
	case BRK_READ:
		/* if reading seek file to end to abort page printing */
		printf ("\n");
		if (*Fpseek == NULL || fseek(*Fpseek,0L,2) < 0)
			putchar ('\07');
		break;
	default:
		printex (BRK_MSG,sig);
	}
	signal (sig,sigcatch);
}

/*
	sig_set controls what will be done with a signal when picked up by
	sigcatch.  grp_indic / fgprintf is included here to keep knowledge
	of TSTP state localized.
*/
/* VARARGS */
sig_set (flag,dat)
int flag, *dat;
{
	int i, *xfer, pgrp;
	if (Sigflag == BRK_INIT)
	{
		Cur_scn = NULL;
		signal (SIGINT,sigcatch);
		signal (SIGQUIT,sigcatch);
		signal (SIGHUP,sigcatch);
		signal (SIGTERM,sigcatch);
		signal (SIGTSTP,sigcatch);
		ioctl (1,TIOCGPGRP,&pgrp);
		if (pgrp == getpgrp(0))
		{
			Foreground = 1;
			fgprintf ("Visual News, Release %s, reading:\n",Version);
		}
		else
			Foreground = 0;
	}
	switch (flag)
	{
	case BRK_IN:
	case BRK_OUT:
		Sigflag = flag;
		break;
	case BRK_READ:
		if (Sigflag != BRK_SESS)
			printex ("unexpected read state, sig_set\n");
		Fpseek = (FILE **) dat;
		Sigflag = BRK_READ;
		break;
	case BRK_SESS:
		xfer = (int *) Jumploc;
		for (i=0; i < sizeof(Jumploc) / sizeof(int); ++i)
			xfer[i] = dat[i];
		Sigflag = BRK_SESS;
		break;
	case BRK_RFIN:
		if (Sigflag != BRK_READ)
			printex ("unexpected finish state, sig_set\n");
		Sigflag = BRK_SESS;
		break;
	default:
		printex ("bad state %d, sig_set\n",flag);
	}
}

grp_indic (s,ok)
char *s;
int ok;
{
	NODE *ptr,*hashfind();

	/* we go to hash table because s might be a temporary buffer */
	if ((ptr = hashfind(s)) != NULL)
	{
		Cur_scn = ptr->nd_name;
		if (Foreground)
		{
			if (ok)
				fgprintf("    %s\n",Cur_scn);
			else
				fgprintf("    %s - Can't access spool directory\n",Cur_scn);
		}
	}
}

fgprintf (fs,a,b,c,d,e)
char *fs;
int a,b,c,d,e;
{
	if (Foreground)
		fprintf (stderr,fs,a,b,c,d,e);
	fflush (stderr);
}

/* TECO for Ultrix   Copyright 1986 Matt Fichtenbaum						*/
/* This program and its components belong to GenRad Inc, Concord MA 01742	*/
/* They may be copied if this copyright notice is included					*/

/* te_chario.c   character I/O routines   10/9/86 */
#include <errno.h>
#include "te_defs.h"

#include <fcntl.h>
#ifndef DEBUG
#include <signal.h>
extern int int_handler();
extern int stp_handler();
extern int hup_handler();
#define SIGINTMASK 2
#endif

int lf_sw;									/* nonzero: make up a LF following an entered CR */
int ttyflags;								/* flags for (stdin) file descriptor */
struct tchars tc_orig, tc_new, tc_noint;	/* original, new, disabled intrpt tty special chars */
struct ltchars lc_orig, lc_new;				/* original and new local special chars */
struct sgttyb tty_orig, tty_new;			/* original and new tty flags */
int tty_local;								/* original tty local mode flags */
int lnoflsh = LNOFLSH;						/* bit to force "no flush on interrupt */

#ifndef DEBUG
struct sigvec intsigstruc = { int_handler, 0, 0 } ;		/* info structure for ^C interrupt		*/
struct sigvec stpsigstruc = { stp_handler, 0, 0 } ;		/* info structure for "stop" signal		*/
struct sigvec hupsigstruc = { hup_handler, 0, 0 } ;		/* info structure for "hangup" signal	*/
struct sigvec nosigstr = { SIG_DFL, 0, 0 };				/* default structure for signal			*/
#endif

int inp_noterm;				/* nonzero if standard input is not a terminal */
int out_noterm;				/* nonzero if standard output is not a terminal */
/* set tty (stdin) mode.  TECO mode is CBREAK, no ECHO, sep CR & LF				*/
/* operation; normal mode is none of the above.  TTY_OFF and TTY_ON do this		*/
/* absolutely; TTY_SUSP and TTY_RESUME use saved signal status.					*/

setup_tty(arg)
	int arg;
	{
	extern int errno;
	int ioerr;
	struct sgttyb tmpbuf;

/* initial processing: set tty mode */

	if (arg == TTY_ON)
		{
		ioerr = ioctl(fileno(stdin), TIOCGETP, &tty_orig);	/* get std input characteristics */
		inp_noterm = (ioerr && (errno == ENOTTY));			/* nonzero if input not a terminal */
		ioerr = ioctl(fileno(stdout), TIOCGETP, &tmpbuf);	/* get std output characteristics */
		out_noterm = (ioerr && (errno == ENOTTY));			/* nonzero if output not a terminal */
		ioctl(fileno(stdout), TIOCLGET, &tty_local);		/* get current "local mode flags" word */

		ttybuf = tty_new = tty_orig;						/* make a copy of tty control structure */
		tty_new.sg_flags = (tty_new.sg_flags & ~ECHO & ~CRMOD) | CBREAK;	/* turn on teco modes */

		ioctl(fileno(stdin), TIOCGETC, &tc_orig);		/* read current tchars */
		tc_new = tc_orig;								/* make local copy */
		tc_new.t_quitc = tc_new.t_brkc = -1;			/* disable "quit" and "delimiter" chars */
		tc_noint = tc_new;
		tc_noint.t_intrc = -1;							/* disable the interrupt char in this one */

		ioctl(fileno(stdin), TIOCGLTC, &lc_orig);		/* read current ltchars */
		lc_new = lc_orig;								/* make local copy */
		lc_new.t_rprntc = lc_new.t_werasc = lc_new.t_lnextc = -1;	/* disable "reprint," "word erase," "lit next" */
		}

	if ((arg == TTY_ON) || (arg == TTY_RESUME))
		{
		ioctl(fileno(stdin), TIOCSETP, &tty_new);		/* set flags for teco */
		ioctl(fileno(stdin), TIOCSETC, &tc_new);		/* update both */
		ioctl(fileno(stdin), TIOCSLTC, &lc_new);
		ioctl(fileno(stdout), TIOCLBIS, &lnoflsh);		/* disable "interrupt => flush buffers" */
#ifndef DEBUG
		sigvec(SIGTSTP, &stpsigstruc, 0);				/* set up to trap "stop" signal */
		sigvec(SIGINT, &intsigstruc, 0);				/* and "interrupt" signal */
		sigvec(SIGHUP, &hupsigstruc, 0);				/* and "hangup" signal */
#endif
		}
	else
		{
		ioctl(fileno(stdin), TIOCSETP, &tty_orig);		/* set flags back to original */
		ioctl(fileno(stdin), TIOCSETC, &tc_orig);		/* put both back to orig states */
		ioctl(fileno(stdin), TIOCSLTC, &lc_orig);
		ioctl(fileno(stdout), TIOCLSET, &tty_local);	/* restore local mode flags to original states */
#ifndef DEBUG
		sigvec(SIGTSTP, &nosigstr, 0);					/* restore default signal handling */
		sigvec(SIGINT, &nosigstr, 0);
		sigvec(SIGHUP, &nosigstr, 0);
#endif
		}
	}
/* routines to handle keyboard input */

/* routine to get a character without waiting, used by ^T when ET & 64 is set	*/
/* if lf_sw is nonzero, return the LF; else use the FNDELAY fcntl to inquire of the input */

int gettty_nowait()
	{
	int c;

	if (lf_sw)
		{
		lf_sw = 0;
		return(LF);			/* LF to be sent: return it */
		}
	fcntl(fileno(stdin), F_SETFL, ttyflags | FNDELAY);		/* set to "no delay" mode */
	while (!(c = getchar()));					/* read character, or -1, skip nulls */
	fcntl(fileno(stdin), F_SETFL, ttyflags);				/* reset to normal mode */
	if (c == CR) ++lf_sw;						/* CR: set switch to make up a LF */
	return(c);
	}



/* normal routine to get a character */

int in_read = 0;		/* flag for "read busy" (used by interrupt handler) */

char gettty()
	{
	int c;

	if (lf_sw)
		{
		lf_sw = 0;
		return(LF);		/* if switch set, make up a line feed */
		}
	++in_read;							/* set "read busy" switch */
	while(!(c = getchar()));				/* get character; skip nulls */
	in_read = 0;						/* clear switch */
	if (c == CR) ++lf_sw;					/* CR: set switch to make up a LF */
	if (c == EOF) ERROR(E_EFI);				/* end-of-file from standard input */
	return( (char) c & 0177);				/* and return the 7-bit char */
	}

#ifndef DEBUG

/* routine to handle interrupt signal */

int_handler()
	{

	if (exitflag <= 0)						/* if executing commands */
		{
		if (et_val & ET_CTRLC) et_val &= ~ET_CTRLC;		/* if "trap ^C" set, clear it and ignore */
		else exitflag = -2;								/* else set flag to stop execution */
		}
	if (in_read)							/* if interrupt happened in "getchar" pass a ^C to input */
		{
		in_read = 0;									/* clear "read" switch */
		ioctl(fileno(stdin), TIOCSETC, &tc_noint);		/* disable interrupt char */
		qio_char(CTL (C));								/* send a ^C to input stream */
		ioctl(fileno(stdin), TIOCSETC, &tc_new);		/* reenable interrupt char */
		}
	}
#endif
/* routine to disable (1), enable (0) ^C interrupt, used to block interrupts during display update */

int old_mask;				/* storage for previous signal mask */
#define INT_MASK 2

block_inter(func)
	int func;
	{
#ifndef DEBUG
	if (func) old_mask = sigblock(INT_MASK);			/* if arg nonzero, block interrupt */
	else sigsetmask(old_mask);							/* otherwise restore old signal mask */
#endif
	}



#ifndef DEBUG
/* routine to handle "stop" signal (^Y) */

stp_handler()
	{
	window(WIN_SUSP);				/* restore screen */
	setup_tty(TTY_SUSP);			/* put tty back to normal */
	sigvec(SIGTSTP, &nosigstr, 0); 	/* put default action back */
	sigsetmask(0);					/* unblock "suspend" signal */
	kill(0, SIGTSTP);				/* suspend this process */

/* ----- process gets suspended here ----- */

	sigvec(SIGTSTP, &stpsigstruc, 0);	/* restore local handling of "stop" signal */
	setup_tty(TTY_RESUME);				/* restore tty */
	buff_mod = 0;						/* set whole screen modified */
	if (win_data[7])			/* redraw window */
		{
		window(WIN_RESUME);		/* re-enable window */
		window(WIN_REDRAW);		/* force complete redraw */
		window(WIN_REFR);		/* and refresh */
		}
	qio_char('\0');				/* wake up the input
	if (exitflag) retype_cmdstr('*');	/* if not executing, prompt again and echo command string so far */
	}
#endif



/* simulate a character's having been typed on the keyboard */

qio_char(c)
	char c;
	{
	ioctl(fileno(stdin), TIOCSTI, &c);				/* send char to input stream */
	}
/* routine to handle "hangup" signal */
#ifndef DEBUG

hup_handler()
	{
	if (!exitflag) exitflag = -3;					/* if executing, set flag to terminate */
	else
		{
		panic();									/* dump buffer and close output files */
		exit(1);
		}
	}
#endif



/* type a crlf */

crlf()
	{
	type_char(CR);
	type_char(LF);
	}



/* reset ^O status */

int lflusho = LFLUSHO;
int lfo;

reset_ctlo()
	{
	ioctl(fileno(stdin), TIOCLGET, &lfo);		/* read flags */
	if (lfo & LFLUSHO)							/* if ^O was set */
		{
		ioctl(fileno(stdin), TIOCLBIC, &lflusho);	/* reset ^O */
		crlf();										/* type a crlf */
		}
	}
/* routine to type one character */

type_char(c)
	char c;
	{

	if ((char_count >= WN_width) && (c != CR) && !(spec_chars[c] & A_L))	/* spacing char beyond end of line */
		{
		if (et_val & ET_TRUNC) return;		/* truncate output to line width */
		else crlf();						/* otherwise do automatic new line (note recursive call to type_char) */
		}

	if ((c & 0140) == 0)				/* control char? */
		{
		switch (c & 0177)
			{
			case CR:
				putchar(c);
				char_count = 0;
				break;

			case LF:
				putchar(c);
/*				scroll_dly();			/* filler chars in case VT-100 scrolls */
				break;

			case ESC:
				if ((et_val & ET_IMAGE) && !exitflag) putchar(c);
				else
					{
					putchar('$');
					char_count++;
					}
				break;

			case TAB:
				if ((et_val & ET_IMAGE) && !exitflag) putchar(c);
				else for (type_char(' '); (char_count & tabmask) != 0; type_char(' '));
				break;

			default:
				if ((et_val & ET_IMAGE) && !exitflag) putchar(c);
				else
					{
					putchar('^');
					putchar(c + 'A'-1);
					char_count += 2;
					}
				break;
			}
		}
	else
		{
		putchar(c);
		char_count++;
		}
	}


/*
 *		Ultrix-32 and Unix terminal I/O.
 * The functions in this file
 * negotiate with the operating system for
 * keyboard characters, and write characters to
 * the display in a barely buffered fashion.
 */
#include	"def.h"

#include	<sgtty.h>

#define	NOBUF	512			/* Output buffer size.		*/

char	obuf[NOBUF];			/* Output buffer.		*/
int	nobuf;
struct	sgttyb	oldtty;			/* V6/V7 stty data.		*/
struct	sgttyb	newtty;
struct	tchars	oldtchars;		/* V7 editing.			*/
struct	tchars	newtchars;
struct	ltchars oldltchars;		/* 4.2 BSD editing.		*/
struct	ltchars	newltchars;
#ifdef	TIOCGWINSZ
struct	winsize	winsize;		/* 4.3 BSD window sizing	*/
#endif
int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

/*
 * This function gets called once, to set up
 * the terminal channel. On Ultrix is's tricky, since
 * we want flow control, but we don't want any characters
 * stolen to send signals. Use CBREAK mode, and set all
 * characters but start and stop to 0xFF.
 */
ttopen() {
        register char *tv_stype;
        char *getenv(), *tgetstr(), tcbuf[1024], err_str[72];

	if (ioctl(0, TIOCGETP, (char *) &oldtty) < 0)
		panic("ttopen can't get sgtty");
	newtty.sg_ospeed = oldtty.sg_ospeed;
	newtty.sg_ispeed = oldtty.sg_ispeed;
	newtty.sg_erase  = oldtty.sg_erase;
	newtty.sg_kill   = oldtty.sg_kill;
	newtty.sg_flags  = oldtty.sg_flags;
	newtty.sg_flags &= ~(ECHO|CRMOD);	/* Kill echo, CR=>NL.	*/
#ifdef FLOWCONTROL
	newtty.sg_flags |= CBREAK;		/* Half-cooked mode.	*/
#else
	newtty.sg_flags |= RAW|ANYP;		/* raw mode for 8 bit path.*/
#endif
	if (ioctl(0, TIOCSETP, (char *) &newtty) < 0)
		panic("ttopen can't set sgtty");
	if (ioctl(0, TIOCGETC, (char *) &oldtchars) < 0)
		panic("ttopen can't get chars");
	newtchars.t_intrc  = 0xFF;		/* Interrupt.		*/
	newtchars.t_quitc  = 0xFF;		/* Quit.		*/
#if FLOWCONTROL
	newtchars.t_startc = 0x11;		/* ^Q, for terminal.	*/
	newtchars.t_stopc  = 0x13;		/* ^S, for terminal.	*/
#else
	newtchars.t_startc = 0xFF;		/* ^Q, for terminal.	*/
	newtchars.t_stopc  = 0xFF;		/* ^S, for terminal.	*/
#endif
	newtchars.t_eofc   = 0xFF;
	newtchars.t_brkc   = 0xFF;
	if (ioctl(0, TIOCSETC, (char *) &newtchars) < 0)
		panic("ttopen can't set chars");
	if (ioctl(0, TIOCGLTC, (char *) &oldltchars) < 0)
		panic("ttopen can't get ltchars");
	newltchars.t_suspc  = 0xFF;		/* Suspend #1.		*/
	newltchars.t_dsuspc = 0xFF;		/* Suspend #2.		*/
	newltchars.t_rprntc = 0xFF;
	newltchars.t_flushc = 0xFF;		/* Output flush.	*/
	newltchars.t_werasc = 0xFF;
	newltchars.t_lnextc = 0xFF;		/* Literal next.	*/
	if (ioctl(0, TIOCSLTC, (char *) &newltchars) < 0)
		panic("ttopen can't set ltchars");

/* do this the REAL way */
        if ((tv_stype = getenv("TERM")) == NULL)
        {
                puts("Environment variable TERM not defined!");
                exit(1);
        }

        if((tgetent(tcbuf, tv_stype)) != 1)
        {
                (void) sprintf(err_str, "Unknown terminal type %s!", tv_stype);
                puts(err_str);
                exit(1);
        }

	setttysize() ;
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
ttclose() {
	ttflush();
	if (ioctl(0, TIOCSLTC, (char *) &oldltchars) < 0)
		panic("ttclose can't set ltchars");
	if (ioctl(0, TIOCSETC, (char *) &oldtchars) < 0)
		panic("ttclose can't set chars");
	if (ioctl(0, TIOCSETP, (char *) &oldtty) < 0)
		panic("ttclose can't set sgtty");
}

/*
 * Write character to the display.
 * Characters are buffered up, to make things
 * a little bit more efficient.
 */
ttputc(c) {
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
}

/*
 * Flush output.
 */
ttflush() {
	if (nobuf != 0) {
		if (write(1, obuf, nobuf) != nobuf)
			panic("ttflush write failed");
		nobuf = 0;
	}
}

/*
 * Read character from terminal.
 * All 8 bits are returned, so that you can use
 * a multi-national terminal.
 */
ttgetc() {
	char	buf[1];

	while (read(0, &buf[0], 1) != 1)
		;
	return (buf[0] & 0xFF);
}
/*
 * set the tty size. Functionized for 43BSD.
 */
setttysize() {

#ifdef	TIOCGWINSZ
	if (ioctl(0, TIOCGWINSZ, (char *) &winsize) == 0) {
		nrow = winsize . ws_row;
		ncol = winsize . ws_col;
	} else nrow = 0;
 	if(nrow<=0 || ncol<=0)
#endif
	if ((nrow=tgetnum ("li")) <= 0
	|| (ncol=tgetnum ("co")) <= 0) {
		nrow = 24;
		ncol = 80;
	}
	if (nrow > NROW)			/* Don't crash if the	*/
		nrow = NROW;			/* termcap entry is	*/
	if (ncol > NCOL)			/* too big.		*/
		ncol = NCOL;
}

/*
 * typeahead returns TRUE if there are characters available to be read
 * in.
 */
typeahead() {
	int	x;

	return((ioctl(0, FIONREAD, (char *) &x) < 0) ? 0 : x);
}

/*
 * panic - just exit, as quickly as we can.
 */
panic(s) char *s; {
	printf(stderr, "panic: %s\n", s);
	abort();		/* To leave a core image. */
}

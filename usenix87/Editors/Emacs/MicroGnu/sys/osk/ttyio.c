/*
 *	sys/osk/ttyio.c	by Robert A. Larson
 *
 * The functions in this file
 * negotiate with the operating system for
 * keyboard characters, and write characters to
 * the display in a barely buffered fashion.
 */
#include	"def.h"

#include	<sgstat.h>
#ifdef	DPROMPT
#  include	<varargs.h>
#  define S_RDY 2437			/* arbitrary user signal */
#endif

#define	NOBUF	512			/* Output buffer size.		*/

char	obuf[NOBUF];			/* Output buffer.		*/
int	nobuf;
struct	sgbuf	oldtty, newtty;
int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/
short	ospeed;				/* Terminal speed, for termlib.l */

#ifdef	DPROMPT
wakeup(signum)
int signum;
{
	/* ignore the signal */
}
#endif

/*
 * This function gets called once, to set up
 * the terminal channel.
 */

ttopen()
{
	if(_gs_opt(0, &oldtty) == -1) panic("can't get options");
	ospeed = oldtty.sg_baud;
	_strass(&newtty, &oldtty, sizeof(newtty));	/* newtty=oldtty; */
	if(oldtty.sg_class == 0) { 			/* scf */
		newtty.sg_backsp=
		newtty.sg_delete=
		newtty.sg_echo  =
		newtty.sg_alf   =
		newtty.sg_pause =
		newtty.sg_bspch =
		newtty.sg_dlnch =
		newtty.sg_eorch =
		newtty.sg_eofch =
		newtty.sg_rlnch =
		newtty.sg_dulnch=
		newtty.sg_psch  =
		newtty.sg_kbich =
		newtty.sg_kbach = 0;
#ifndef	xon_xoff
		newtty.sg_xon   =
		newtty.sg_xoff  = 0;
#endif
		if(_ss_opt(0, &newtty) == -1) panic("can't set options");
		nrow = oldtty.sg_page == 0 ? NROW : oldtty.sg_page;
	} else {				/* not scf, fake it */
		nrow = NROW;
	}
	ncol = NCOL;
#ifdef	DPROMPT
	intercept(wakeup);		/* ignore signals */
#endif
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
ttclose()
{
	ttflush();
	if(_ss_opt(0, &oldtty) == -1) panic("can't reset options");
}

/*
 * Write character to the display.
 * Characters are buffered up, to make things
 * a little bit more efficient.
 */
ttputc(c)
{
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
}

/*
 * Flush output.
 */
ttflush()
{
	if (nobuf != 0) {
		write(1, obuf, nobuf);
		nobuf = 0;
	}
}

/*
 * Read character from terminal.
 * All 8 bits are returned, so that you can use
 * a multi-national terminal.
 */
ttgetc()
{
	char	buf[1];

	while (read(0, &buf[0], 1) != 1)
		;
	return (buf[0] & 0xFF);
}

int typeahead()
{
  return _gs_rdy(0) > 0;
}

panic(s) char *s; {
  _ss_opt(0, &oldtty);			/*  ignore errors */
  fputs("Panic: ", stdout);		/* avoid printf, don't load all that */
  puts(s);
  exit(1);
}

#ifdef	DPROMPT
ttwait() {
  if(_gs_rdy(0) > 0) return FALSE;	/* already something waiting */
  _ss_ssig(0, S_RDY);			/* wake me when you have something */
  if(sleep(2)!=0) return FALSE;		/* sleep interupted */
  _ss_rel(0);
  return TRUE;
}
#endif

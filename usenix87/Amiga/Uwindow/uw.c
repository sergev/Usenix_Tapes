/* uw protocol interpretation */
#include "term.h"

/*
 * Protocol 0:
 *
 * The connection between the Macintosh and the host is simply a serial
 * line.  Flow control may be enabled, but no special commands are
 * recognized.  Only one active window is supported.  This "protocol"
 * does not require the UW server; hence, there is no need to support it.
 */
#define XON 		021
#define XOFF 		023
#define META 		0200	/* Meta Bit */

/*
 * Protocol 1: (original UW protocol)
 *
 * Two types of information are exchanged through the 7-bit serial line:
 * ordinary data and command bytes.  Command bytes are preceeded by
 * an IAC byte.  IAC bytes and literal XON/XOFF characters (those which
 * are not used for flow control) are sent by a P1_FN_CTLCH command.
 * Characters with the eighth bit set (the "meta" bit) are prefixed with
 * a P1_FN_META function.
 *
 * The next most-significant bit in the byte specifies the sender and
 * recipient of the command.  If this bit is clear (0), the command byte
 * was sent from the host computer to the Macintosh; if it is set (1)
 * the command byte was sent from the Macintosh to the host computer.
 * This prevents confusion in the event that the host computer
 * (incorrectly) echos a command back to the Macintosh.
 *
 * The remaining six bits are partitioned into two fields.  The low-order
 * three bits specify a window number from 1-7 (window 0 is reserved for
 * other uses) or another type of command-dependent parameter.  The next
 * three bits specify the operation to be performed by the recipient of
 * the command byte.
 *
 * Note that the choice of command bytes prevents the ASCII XON (021) and
 * XOFF (023) characters from being sent as commands.  P1_FN_ISELW commands
 * are only sent by the Macintosh (and thus are tagged with the P1_DIR_MTOH
 * bit).  Since XON and XOFF data characters are handled via P1_FN_CTLCH,
 * this allows them to be used for flow control purposes.
 */
#define	P1_IAC		0001		/* interpret as command */
#define	P1_DIR		0100		/* command direction: */
#define	P1_DIR_HTOM	0000		/*	from host to Mac */
#define	P1_DIR_MTOH	0100		/*	from Mac to host */
#define	P1_FN		0070		/* function code: */
#define	P1_FN_NEWW	0000		/*	new window */
#define	P1_FN_KILLW	0010		/*	kill (delete) window */
#define	P1_FN_ISELW	0020		/*	select window for input */
#define	P1_FN_OSELW	0030		/*	select window for output */
#define	P1_FN_META	0050		/*	add meta to next data char */
#define	P1_FN_CTLCH	0060		/*	low 3 bits specify char */
#define	P1_FN_MAINT	0070		/*	maintenance functions */
#define	P1_WINDOW	0007		/* window number mask */
#define	P1_CC		0007		/* control character specifier: */
#define	P1_CC_IAC	1		/*	IAC */
#define	P1_CC_XON	2		/*	XON */
#define	P1_CC_XOFF	3		/*	XOFF */
#define	P1_MF		0007		/* maintenance functions: */
#define	P1_MF_ENTRY	0		/*	beginning execution */
#define	P1_MF_ASKPCL	2		/*	request protocol negotiation */
#define	P1_MF_CANPCL	3		/*	suggest protocol */
#define	P1_MF_SETPCL	4		/*	set current protocol */
#define	P1_MF_EXIT	7		/*	execution terminating */
#define	P1_NWINDOW	7		/* maximum number of windows */


int 	uwflag = FALSE,
	IACflag = FALSE,
	Metaflag = FALSE;
char processed[4096];
int j = 0;			/* an index into to processed buffer */

UWwrite(buf,length)
char buf[];
int length;
{
	char c;
	int i;

	for(i = 0; i < length ; ++i) {
		c = buf[i];
		if (!IACflag) {
			if (c == P1_IAC) {
				IACflag = TRUE;
			} else if (!Metaflag) {
				processed[j++] = c;
			} else {
				processed[j++] = c | META;
				Metaflag = FALSE;
			}
		} else if ((c & P1_DIR) == P1_DIR_HTOM) {
/*			printf(" P1_IAC & 0%o ", c); */
			switch(c & P1_FN) {
			case P1_FN_NEWW:  NewWin(c & P1_WINDOW);
				break;
			case P1_FN_KILLW:  KillWin(c & P1_WINDOW);
				break;
			case P1_FN_OSELW:  SelOutput(c & P1_WINDOW);
				break;
			case P1_FN_META:  Metaflag = TRUE;
				break;
			case P1_FN_CTLCH:  CtlCh(c);
				break;
			case P1_FN_MAINT:  Maint(c);
				break;
			} /* end switch */
			IACflag = FALSE;
		} /* end if P1_DIR_HTOM */
	} /* end for */
	Pflush();
} /* end uw_write */

Pflush()
{
	if (j > 0) WriteCon(processed, j);
	j = 0;
}

NewWin(w)
int w;
{
	if (uwflag) {
		if (uw[w].free) {
/*			printf("NEWW %d\n", w); */
			AllocTerm(w);
		}
	} else {
		processed[j++] = P1_IAC;
		processed[j++] = P1_DIR_HTOM | P1_FN_NEWW | w;
	}
} /* end of NewWin */

KillWin(w)
int w;
{
	if (uwflag) {
/*		printf("KILLW %d\n", w); */
		if (uw[w].free == FALSE) CloseWinNoMsg(w);
	} else {
		processed[j++] = P1_IAC;
		processed[j++] = P1_DIR_HTOM | P1_FN_KILLW | w;
	}
} /* end of KillWin */

SelInput(w)
int w;
{
/*	printf("C:ISELW %d\n", w); */
	uw_read = w;
	sputc(P1_IAC);
	sputc(P1_DIR_MTOH | P1_FN_ISELW | w);
} /* end of SelInput */

SelOutput(w)
int w;
{
	if (uwflag) {
/*		printf("OSELW %d\n",w); */
		Pflush();
		if (uw[w].free == FALSE) uw_write = w;
	} else {
		processed[j++] = P1_IAC;
		processed[j++] = P1_DIR_HTOM | P1_FN_OSELW | w;
	}
} /* end of SelOutput */

CtlCh(c)
char c;
{
	char d;

if (uwflag) {
	d = 0;
	switch(c & P1_CC) {
	case P1_CC_IAC: d = P1_IAC;
		break;
	case P1_CC_XON: d = XON;
		break;
	case P1_CC_XOFF: d = XOFF;	
		break;
	}
	if (d != 0)
		if (Metaflag) {
			processed[j++] = d | META;
			Metaflag = FALSE;
		} else processed[j++] = d;
} else {
	processed[j++] = P1_IAC;
	processed[j++] = c;
}
} /* end of CtlCh */

Maint(c)
char c;
{
	switch (c & P1_MF) {
	case P1_MF_ENTRY:  InitUW();
		break;
	case P1_MF_EXIT: ExitUW();
		break;
	} 
} /* end of Maint */

InitUW()
{
	int i;

/*	printf("MF_ENTRY\n"); */
	uwflag = TRUE;
	for (i = 1; i < 8; ++i) if (uw[i].free == FALSE) DeallocTerm(i);
	uw_read = -1;	/* No current input */
/*	MkNewWin(); */
}

ExitUW()
{
	int i;

/*	printf("MF_EXIT\n"); */
	for(i = 0; i < 8; ++i)
		if (uw[i].free == FALSE) DeallocTerm(i);
	AllocTerm(1);
	uw_read = 1;
	uw_write = 1;
	uwflag = FALSE;
}

MkNewWin()
{
	int i;

if(uwflag){
	i = 1;
	while ((i < 8) && (uw[i].free == FALSE)) ++i;

	if (i < 8) {
/*		printf("C:NEWW %d\n",i); */
		AllocTerm(i);
		sputc(P1_IAC);
		sputc(P1_DIR_MTOH | P1_FN_NEWW | i);
	}
}
} /* end of MkNewWin */

CloseWinNoMsg(i)
int i;
{
	if (uw[i].free == FALSE) DeallocTerm(i);
	if (uw_count == 0) {
/*		printf("C:Reset\n"); */
		UWExit();
		InitTerm();
	}
} /* end of CloseWin */

CloseWin(i)
int i;
{
	if (uw[i].free == FALSE) {
/*		printf("C:KILLW %d\n",i); */
		DeallocTerm(i);
		if (uwflag) {
			sputc(P1_IAC);
			sputc(P1_DIR_MTOH | P1_FN_KILLW | i);
		}
	}
	if (uw_count == 0) {
		if(uwflag) {
			UWExit();
			InitTerm();
			return(TRUE);
		} else 	return(FALSE);
	} else return(TRUE);
} /* end of CloseWin */

UWWriteChar(c)
char c;
{
	if(uwflag) {
		switch (c) {
		case XON:  sputc(P1_IAC);
			sputc(P1_DIR_MTOH | P1_FN_CTLCH | P1_CC_XON);
			break;
		case XOFF: sputc(P1_IAC);
			sputc(P1_DIR_MTOH | P1_FN_CTLCH | P1_CC_XOFF);
			break;
		case P1_IAC: sputc(P1_IAC);
			sputc(P1_DIR_MTOH | P1_FN_CTLCH | P1_CC_IAC);
			break;
		default:
			sputc(c);
			break;
		} /* end switch */
	} else {
		sputc(c);
	}
} /* end of UWWriteChar */

UWExit()
{
/*	printf("C:MF_EXIT\n"); */
	sputc(P1_IAC);
	sputc(P1_DIR_MTOH | P1_FN_MAINT | P1_MF_EXIT);
} /* end of ExitUW */

InitTerm()
{
	uwflag = FALSE;
	Metaflag = FALSE;
	IACflag = FALSE;
	uw_read = 1;
	uw_write = uw_read;
	AllocTerm(uw_read);
}

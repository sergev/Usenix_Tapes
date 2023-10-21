/*
 *	RT11 EMULATOR
 *	initialization and
 *	system call interpretation
 *
 *	Daniel R. Strick
 *	May 31, 1979
 *	1/3/80 -- changed getprog() to use rt11 I/O routines
 *	1/3/80 -- changed TTYIN to map lower case into upper case unless
 *		  the lower case bit in the job status word is set
 *	4/14/81 -- csinter() now takes args and returns switches on stack
 *	4/15/81 -- .EXIT now flushes terminal output
 *	7/17/81 -- no longer needs the standard I/O library
 *	7/20/81 -- added memory size option
 *	7/21/81 -- the fixed offset area is now created here
 *	7/21/81 -- implemented the .DELETE and .RENAME system calls, and
 *		   added version 2 hooks for .SAVESTATUS and .REOPEN (untested!)
 *	7/23/81 -- will now try appending ".sav" to program name
 *	8/24/81 -- extracted rtprint() from rt() so that csinter() can prompt
 *	8/30/81 -- added a core dump of sorts
 *	8/30/81 -- requests that "load" things at given addresses now return
 *		   even addresses slightly larger than their argument
 *	8/30/81 -- range check for .GTVAL offsets
 *	8/30/81 -- implemented the .PURGE request (well, almost)
 *	10/15/81 -- disabled the core dump gimmick
 */

#include "define.h"
#include "emt.h"
#include "rmon.h"
#include "syscom.h"

#define	DEFSIZ	50	/* default size of RT11 address space in K bytes */
#define	FNAMAX	100	/* size of buffer for appending .sav to prog name */

#ifdef	TRACE
#define	emtrace(param)	printf param
#else
#define emtrace(param)
#endif

int	usertop;	/* to top of memory available to user */
wordp	compret;	/* to completion routine for completion routines */


/*
 *	usage:  rtrun  [-s#]  [-v]  save-file
 *
 *  The -s# option declares the size of the RT11 address space in k byte.
 *  The -v option permits storage of overlays in the VAX address space.
 */
main (argc, argv)
char **argv;
{
	register unsigned int n;
	register char *fname;
	register char *cp;
	int vflag;
	unsigned int size;
	extern char embase[];

	/*
	 *  process arguments
	 */
	vflag = NO;
	size = DEFSIZ;
	fname = NULL;
	for (n = 1;  n < argc;  ++n) {
		cp = argv[n];
		if (*cp == '-') {
			++cp;
			switch (*cp++) {

			case 'v':
				vflag = YES;
				break;

			case 's':
				size = atoi (cp);
				if (size == 0  ||  size > 64) {
					printf ("bad size: %s\n", cp);
					size = 0;
				}
				break;

			default:
				printf ("bad flag: %s\n", cp-2);
			}
		} else
			fname = cp;
	}
	if (size == 0  ||  fname == NULL) {
		printf("usage: %s [-s#] [-v] save-file\n", argv[0]);
		return (1);
	}

	/*
	 *  set up RT11 address space
	 */
	n = size * 1024;
	size = (unsigned int) embase - 600;
	if (n > size) {
		printf ("max size is %d kb!\n", size / 1024);
		return (1);
	}
	size = sbrk(0);
	if (size < n  &&  brk(n) < 0) {
		printf ("cannot expand memory!\n");
		return (1);
	}
	if (getprog (fname, n/2, vflag) == NO)
		return (1);
	n -= R_TOTSIZ;
	rmon = (word *) n;
	syscom.c_rmon = n;
	usertop = n - 20;
	offinit();

	start();
	return (0);
}


#define	OFFBYT(o)	((char *) rmon) [o]
#define	OFFWRD(o)	(* (word *) ((char *) rmon + o))

/*
 *  Initialize the contents of the
 *  fixed offset area.
 */
offinit ()
{
	register word *wp;
	wordp fakereg, fakesub;

	wp = rmon;
	do
		*wp++ = 0;
	while (wp < &OFFWRD(R_TOTSIZ));

	fakereg = (wordp) &OFFWRD(R_FAKEREG);
	wp = &OFFWRD(R_FAKESUB);
	fakesub = (wordp) wp;
	*wp++ =	000004;			/* iot */
	compret = (wordp) wp;
	*wp++ = 012601;			/* mov (r6)+, r1 */
	*wp++ = 012600;			/* mov (r6)+, r2 */
	*wp++ = 000006;			/* rtt */

	OFFWRD(R_INTVEC) = fakesub;
	OFFWRD(R_DATE) = 02040;		/* January 1, 1972 */
	OFFWRD(R_USRLC) = fakesub;
	OFFWRD(R_QCOMP) = fakesub;
	OFFBYT(R_SYSVER) = 4;
	wp = &OFFWRD(R_SCROLL);
	*wp++ = fakesub;		/* VT11 scroller */
	*wp++ = fakereg;		/* address of console keyboard status */
	*wp++ = fakereg;		/* address of console keyboard buffer */
	*wp++ = fakereg;		/* address of console printer status */
	*wp++ = fakereg;		/* address of console printer buffer */
	*wp++ = -1;			/* maximum output file size */
	OFFWRD(R_SYNCH) = fakesub;
	OFFWRD(R_USRLOC) = fakesub;
}


#define	OVRCHN	15

/*
 *  Get the user program.
 *  Return YES if successful.
 *  This routine just copies the first "maxpro" words of the
 *  given file into memory at location 0.  It ought to (but
 *  does not) do something special for the first block which
 *  contains areas that should not be loaded and some data
 *  that should control the loading of the rest of the file.
 */
getprog (name, maxpro, vflag)
char *name;
unsigned int maxpro;
{
	register int n;
	char fn[FNAMAX];
	char *strcop();

	n = RONLY;
	if (vflag)
		n = VREAD;
	strcop (".sav", strcop(name,fn,fn+FNAMAX), fn+FNAMAX);
	if (flookup(OVRCHN, name, n) >= 0  &&
	    flookup(OVRCHN, fn, n) >= 0) {
		perror (name);
		return (NO);
	}
	n = rtread (OVRCHN, (char*) 0, maxpro, 0, 0, 0);
	if (n == 0) {
		perror (name);
		rtclose (OVRCHN);
		return (NO);
	}
	if ((syscom.c_jobst & C_OVRLY)  ==  0)
		rtclose (OVRCHN);
	return (YES);
}


/*
 *  like puts() but with rt11 crlf suppression option
 */
rtprint (acp)
char *acp;
{
	register char *cp;

	cp = acp;
	while (*cp & 0177)
		putchr (*cp++);
	if (*cp == 0)
		putchr ('\n');
}


#define	CBIT	1	/* program status word carry bit */

/*
 * Intercept an RT11 EMT system call and
 * execute the silly thing with UNIX system calls.
 */
/*ARGSUSED*/
#ifdef VAX
emtrap (r0, r1, r2, r3, r4, r5, sp, pc, ps)
#else
emtrap (sp, ps, pc, r1, r0)
#endif
word *sp, *pc;
{
	int emtcode;
	word *csinter();

	ps &= ~CBIT;
	syscom.c_error = -1;
	switch (emtcode = pc[-1] & 0377) {


	case CSIGEN: {
			emtrace (("CSIGEN: '%s' at pc 0%o\n", sp[0], pc-1));
			r0 = (sp[2] + 3) & ~1;	/* addr after handlers */
			sp = csinter (sp, NO);
			break;
		}

	case CSISPC: {
			emtrace (("CSISPC: '%s' at pc 0%o\n", sp[0], pc-1));
			sp = csinter (sp, YES);
			break;
		}

	case DSTATUS:
		emtrace (("DSTATUS at pc 0%o\n", pc-1));
		dstatus ((rad50*) r0, (word*) *sp++);
		break;

	case EXIT:
		emtrace (("EXIT at pc 0%o\n", pc-1));
		flush();
		exit(0);

	case FETCH:
		emtrace (("FETCH at pc 0%o\n", pc-1));
		r0 = (*sp++ + 3) & ~1;
		break;

	case HRESET:
		emtrace (("HRESET at pc 0%o\n", pc-1));
		closall (syscom.c_jobst & C_OVRLY  ?  CHNMAX-2 : CHNMAX-1);
		break;

	case LOCK:
		emtrace (("LOCK at pc 0%o\n", pc-1));
		break;

	case PRINT:
		rtprint ((char *) r0);
		break;

	case QSET:
		emtrace (("QSET at pc 0%o\n", pc-1));
		r0 = (*sp++ + 3) & ~1;
		break;

	case RCTRLO:
		emtrace (("RCTRLO at pc 0%o\n", pc-1));
		break;

	case SETTOP:
		emtrace(("SETTOP to 0%o (max %o) at pc 0%o\n",r0,usertop,pc-1));
		if (r0 > usertop)
			r0 = usertop;
		syscom.c_hmem = r0;
		break;

	case SRESET:
		emtrace (("SRESET at pc 0%o\n", pc-1));
		closall (syscom.c_jobst & C_OVRLY  ?  CHNMAX-2 : CHNMAX-1);
		break;

	case TTYIN: {
			register int c;
			static int nflag;
	
			if (nflag) {
				nflag = NO;
				c = '\n';
			} else {
				c = getchr();
				if (c == '\n') {
					nflag = YES;
					c = '\r';
				} else
				if (c >= 'a'  &&  c <= 'z'  &&
				   (syscom.c_jobst & C_LOWCS) == 0)
					c += 'A' - 'a';
			}
			r0 = c;
		}
		break;

	case TTYOUT:
		putchr (r0);
		break;

	case UNLOCK:
		emtrace (("UNLOCK at pc 0%o\n", pc-1));
		break;

	case EMT374:
		r0 = emt374 (r0, pc);
		break;

	case EMT375:
		r0 = emt375 (r0, pc);
		break;

	default:
		if (emtcode < V1_END) {
			r0 = v1emt (r0, pc, &sp, emtcode);
			break;
		}
		printf ("UNIMPLEMENTED EMT %o at user pc %o\n", emtcode, pc-1);
		dump();
	}
	if (syscom.c_error >= 0)
		ps |= CBIT;
}


/*
 *  EMT374 channel/function word format
 */
struct	chanwrd	{
	char	em_chan;	/* I/O channel number */
	char	em_func;	/* function code */
};
#define	r0ch	(* (struct chanwrd *) &r0)

emt374 (r0, pc)
word *pc;
{
	register unsigned int chan;

	chan = r0ch.em_chan & 0377;
	switch (r0ch.em_func) {

	case S_CLOSE:
		emtrace (("CLOSE on channel %d at pc 0%o\n", chan, pc-1));
		rtclose (chan);
		break;

	case S_DATE:
		emtrace (("DATE at pc 0%o\n", pc-1));
		r0 = OFFWRD (R_DATE);
		break;

	case S_PURGE:
		/*
		 *  This code does not purge channels, it only
		 *  closes them.  A more precise implementation
		 *  does not seem worthwhile at this time.
		 */
		emtrace (("PURGE at pc 0%o\n", pc-1));
		rtclose (chan);
		break;

	case S_WAIT:
		emtrace (("WAIT on channel %d at pc 0%o\n", chan, pc-1));
		rtwait (chan);
		break;

	default:
		printf ("EMT374 code %o channel %d at user pc %o\n",
			r0ch.em_func, chan, pc-1);
		dump();
	}
	return (r0);
}


/*
 * EMT375 argument block format
 */
struct	ab375	{
	char	rw_chan;	/* I/O channel number */
	char	rw_func;	/* I/O function code */
	word	rw_blk;		/* number of block to be xferred */
	wordp	rw_buff;	/* buffer address */
	word	rw_wcnt;	/* transfer word count */
	wordp	rw_crtn;	/* address of completion routine */
};
#define	r0bp	((struct ab375 *) r0)
#define	r0iv	((word *) r0)
#define	r0chan	(/*(unsigned)*/ r0bp->rw_chan)	/* cast makes phototypesetter */
						/* cc generate bad code */
emt375 (ar0, pc)
word *pc;
{
	register int r0;

	r0 = ar0;
	switch (r0bp->rw_func) {

	case S_DELETE:
		emtrace (("DELETE at pc 0%o\n", pc-1));
		delete ((rad50*) r0iv[1]);
		break;

	case S_GTIM: {
			register short *ctpm;
			long secs;

			ctpm = (short *) r0iv[1];
			time (&secs);
			secs = (secs % 86400) * 60;
			ctpm[0] = secs / 65536;
			ctpm[1] = secs % 65536;
		}
		break;

	case S_GTJB: {
			register word *ap;

			emtrace (("GTJB at pc 0%o\n", pc-1));
			ap = (word *) r0iv[1];
			*ap++ = 0;		/* job # */
			*ap++ = usertop;	/* high mem limit */
			*ap++ = 0;		/* low memory limit */
			*ap = 0;		/* addr I/O channel space */
		}
		break;

	case S_GVAL:
		emtrace (("GTVAL offset 0%o at pc 0%o\n", r0iv[1], pc-1));
		if (r0iv[1] >= R_ENTAB)
			syscom.c_error = 0;
		else
			r0 = OFFWRD ( r0iv[1] );
		break;

	case S_ENTER:
		emtrace (("ENTER at pc 0%o\n", pc-1));
		enter (r0chan, (rad50*) r0iv[1]);
		break;

	case S_LOOKUP:
		emtrace (("LOOKUP at pc 0%o\n", pc-1));
		lookup (r0chan, (rad50*) r0iv[1]);
		break;

	case S_PROTECT:
		emtrace (("PROTECT at pc 0%o\n", pc-1));
		syscom.c_error = 0;		/* always fails */
		break;
	
	case S_READ:
		emtrace(("READ on channel %d at pc 0%o  (blk=%d  wc=%d)\n",
			r0chan, pc-1, r0bp->rw_blk, r0bp->rw_wcnt));
		r0 = rtread (r0chan, (char*) r0bp->rw_buff, r0bp->rw_wcnt,
			r0bp->rw_crtn, r0bp->rw_blk, r0);
		break;

	case S_RENAME:
		emtrace (("RENAME at pc 0%o\n", pc-1));
		rename ((rad50*) r0iv[1]);
		break;

	case S_REOPEN:
		emtrace (("REOPEN on channel %d at pc 0%o\n", r0chan, pc-1));
		reopen (r0chan, (word*) r0iv[1]);
		break;

	case S_SAVSTS:
		emtrace(("SAVESTATUS on channel %d at pc 0%o\n", r0chan, pc-1));
		savesta (r0chan, (word*) r0iv[1]);
		break;

	case S_SFPA:
		emtrace (("SFPA trap to 0%o at pe 0%o\n", r0iv[1], pc-1));
		break;

	case S_TRPSET:
		emtrace (("TRAPSET ap pc 0%o\n", pc-1));
		break;

	case S_WRITE:
		emtrace(("WRITE on channel %d at pc 0%o  (blk=%d  wc=%d)\n",
			r0chan, pc-1, r0bp->rw_blk, r0bp->rw_wcnt));
		r0 = rtwrite (r0chan, (char*) r0bp->rw_buff, r0bp->rw_wcnt,
			r0bp->rw_crtn, r0bp->rw_blk, r0);
		break;

	default:
		printf ("EMT375 code %o channel %d at user pc %o\n",
			r0bp->rw_func, r0chan, pc-1);
		dump();
	}
	return (r0);
}


v1emt (r0, pc, psp, emtcode)
word *pc, **psp;
{
	register word *sp;
	register unsigned int chan;

	sp = *psp;
	chan = emtcode & 017;
	emtcode &= ~017;
	switch (emtcode) {

	case V1_CLOS:
		emtrace (("CLOSE-V1 on channel %d at pc 0%o\n", chan, pc-1));
		rtclose (chan);
		break;

	case V1_DELT:
		emtrace (("DELETE-V1 at pc 0%o\n", pc-1));
		delete ((rad50*) r0);
		break;

	case V1_ENTR:
		emtrace (("ENTER-V1 at pc 0%o\n", pc-1));
		*psp += 1;
		enter (chan, (rad50*) r0);
		break;

	case V1_LOOK:
		emtrace (("LOOKUP-V1 at pc 0%o\n", pc-1));
		lookup (chan, (rad50*) r0);
		break;

	case V1_READ:
		emtrace (("READ-V1 on channel %d at pc 0%o  (blk=%d  wc=%d)\n",
			chan, pc-1, r0, sp[1]));
		*psp += 3;
		r0 = rtread (chan, sp[0], sp[1], sp[2], r0, r0);
		break;

	case V1_RENM:
		emtrace (("RENAME-V1 at pc 0%o\n", pc-1));
		rename ((rad50*) r0);
		break;

	case V1_REOP:
		emtrace (("REOPEN-V1 on channel %d at pc 0%o\n", chan, pc-1));
		reopen (chan, (word*) r0);
		break;

	case V1_SAVS:
		emtrace (("SAVEST-V1 on channel %d at pc 0%o\n", chan, pc-1));
		savesta (chan, (word*) r0);
		break;

	case V1_WAIT:
		emtrace (("WAIT-V1 on channel %d at pc 0%o\n", chan, pc-1));
		rtwait (chan);
		break;

	case V1_WRIT:
		emtrace (("WRITE-V1 on channel %d at pc 0%o  (blk=%d  wc=%d)\n",
			chan, pc-1, r0, sp[1]));
		*psp += 3;
		r0 = rtwrite (chan, sp[0], sp[1], sp[2], r0, r0);
		break;

	default:
		printf ("V1 EMT code %o channel %d at user pc %o\n",
			emtcode, chan, pc-1);
		dump ();
	}
	return (r0);
}


/*
 *  The registers are valid only if called from VAX11/trap.s.
 *  This is because this feature was never completely implemented.
 */
dump (r0, r1, r2, r3, r4, r5, r6)
{
	register int fd;

#ifdef	DUMP
	if ((fd = creat ("core", 0666))  >=  0) {
		write (fd, 0, sbrk(0));
		close (fd);
		printf ("core dumped\n");
	}
#endif
	printf ("execution aborted\n");
	exit (1);
}

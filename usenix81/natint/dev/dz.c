/*
 *      modification of DH-11 Driver
 *      for the DZ-11
 */

#include "../hd/param.h"
#include "../hd/conf.h"
#include "../hd/user.h"
#include "../hd/tty.h"
#include "../hd/proc.h"

#define DZADDR   0160120
#define NDZ11    8      /* number of lines */
#define DZSAMPLE 60     /* number of clock ticks between line status checks */

struct  tty dz11[NDZ11];
int     dzcarr;         /* Stores old value of carrier byte.
			 * Used by dzsample routine */
int     sampling;       /* flag to start dzsample() going */
int     modem;          /* set bits to flag that line is a modem */
int     dzok;           /* set if dz unibus addr responds */

/*
 * Hardware control bits
 * LPR - Line Parameter Register
 */
#define BITS6     010
#define BITS7     020
#define BITS8     030
#define TWOSB     040
#define PENABLE  0100
#define OPAR     0200
#define RCVRON 010000
#define SPEED   07400   /*      Baud    Code
				  50      0
				  75      1
				 110      2
				 134.5    3
				 150      4
				 300      5
				 600      6
				1200      7
	These codes differ      1800      8
	from DH-11 and          2000      9
	stty.c must be          2400     10
	modified to report      3600     11
	correct speeds.         4800     12
				7200     13
				9600     14
			      reserved   15
			*/
/*
 * RBUF - Receiver Buffer Register
 */
#define RLINE     03400
#define RVALID  0100000
#define RERROR   050000
#define PERROR   010000
#define FRERROR  020000
/*
 * CSR - Control Status Register
 */
#define MSE         040
#define IENAB    040100
#define XINT    0100000

char    SSPEED[NDZ11]   {5,5,5,5,5,5,5,5};

struct dzwregs {
	int     dzcsr;
	int     dzlpr;
	char    dztcr;
	char    dzdtr;
	char    dztbuf;
	char    dzbrk;
};
struct dzrregs {
	int     dzcsr;
	int     dzrbuf;
	char    dztcr;
	char    dzdtr;
	char    dzring;
	char    dzcar;
};

#define bit     tp->t_char      /*** 1<<minor for speed (works only if NDZ11<=8)***/

/*
 * Open a DZ11 line.
 */
dzopen(dev, flag)
{
	register struct tty *tp;
	register minor;
	extern dzstart(), dzsample();

	if ((minor = dev.d_minor) >= NDZ11) {
		u.u_error = ENXIO;
		return; }
	if(!dzok)
		if(tkword(DZADDR)){ u.u_error= ENXIO; return; }
		else dzok++;
	if(!sampling){
		if(timeout(&dzsample,0,DZSAMPLE)) sampling++; }
	tp = &dz11[minor];
	tp->t_addr = &dzstart;
	tp->t_dev= dev;
	tp->t_state =| WOPEN|SSTART;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_erase = CERASE;
		tp->t_kill = CKILL;
		tp->t_speeds = SSPEED[minor] | (SSPEED[minor]<<8);
		tp->t_flags = ODDP|EVENP|ECHO;
		bit = 1<<minor;
		dzparam(tp);
	}
	DZADDR->dzdtr =| bit;           /* Turn on DTR */
	if (modem&bit) {
		spl5();
		if ((DZADDR->dzcar & bit) == 0)
			while ((tp->t_state&CARR_ON)==0)
				sleep(&tp->t_rawq, TTIPRI);
		spl0();
	}
	tp->t_state=& ~WOPEN;
	tp->t_state=| ISOPEN | CARR_ON;
	if(u.u_procp->p_ttyp==0) u.u_procp->p_ttyp=tp;
}

/*
 * Close a DZ11 line.
 */
dzclose(dev)
{
	register minor;
	register struct tty *tp;

	minor = dev.d_minor;
	tp = &dz11[minor];
	if(dzok && (tp->t_flags&HUPCL)) {
		DZADDR->dzlpr = minor;  /* Turn off receiver */
		DZADDR->dzdtr =& ~bit;  /* Turn off DTR      */
	}
	tp->t_state =& SSTART|CARR_ON;
	wflushtty(tp);
}

/*
 * Read from a DZ11 line.
 */
dzread(dev)
{
	ttread(&dz11[dev.d_minor]);
}

/*
 * write on a DZ11 line
 */
dzwrite(dev)
{
	ttwrite(&dz11[dev.d_minor]);
	/* Contains a ttstart which in turn calls dzstart */
}

/*
 * stty/gtty for DZ11
 */
dzsgtty(dev, av)
int *av;
{
	register struct tty *tp;
	register r;

	tp = &dz11[dev.d_minor];
	if (ttystty(tp, av))  /* stty returns 0, gtty 1 */
		return;
	dzparam(tp);
}

/*
 * Set parameters from open or stty into the DZ hardware
 * registers.
 */
dzparam(atp)
struct tty *atp;
{
	register struct tty *tp;
	register int lpr, minor;

	if(!dzok) return;
	tp = atp;
	minor= tp - &dz11[0];
	spl5();
	DZADDR->dzcsr = MSE | IENAB;
	/*
	 * Hang up line?
	 */
	if (tp->t_speeds.lobyte<0) {
		tp->t_flags =| HUPCL;
		dzclose(tp->t_dev);
		return;
	}
	SSPEED[minor]= tp->t_speeds.hibyte;
	lpr = (tp->t_speeds & SPEED) | minor | RCVRON;
	if (tp->t_speeds.lobyte == 3)           /* 134.5 baud */
		lpr =| BITS6|PENABLE;
	else if (tp->t_flags&EVENP)
		if (tp->t_flags&ODDP)
			lpr =| BITS8;
		else lpr =| BITS7|PENABLE;
	else lpr =| BITS7|OPAR|PENABLE;
	if (tp->t_speeds.lobyte == 2)           /* 110 baud */
		lpr =| TWOSB;
	DZADDR->dzlpr = lpr;
	spl0();
}

/*
 * DZ11 receiver interrupt.
 * Entered at priority level 5
 */
dzrint()
{
	register struct tty *tp;
	register int c;

	while ((c = DZADDR->dzrbuf) < 0) {      /* char. present */
		tp = &dz11[(c>>8)&7];
		if((tp->t_state&ISOPEN)==0 || (c&RERROR)) {
			printf("dz ovr/par\n");
			wakeup(&tp->t_rawq);
			continue;
		}
		if (c&FRERROR)                  /* break */
			if (tp->t_flags&RAW)
				c = 0;          /* null (for getty) */
			else
				c = CINTR;      /* DEL (intr) */
		ttyinput(c, tp);
	}
}

/*
 * wakeup every so often to check for changes in line status
 */
dzsample(){
	register int c, c1;
	register struct tty *tp;

	c= c1= DZADDR->dzcar;
	c=^ dzcarr;             /* XOR with old value to find changes */
	dzcarr= c1;
	for(tp= &dz11[0]; c && tp< &dz11[NDZ11]; tp++, c=>>1)
		if(c&1){        /* Found a change */
			if((dzcarr&bit) == 0){  /* Line Dropped */
				if((tp->t_state&WOPEN)==0){
					signal(tp, SIGHUP);
					flushtty(tp); }
				tp->t_state=& ~CARR_ON; }
			else                    /* Carrier Detected */
				tp->t_state=| CARR_ON;
			wakeup(&tp->t_rawq); }
	if(timeout(&dzsample,0,DZSAMPLE)==0) sampling= 0; }

/*
 * DZ11 transmitter interrupt.
 * Scanner found a line needing service.
 * Feed it the next character or, if none, turn off dztcr bit
 */
dzxint()
{
	register struct tty *tp;
	register minor, c;
	extern ttrstrt();

	while ((minor = DZADDR->dzcsr) <  0) {  /* XINT bit test */
		minor =>> 8;  minor =& 7;       /* Form minor */
		tp = &dz11[minor];
		if((tp->t_state&SUSPEND) || (c = getc(&tp->t_outq)) < 0)
			goto toff;
		else {
			if (c <= 0177 || (tp->t_flags&RAW))  /* Normal character */
				DZADDR->dztbuf = c;
			else {
				tp->t_state =| TIMEOUT;
				timeout(ttrstrt,tp,c&0177);
	toff:                   DZADDR->dztcr =& ~bit;
			}
		if(tp->t_outq.c_cc==0 || tp->t_outq.c_cc == TTLOWAT )
			wakeup(&tp->t_outq);
		}
		minor =>> 4;    /* 2 microsecond delay for scanner */
	}
}

/*
 * Start (restart) transmission on the given DZ11 line.
 */
dzstart(atp)
struct tty *atp;
{
	register struct tty *tp;
	register sps;

	if(!dzok) return;
	sps = PS->integ;
	spl5();
	tp = atp;
	/*
	 * If it's currently delaying, return.
	 * Otherwise just be sure scan is on.
	 */
	if ((tp->t_state&(TIMEOUT|SUSPEND)) == 0)
		DZADDR->dztcr =| bit;
	PS->integ = sps;
}

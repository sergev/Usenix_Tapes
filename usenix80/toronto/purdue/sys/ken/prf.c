#
/*
 */

#include "../param.h"
#include "../seg.h"
#include "../buf.h"
#include "../conf.h"
#include "../systm.h"
#include "../errlog.h"

/*
 * Address and structure of the
 * KL-11 console device registers.
 */
struct
{
	int	rsr;
	int	rbr;
	int	xsr;
	int	xbr;
};

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

/*
 * sprintf() - printf onto statlog, for chit-chat, etc
 */

sprintf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
{
	register sps;
	register sverrlg;

	sps = PS->integ;
	spl6();
	sverrlg = errlg;
	errlg = -1;
	printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc);
	errlg = sverrlg;
	PS->integ = sps;
}

/*
 * eprintf() - printf onto errlog, for chit-chat, etc
 */

eprintf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
{
	register sps;
	register sverrlg;

	sps = PS->integ;
	spl6();
	sverrlg = errlg;
	errlg = 1;
	printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc);
	errlg = sverrlg;
	PS->integ = sps;
}

/*
 * sputchar() - putchar onto statlog, for chit-chat, etc
 */

sputchar(c)
{
	register sps;
	register sverrlg;

	sps = PS->integ;
	spl6();
	sverrlg = errlg;
	errlg = -1;
	putchar(c);
	errlg = sverrlg;
	PS->integ = sps;
}

/*
 * Scaled down version of C Library printf.
 * Only %s %l %d (==%l) %o are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 * 
 * If the system variable errlg is non zero, characters will be
 * stuffed on the clist instead of being printed on the console
 * with interrupts off.  The errlog minor dev (in mem.c) sucks
 * the characters back out and gives them to a user job to
 * print them out.  This avoids hanging the system for things
 * like magtape and other non system device errors.  Critical
 * errors (like panics) will still print on the console.  It
 * is the device driver's responsibility to set and clear "errlg".
 * -ghg 07/04/77
 */
printf(fmt,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
char fmt[];
{
	register char *s;
	register *adx, c;
	int p;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	p = 0;
	while((c = *fmt++) >= '0' && c <= '9')
		p = p*10 + c - '0';
	if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10, p);
	if(c == 's') {
		s = *adx;
		while(c = *s++)
			putchar(c);
	}
	if(c == 'c')
		putchar(*adx);
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base ba.
 */
printn(n, ba, p)
{
	register a;

	if((a = ldiv(n, ba)) != 0 || p > 1)
		printn(a, ba, p-1);
	putchar(lrem(n, ba) + '0');
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 * If errlg is > 0, output goes to clist for /dev/errlog,
 * If errlg is < 0, output goes to clist for /dev/statlog,
 * If errlg is 0, goes to console tty directly
 */
putchar(c)
{
	register rc, s;
	static int crflg,t[3];

	c =& 0177;
	if((rc = c) == 0)
		return;
	if(crflg){
		crflg = 0;
		prtime(t);
		printf("%2d:%2d:%2d  ",t[0],t[1],t[2]);
	}
	if(errlg > 0 && errlog.EOpen) {
		if(errlog.cc >= MAXERL)
			return;
		putc(rc, &errlog);
		if(rc == '\n') {
			wakeup(&errlog);
			crflg++;
		}
		return;
	}
	if(errlg < 0) {
		if(statlog.cc >= MAXSTA)
			return;
		putc(rc, &statlog);
		if(rc == '\n') {
			wakeup(&statlog);
			crflg++;
		}
	}
	else {
		while((KL->xsr&0200) == 0)  {}
		s = KL->xsr;
		KL->xsr = 0;
		KL->xbr = rc;
		if(rc == '\n') {
			putchar('\r');
			putchar(0177);
			putchar(0177);
			crflg++;
		}
		KL->xsr = s;
	}
}

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
	errlg = 0;	/* force message to console */
	panicstr = s;
	update();
	printf("panic: %s\n", s);
	for(;;)
		idle();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
{
	register k;

	k = dev.d_major;
	if(k >= 0 && (k <= nchrdev || k <= nblkdev) && devname[k])
		printf("\n%s on /dev/%s%l\n", str, devname[k], dev.d_minor);
	else
		printf("\n%s on dev %l/%l\n", str, k, dev.d_minor);
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
int *bp;
{
	register *rbp,a,x;

	rbp = bp;
	prdev("err", rbp->b_dev);
	printf("block# 0%l err 0%o  0%o", rbp->b_blkno, o1, o2);
	x = rbp->b_xmem&03;
	x =<< 1;
	a = rbp->b_addr;
	x =+ (a < 0 ? 1 : 0);
	a =& 077777;
	printf(" addr: %o%5o (18)\n",x,a);
}

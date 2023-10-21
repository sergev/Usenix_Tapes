f
/^#include .*\/systm\.h"$/a

/* Added macro call to support readnl. RLK */
.
-2t .
s:systm:file:
/if((ip->i_mode&IFMT) == IFCHR) {$/a
	/* Turn off the read-up-to new-line flag
	 * when using interruptable character devices.
	 * Line of code added by RLK */
		u.u_nlflag = 0;
.
/^	cp = bp->b_addr + o;$/a

/* When the flag is set by readnl, search the data about
 * to be transferred for a new-line, line-feed character.
 * If found adjust the byte counts requested
 * to include nothing beyond the new-line character.
 * "if" statement added by RLK */

	if (u.u_nlflag) {
		t = n;
		while (t--) if (*cp++ == 012) {
	/* the current number of bytes yet to transfer */
			n =- t;
	/* Lower originally requested number of bytes */
			u.u_arg[1] =- u.u_count-n;
	/* Lower total number of bytes yet to transfer */
			u.u_count = n;
			break;
		}
	/* Recalculate the transfer start address */
		cp = bp->b_addr + o;
	}

.
$a

/*
 * READNL is an added system call which acts like read
 * except that it does not transfer any characters beyond
 * the new-line, line-feed character on a single call
 * to block devices.  This provides line at a time input.
 * Function added by RLK.
 */

readnl()
{
	/* set flag used by iomove RLK */
	u.u_nlflag++;
	rdwr(FREAD);
	/* Reset flag here.
	 * This flag manipulation depends on having
	 * uninterruptable calls to block devices */
	u.u_nlflag = 0;
}
.
w
q

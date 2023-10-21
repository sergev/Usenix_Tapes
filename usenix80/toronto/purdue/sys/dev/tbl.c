#
/*
 */

/*
 *	system table read/write special file
 *	minor device number selects table, 0 <= dev.minor < ntbldev
 */

#include "../param.h"
#include "../user.h"
#include "../buf.h"
#include "../proc.h"
#include "../conf.h"
#include "../systm.h"
#include "../inode.h"
#include "../file.h"
#ifdef STATS
#include "../stats.h"
#endif

/*
 * Temp kludge Tony's APL tables, nobody can make-up their minds
 * on how to load these things, so made a table dev for now, so
 * tony can edit apltab in tty.c
 * --ghg 07/29/78.
 */

struct apltab {
	char	chr1;
	char	chr2;
	} apltab[32];
/* .......................................................... */

extern end;
struct tbldev {
	char	*tb_start;
	char	*tb_end;
} tbldev[] {
	0,	&end,		/* used for fast reference to data only */
	&proc,	&proc+1,	/* proc table */
	&u,	&u + 1,		/* user page */
	&file,	&file+1,	/* file table */
	&inode,	&inode+1,	/* in core inodes */
	&swapmap, &swapmap+1,	/* swap map */
#ifdef STATS
	&stats,	&stats+1,	/* system statistics */
#endif
	&apltab, &apltab+1	/* APL conversion tables in tty.c */
};
#define NTBLDEV (sizeof tbldev/ sizeof tbldev[0])
struct buf tblbuf;	/* all zeroes is correct init. */


tblopen(dev, flag)
{
# ifdef POWERFAIL
	if (dev == NODEV)
		return;
# endif
	if (dev.d_minor >= NTBLDEV)
		u.u_error = ENXIO;
}

tblread(dev)
{
	tblio(dev,B_READ);
}

tblwrite(dev)
{
	tblio(dev,B_WRITE);
}

tblio(dev,flag)
{
	register struct tbldev *tblp;
	register char *bp;

# ifdef XBUF
	*ka5 = praddr;  /* sort of kludgey to get proc table */
# endif
	tblp = &tbldev[dev.d_minor];
	if (u.u_offset[0] != 0) return;
	bp = u.u_offset[1] + tblp->tb_start;
	if (bp < tblp->tb_start || bp >= tblp->tb_end) return;
	iomove(&tblbuf,bp,min(tblp->tb_end-bp,u.u_count),flag);
}

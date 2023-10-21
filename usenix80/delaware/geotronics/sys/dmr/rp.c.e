169c
	if(c > rp_sizes[dev.d_minor & 017].nblocks) {
.
114c
	devstart(bp, &RPADDR->rpca, bp->cylin, bp->b_dev.d_minor>>4);
.
73,74c
	p1 = &rp_sizes[bp->b_dev.d_minor&017];
	if (bp->b_dev.d_minor >= (NRP<<4) ||
.
38a
/* The following were added for the Emulex controller.
 * Since an RP03 has 200 blocks/cylinder and an IBM 3330
 * has 150 blocks/cylinder, these partitions should
 * be multiples of 600 blocks (the drives' l.c.m.). */
	60600,	0,		/* cyl 0 thru 302 */
	61200,	303,		/* cyl 303 thru 608 */
	57000,	0,		/* cyl 0 thru 284 */
	57000,	303,		/* cyl 303 thru 587 */
	3600,	285,		/* cyl 285 thru 302 */
	4200,	588,		/* cyl 588 thru 608 */
	0,	0,		/* unassigned */
	0,	0,		/* unassigned */
.
35,36c
	65535,	0,		/* cyl 0 thru 327 */
	65535,	78,		/* cyl 78 thru 405 */
.
25c
#define	NRP	4
.
5,8d
2a
 * rp.c - RP11 disk driver (modified for Emulex SC11/A2)
 *
 *	modified 05-Apr-1980 by D A Gwyn:
 *	1) support extended block numbers for Emulex controller.
 *
 * Minor device numbers for drive 0 are now from 0..15,
 * drive 1 is from 16..31, etc.
 * The Emulex controller accesses a total of 609 RP blocks.
.
w
q

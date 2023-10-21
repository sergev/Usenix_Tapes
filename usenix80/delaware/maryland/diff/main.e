f
/estabur(0, 1, 0, 0);$/c
/*
 *	Include null overlay segment size RLK
 */
		estabur(0, 1, 0, 0, 0);
/*
 *		estabur(0, 1, 0, 0);
 */
.
/^estabur(nt, nd, ns, sep)$/c
/*
 *	The additional argument gives overlay segment size
 *	for writable I-space.  RLK
 */
estabur(nt, nd, ns, sep, no)
/*
 * estabur(nt, nd, ns, sep)
 */
.
/if(nseg(nt) > 8 || nseg(nd)+nseg(ns) > 8)$/c
/*
 *	Include size of writable I-space.  RLK
 */
		if(nseg(nt) + nseg(no) > 8 || nseg(nd)+nseg(ns) > 8)
/*
 *		if(nseg(nt) > 8 || nseg(nd)+nseg(ns) > 8)
 */
.
/if(nseg(nt)+nseg(nd)+nseg(ns) > 8)$/c
/*
 *	Include size of writable I-space.  RLK
 */
		if(no || nseg(nt)+nseg(nd)+nseg(ns) > 8)
/*
 *		if(nseg(nt)+nseg(nd)+nseg(ns) > 8)
 */
.
/if(nt+nd+ns+USIZE > maxmem)$/c
/*
 *	Include size of writable I-space.  RLK
 */
	if(nt+nd+ns+no+USIZE > maxmem)
/*
 *	if(nt+nd+ns+USIZE > maxmem)
 */
.
/if(sep)$/a
/*
 *	Set up prototypes for writable I-space.  RLK
 *	The following is added code.
 */
	{
	a = nd+ns+USIZE;
	while (no >= 128) {
		*dp++ = (127 << 8) | RW;
		*ap++ = a;
		a =+ 128;
		no =- 128;
		}
	/* Finish last partial segment of writable I-space */
	if (no) {
		*dp++ = ((no - 1) << 8) | RW;
		*ap++ = a;
	}
/*
 *	end of added code.
 */
.
/a = USIZE;$/-1a
/*
 *	This parenthesis added for balance.
 */
	}
.
w
q

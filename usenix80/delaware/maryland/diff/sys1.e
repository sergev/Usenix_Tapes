f
/estabur(/c
/*
 *	Include a null writable I-space length.  RLK
 */
	if(estabur(ts, ds, SSIZE, sep, 0))
/*
 *	if(estabur(ts, ds, SSIZE, sep))
 */
.
/estabur(/c
/*
 *	Includes a null overlay segment argument.  RLK
 */
	estabur(0, ds, 0, 0, 0);
/*
 *	estabur(0, ds, 0, 0);
 */
.
/estabur(/c
/*
 *	Reset size of overlay, writable I-space.  RLK
 */
	u.u_osize = 0;
	estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep,
		u.u_osize);
/*
 *	estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep);
 */
.
/(estabur(/-1,//c
/*
 *	Include overlay segment in total size computation.  RLK
 */
	n =+ USIZE+u.u_ssize+u.u_osize;
/*
 *	n =+ USIZE+u.u_ssize;
 *
 *	Include overlay, writable I-space in size checking.  RLK
 */
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep,
		u.u_osize))
/*
 *	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep))
 */
.
/^	a = u.u_procp->p_addr + n - u.u_ssize;$/c
	a = u.u_procp->p_addr + n - u.u_ssize - u.u_osize;
/*
 *	a = u.u_procp->p_addr + n - u.u_ssize;
 */
.
/^	n = u.u_ssize;$/c
/*
 *	Include size of overlay area when moving.  RLK
 */
	n = u.u_ssize + u.u_osize;
/*
 *	n = u.u_ssize;
 */
.
/^	n = u.u_ssize;$/c
/*
 *	Include the overlay segment with the stack
 *	when moving them up.  RLK
 */
	n = u.u_ssize + u.u_osize;
/*
 *	n = u.u_ssize;
 */
.
$a
/*
 *	OBREAK system call.
 *	Expand the writable I-space, overlay area which follows
 *	the stack segment.
 *	R0 contains some address within the highest segment to be
 *	allocated.
 *
 *	Added by RLK.
 */
obreak()
{
	register a, n, d;
	a = u.u_ar0[R0];
	n = ((a + 63) >> 6) & 01777;
	if(n==0 && a<0) n = 02000;
	n =- nseg(u.u_tsize) * 128;
	if(n<0) n = 0;
	d = n - u.u_osize;
	if(estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep, n))
		return;
	a = u.u_procp->p_size;
	expand(a + d);
	u.u_osize = n;
	if(d>0) {
		a =+ u.u_procp->p_addr;
		while(d--) clearseg(a++);
	}
}
.
w
q

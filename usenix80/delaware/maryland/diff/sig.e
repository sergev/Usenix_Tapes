f
/^	register s, \*ip;$/c
/*
 * More variables are needed with writable I-space overlay areas.  RLK
 */
	register s, *ip, i;
	int a;
/*
 *	register s, *ip;
 */
.
/estabur(0/-1,//c
/*
 *	First write data and stack segments.  RLK
 */
		s = u.u_dsize + u.u_ssize;
/*
 *		s = u.u_procp->p_size - USIZE;
 *
 *	Include the writable I-space in the area.  RLK
 */
		estabur(0, s, 0, u.u_sep, u.u_osize);
/*
 *		estabur(0, s, 0, 0);
 */
.
/^		writei(ip);$/a
/*
 *	Copy any writable I-space onto the data and stack area,
 *	then write it at end of file.  RLK
 *	The following if statement was added.
 */
		if(a = u.u_osize) {
			i = u.u_procp->p_addr + USIZE;
			while(a--) {
				copyseg(i+s, i);
				i++;
			}
			i = u.u_osize;
			estabur(0, i, 0, 0, 0);
			u.u_base = 0;
			u.u_count = i * 64;
			writei(ip);
		}
.
/(estabur(u.u_tsize,/c
/*
 *	Include writable I-space in size checking.  RLK
 */
	if(estabur(u.u_tsize, u.u_dsize, u.u_ssize+si, u.u_sep, 
		u.u_osize))
/*
 *	if(estabur(u.u_tsize, u.u_dsize, u.u_ssize+si, u.u_sep))
 */
.
/^	for(i=u.u_ssize; i; i--) {$/c
/*
 *	Move both stack and writable I-space up.  RLK
 */
	for(i=u.u_ssize+u.u_osize; i; i--) {
/*
 *	for(i=u.u_ssize; i; i--) {
 */
.
w
q

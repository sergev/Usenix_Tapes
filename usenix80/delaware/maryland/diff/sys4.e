f
$a

/*
 *	MFPIT System call.
 *	The content of the location in USER I-space
 *	given by R0 is returned in R0.
 *	Because the PDP 11/45 was designed to support execute
 *	only code, USER I-space may not be read with MFPI by USER
 *	programs, even though MTPI may write directly.
 *	To overcome this annoyance, the system reads USER I-space
 *	while in KERNEL mode.
 *	Added by RLK.
 */
mfpit()
{
	u.u_ar0[R0] = fuiword(u.u_ar0[R0]);
}
.
w
q

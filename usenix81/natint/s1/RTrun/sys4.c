/*
 * put this in sys4.c
 * note addition to user structure
 * and mods to trap.c
 *
 * use sysent initialization:
 *      0, &setwin,                     /* 62 = setwin
 */

/*
 * set catchable trap window
 */
setwin(){
	u.u_window[0]= u.u_ar0[R0];
	u.u_window[1]= u.u_ar0[R1];
	}

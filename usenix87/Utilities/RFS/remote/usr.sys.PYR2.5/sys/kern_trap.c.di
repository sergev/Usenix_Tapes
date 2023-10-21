These changes enable syscall() to start up a remote version of a system
call if the normal system call fails with error == EISREMOTE.
***************
*** 47,52
  #include "../h/vmmac.h"
  #include "../h/vmmeter.h"
  #include "../h/debug.h"
  
  extern int kernstrt;
  #define	USER	040		/* user-mode flag added to type */

--- 47,59 -----
  #include "../h/vmmac.h"
  #include "../h/vmmeter.h"
  #include "../h/debug.h"
+ #ifdef REMOTEFS
+ #include "../remote/remotefs.h"
+ /*
+  * needed only if EISREMOTE isn't in /usr/include/errno.h
+  */
+ #include "../h/errno.h"
+ #endif REMOTEFS
  
  extern int kernstrt;
  #define	USER	040		/* user-mode flag added to type */
***************
*** 395,400
  	register long syst_sec, syst_usec;
  	register int s;
  	register int onslave = 0;
  
  	SETGR9();	/* for dual cpu, put mastercpu into gr9 */
  	spl0();		/* move up to system base priority level */

--- 402,411 -----
  	register long syst_sec, syst_usec;
  	register int s;
  	register int onslave = 0;
+ #ifdef REMOTEFS
+ 	extern u_char remote_sysmap[];
+ 	register int rmt_syscall, runremote, rmtcalled, rmtcnt;
+ #endif REMOTEFS
  
  	SETGR9();	/* for dual cpu, put mastercpu into gr9 */
  	spl0();		/* move up to system base priority level */
***************
*** 418,424
  		if (u.u_error == 0 && u.u_eosys == JUSTRETURN)
  			u.u_error = EINTR;
  	} else {
! 		(*(callproc->sy_call))(arg1, arg2, arg3, arg4, arg5, arg6);
  	}
  
  	if (u.u_eosys != RESTARTSYS)

--- 429,461 -----
  		if (u.u_error == 0 && u.u_eosys == JUSTRETURN)
  			u.u_error = EINTR;
  	} else {
! #ifdef REMOTEFS
! 		if (index < 512) {
! 			rmt_syscall = remote_sysmap[ index ];
! 			rmtcalled = FALSE;
! 			rmtcnt = 0;
! 
! 			u.u_eosys = JUSTRETURN;
! 			while (! rmtcalled) {
! 				runremote = (rmt_syscall != RSYS_nosys
! 					    && u.u_procp->p_flag & SREMOTE);
! 				if (runremote)
! 					rmtcalled = remote_startup(rmtcnt,
! 						rmt_syscall, arg1, arg2, arg3,
! 						arg4, arg5, arg6);
! 				if (! rmtcalled) {
! 					(*callproc->sy_call)(arg1, arg2, arg3,
! 						arg4, arg5, arg6);
! 					if (u.u_error != EISREMOTE)
! 						rmtcalled = TRUE;
! 					else
! 						rmtcnt++;
! 				}
! 			}
! 		} else
! #endif REMOTEFS
! 			(*(callproc->sy_call))(arg1, arg2, arg3,
! 				arg4, arg5, arg6);
  	}
  
  	if (u.u_eosys != RESTARTSYS)

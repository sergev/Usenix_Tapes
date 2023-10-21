The following changes ensure that upon exit, a process notifies any remote
servers that may know about him, that he is indeed dead.
***************
*** 23,28
  #include "mbuf.h"
  #include "inode.h"
  #include "syslog.h"
  
  /*
   * Exit system call: pass back caller's arg

--- 23,31 -----
  #include "mbuf.h"
  #include "inode.h"
  #include "syslog.h"
+ #ifdef REMOTEFS
+ #include "../remote/remotefs.h"
+ #endif REMOTEFS
  
  /*
   * Exit system call: pass back caller's arg
***************
*** 56,61
  	vmsizmon();
  #endif
  	p = u.u_procp;
  	p->p_flag &= ~(STRC|SULOCK);
  	p->p_flag |= SWEXIT;
  	p->p_sigignore = ~0;

--- 59,71 -----
  	vmsizmon();
  #endif
  	p = u.u_procp;
+ #ifdef REMOTEFS
+ 	/*
+ 	 * First, release our server.
+ 	 */
+ 	if (p->p_flag & SREMOTE)
+ 		remote_exit();
+ #endif REMOTEFS
  	p->p_flag &= ~(STRC|SULOCK);
  	p->p_flag |= SWEXIT;
  	p->p_sigignore = ~0;

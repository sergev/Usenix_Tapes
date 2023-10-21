The following changes ensure that upon exit, a process notifies any remote
servers that may know about him, that he is indeed dead.
***************
*** 33,38
  #include "../h/file.h"
  #include "../h/mbuf.h"
  #include "../h/inode.h"
  
  /*
   * Exit system call: pass back caller's arg

--- 33,41 -----
  #include "../h/file.h"
  #include "../h/mbuf.h"
  #include "../h/inode.h"
+ #ifdef REMOTEFS
+ #include "../remote/remotefs.h"
+ #endif REMOTEFS
  
  /*
   * Exit system call: pass back caller's arg
***************
*** 66,71
  	vmsizmon();
  #endif
  	p = u.u_procp;
  	p->p_flag &= ~(STRC|SULOCK);
  	p->p_flag |= SWEXIT;
  	p->p_sigignore = ~0;

--- 69,81 -----
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

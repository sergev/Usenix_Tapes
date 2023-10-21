The following changes ensure that upon exit, a process notifies any remote
servers that may know about him, that he is indeed dead.
***************
*** 59,64
  #include "../h/quota.h"
  #include "../h/cmap.h"
  #include "../h/text.h"
  
  /*
   * Exit system call: pass back caller's arg

--- 59,67 -----
  #include "../h/quota.h"
  #include "../h/cmap.h"
  #include "../h/text.h"
+ #ifdef REMOTEFS
+ #include "../remote/remotefs.h"
+ #endif REMOTEFS
  
  /*
   * Exit system call: pass back caller's arg
***************
*** 99,104
  	vmsizmon();
  #endif
  	p = u.u_procp;
  
  	gpid = p->p_gpid;
  	/*

--- 102,114 -----
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
  
  	gpid = p->p_gpid;
  	/*

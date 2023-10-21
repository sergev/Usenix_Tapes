
*** /tmp/,RCSt1014645	Fri Apr 17 22:57:35 1987
--- openpty.c	Fri Apr 17 22:57:06 1987
***************
*** 5,11
  /*
   *------------------------------------------------------------------
   * Copyright 1987, Josh Siegel
-  * All rights reserved.
   *
   * Josh Siegel (siegel@hc.dspo.gov)
   * Dept of Electrical and Computer Engineering,

--- 5,10 -----
  /*
   *------------------------------------------------------------------
   * Copyright 1987, Josh Siegel
   *
   * Josh Siegel (siegel@hc.dspo.gov)
   * Dept of Electrical and Computer Engineering,
***************
*** 35,40
  
  #include "netw.h"
  #include "openpty.h"
  #include <sys/ioctl.h>
  
  _loadtty(cond)

--- 34,40 -----
  
  #include "netw.h"
  #include "openpty.h"
+ #include <sys/file.h>
  #include <sys/ioctl.h>
  
  _loadtty(cond)
***************
*** 83,89
  		return (proc.pt_pfd);
  	}
  
! 	t = open("/dev/tty", 2);
  
  	if (t >= 0) {
  		ioctl(t, TIOCNOTTY, 0);

--- 83,89 -----
  		return (proc.pt_pfd);
  	}
  
! 	t = open("/dev/tty", O_RDWR);
  
  	if (t >= 0) {
  		ioctl(t, TIOCNOTTY, 0);

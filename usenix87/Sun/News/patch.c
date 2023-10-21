
/\/\* #define GHNAME/a
#define HNAME "news-host-name"
.
p
-----Then make the following change to uname.c-----
*** uname.c.orig	Fri Mar  8 13:57:43 1985
--- uname.c	Mon Sep 15 10:59:09 1986
***************
*** 13,18
  
  #include "params.h"
  
  #ifdef UNAME
  # define DONE
  #endif

--- 13,28 -----
  
  #include "params.h"
  
+ #ifdef HNAME
+ /* use a fixed hostname, usefull when sharing news via network file system */
+ uname(uptr)
+     struct utsname *uptr;
+ {
+ 	strncpy(uptr->nodename, HNAME, sizeof (uptr->nodename));
+ }
+ # define DONE
+ #endif HNAME
+ 
  #ifdef UNAME
  # define DONE
  #endif

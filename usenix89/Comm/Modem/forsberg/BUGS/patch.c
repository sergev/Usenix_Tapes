

*** rbsb.c.00	Sun Oct 26 19:10:13 1986
--- rbsb.c	Mon Oct 27 00:05:49 1986
***************
*** 219,224
  	return ((int) lf);
  }
  #endif
  
  #ifdef CRCTABLE
  /* crctab calculated by Mark G. Mendel, Network Systems Corporation */

--- 219,243 -----
  	return ((int) lf);
  }
  #endif
+ #ifdef SVR2
+ #define READCHECK
+ #include <fcntl.h>
+ 
+ char checked = '\0' ;
+ /*
+  * Nonblocking I/O is a bit different in System V, Release 2
+  */
+ rdchk(f)
+ {
+ 	int lf, savestat = fcntl(f, F_GETFL) ;
+ 
+ 	fcntl(f, F_SETFL, savestat | O_NDELAY) ;
+ 	lf = read(f, &checked, 1) ;
+ 	fcntl(f, F_SETFL, savestat) ;
+ 	return(lf) ;
+ }
+ #endif
+ 
  
  #ifdef CRCTABLE
  /* crctab calculated by Mark G. Mendel, Network Systems Corporation */



*** sz.c.00	Mon Oct 27 00:06:02 1986
--- sz.c	Mon Oct 27 00:25:52 1986
***************
*** 6,11
   * sz.c By Chuck Forsberg
   *
   *	cc -O sz.c -o sz		USG (SYS III/V) Unix
   * 	cc -O -DV7  sz.c -o sz		Unix Version 7, 2.8 - 4.3 BSD
   *
   *		define CRCTABLE to use table driven CRC

--- 6,13 -----
   * sz.c By Chuck Forsberg
   *
   *	cc -O sz.c -o sz		USG (SYS III/V) Unix
+  *  cc -O -DSVR2 sz.c -o sz		Sys V Release 2 with non-blocking input
+  *								Define to allow reverse channel checking
   * 	cc -O -DV7  sz.c -o sz		Unix Version 7, 2.8 - 4.3 BSD
   *
   *		define CRCTABLE to use table driven CRC
***************
*** 1069,1074
  				tcount += strlen(qbf);
  #ifdef READCHECK
  				while (rdchk(iofd)) {
  					switch (readline(1)) {
  					case CAN:
  					case ZPAD:

--- 1071,1079 -----
  				tcount += strlen(qbf);
  #ifdef READCHECK
  				while (rdchk(iofd)) {
+ #ifdef SVR2
+ 					switch (checked) {
+ #else
  					switch (readline(1)) {
  #endif
  					case CAN:
***************
*** 1070,1075
  #ifdef READCHECK
  				while (rdchk(iofd)) {
  					switch (readline(1)) {
  					case CAN:
  					case ZPAD:
  #ifdef TCFLSH

--- 1075,1081 -----
  					switch (checked) {
  #else
  					switch (readline(1)) {
+ #endif
  					case CAN:
  					case ZPAD:
  #ifdef TCFLSH
***************
*** 1119,1124
  		 */
  		fflush(stdout);
  		while (rdchk(iofd)) {
  			switch (readline(1)) {
  			case CAN:
  			case ZPAD:

--- 1125,1133 -----
  		 */
  		fflush(stdout);
  		while (rdchk(iofd)) {
+ #ifdef SVR2
+ 			switch (checked) {
+ #else
  			switch (readline(1)) {
  #endif
  			case CAN:
***************
*** 1120,1125
  		fflush(stdout);
  		while (rdchk(iofd)) {
  			switch (readline(1)) {
  			case CAN:
  			case ZPAD:
  #ifdef TCFLSH

--- 1129,1135 -----
  			switch (checked) {
  #else
  			switch (readline(1)) {
+ #endif
  			case CAN:
  			case ZPAD:
  #ifdef TCFLSH



-- 
    Dan Frank
    uucp: ... uwvax!prairie!dan
    arpa: dan%caseus@spool.wisc.edu

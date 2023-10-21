#include "uutty.h"
#include <signal.h>

sig_1() {P("Signal  1 [SIGHUP]" ); fflush(stdout); die( 1);}
sig_2() {P("Signal  2 [SIGINT]" ); fflush(stdout); die( 2);}
sig_3() {P("Signal  3 [SIGQUIT]"); fflush(stdout); die( 3);}
sig_4() {P("Signal  4 [SIGILL]" ); fflush(stdout); die( 4);}
sig_5() {P("Signal  5 [SIGTRAP]"); fflush(stdout); die( 5);}
sig_6() {P("Signal  6 [SIGIOT]" ); fflush(stdout); die( 6);}
sig_7() {P("Signal  7 [SIGEMT]" ); fflush(stdout); die( 7);}
sig_8() {P("Signal  8 [SIGFPE]" ); fflush(stdout); die( 8);}
sig_9() {P("Signal  9 [SIGKILL]"); fflush(stdout); die( 9);}
sig10() {P("Signal 10 [SIGBUS]" ); fflush(stdout); die(10);}
sig11() {P("Signal 11 [SIGSEGV]"); fflush(stdout); die(11);}
sig12() {P("Signal 12 [SIGSYS]" ); fflush(stdout); die(12);}
sig13() {P("Signal 13 [SIGPIPE]"); fflush(stdout); die(13);}
sig14() {P("Signal 14 [SIGALRM]"); fflush(stdout); die(14);}
sig15() {P("Signal 15 [SIGTERM]"); fflush(stdout); die(15);}
sig16() {P("Signal 16 [SIGADDR]"); fflush(stdout); die(16);}
sig17() {P("Signal 17 [SIGZERO]"); fflush(stdout); die(17);}
sig18() {P("Signal 18 [SIGCHK]" ); fflush(stdout); die(18);}
sig19() {P("Signal 19 [SIGOVER]"); fflush(stdout); die(19);}
sig20() {P("Signal 20 [SIGPRIV]"); fflush(stdout); die(20);}
sig21() {P("Signal 21 [SIGUSR1]"); fflush(stdout); die(21);}
sig22() {P("Signal 22 [SIGUSR2]"); fflush(stdout); die(22);}

/* Catch all the signals we can:
*/
sig() 
{
  signal( 1,sig_1);
  signal( 2,sig_2);
  signal( 3,sig_3);
  signal( 4,sig_4);
  signal( 5,sig_5);
  signal( 6,sig_6);
  signal( 7,sig_7);
  signal( 8,sig_8);
  signal( 9,sig_9);
  signal(10,sig10);
  signal(11,sig11);
  signal(11,sig11);
  signal(12,sig12);
  signal(13,sig13);
  signal(14,sig14);
  signal(15,sig15);
  signal(16,sig16);
  signal(17,sig17);
  signal(18,sig18);
  signal(19,sig19);
  signal(20,sig20);
  signal(21,sig21);
  signal(22,sig22);
}


/* Define how to access the structure that the wait system call stores.
   On many systems, there is a structure defined for this.
   But on vanilla-ish USG systems there is not.  */

#ifndef HAVE_WAIT_STRUCT
#define WAITTYPE int
#define WIFSTOPPED(w) (((w)&0377) == 0177)
#define WIFSIGNALED(w) (((w)&0377) != 0177 && ((w)&~0377) == 0)
#define WIFEXITED(w) (((w)&0377) == 0)
#define WRETCODE(w) ((w) >> 8)
#define WSTOPSIG(w) ((w) >> 8)
#define WCOREDUMP(w) (((w)&0200) != 0)
#define WTERMSIG(w) ((w) & 0177)
#define WSETSTOP(w,sig)  ((w) = (0177 | ((sig) << 8)))
#else
#include <sys/wait.h>
#define WAITTYPE union wait
#define WRETCODE(w) (w).w_retcode
#define WSTOPSIG(w) (w).w_stopsig
#define WCOREDUMP(w) (w).w_coredump
#define WTERMSIG(w) (w).w_termsig
#define WSETSTOP(w,sig)  \
  ((w).stopsig = (sig), (w).coredump = 0, (w).termsig = 0177)
#endif

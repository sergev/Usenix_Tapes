#include "process.h"
struct process shellproc;
#define shellpid shellproc.pr_pid
#define fmshell  shellproc.pr_fmbkg
#define toshell  shellproc.pr_tobkg

int	onfpe();		/* handler for shell signal */
int	getpipe();		/* append function for shell output */
int	getstring();

int	*lastcmd;		/* address of last shell command */
int	shellactive;		/* set between sendtoshell and return */

#ifdef vax
#define SIGSHELL SIGTRAP
#else
#define SIGSHELL SIGFPE
#endif

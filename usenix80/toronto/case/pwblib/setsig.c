#include "param.h"

char setsig_____[] "~|^`setsig.c:	3.2";
/*
	General-purpose signal setting routine.  Hangup, interrupt and
	quit signals are set according to h, i and q arguments,
	but only if they are not already set to be ignored.  Other
	catchable signals (except for trace trap) are caught and a
	"user-oriented" message is printed on file descriptor 2 with
	a number for help(I).  Termination is like that of "fatal",
	via "clean_up" and "exit2".

	If the file "dump.core" exists in the current directory and
	a signal greater than 3 is received the function commits
	suicide to produce a core dump.
*/


char	*mesg[NSIG] {
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	0,
	"Alarm clock"
};


setsig(h,i,q)
int h, i, q;
{
	extern int setsig1();
	register int j, n;

	if(signal(1,1) == 0) signal(1,h);
	if(signal(2,1) == 0) signal(2,i);
	if(signal(3,1) == 0) signal(3,q);
	for (j=3; j<NSIG; j++){
		if (j == 4) continue;
		if (n=signal(j,setsig1))
			signal(j,n);}
}


setsig1(n)
int n;
{
	if (mesg[n])
		msg2("ERROR: hardware -- ",mesg[n]," (115)\n");
	clean_up();
	if(open("dump.core",0) > 0) {
		signal(6,0);
		abort();
	}
	exit2(1);
}


#include "uutty.h"
#include <signal.h>
static char id[] = "@(#)RELEASE: 1.0  Sep 20 1986 ~jc/uutty";
/*
**	uutty device [option]...
**
** This is John Chambers' own personal serial-port daemon, 
** to watch a port, determine what sort of critter is at the 
** other end of the line, and engage it in a semi-intelligent 
** dialog.  The primary goal of this version, uutty, is to
** make it possible to use direct links (null modems) freely in
** both directions, without the problems caused by the getty(1)
** program.  In particular, it allows the use of the uucp package
** (including uux and cu) in both directions across a line.
**
** Another thing uutty is good for is using a modem in either
** without needing any special actions.
**
** Uutty is a thinly-disguised "state machine" whose basic 
** behavior is determined by a global "state variable" ss
** and a string of input characters.  The state variable is
** simply an integer that encodes the last action performed.  
** An input routine is called, and when it returns, the input
** is examined to determine the program's next action.  Normally, 
** this action is to produce an output string, change the state 
** variable, and wait for the next input.
**
** The major intent of this program is to do a login interview,
** but to ignore anyone at the other end of the line that acts
** like it's trying to log us in.  If getty or this program is
** at the other end, the result will be silence until someone
** sends a CR or LF (which elicits "login:") or something that
** is acceptable as a login id.  Note in particular that input
** containing colons and spaces will be ignored.
**
** The other major intent is to cooperate with any software that
** sets the uucp lockfiles ("/usr/spool/uucp/LCK..<device>"), by
** checking for such lockfiles frequently, and sleeping any time
** they are found.  This program never creates a lockfile; it just
** sleeps when someone else creates one.  Thus, this program may
** be left running on a port used by uucp, cu, or other such serial-
** port users.  There is a slight possibility that this program
** may eat up part of the first response from the port, but that
** is rarely much of a problem.  One or two CRs will usually suffice
** to elicit a "gin:" prompt from the other end.
**
** A third, minor intent is to recognize overly-intelligent modems,
** and refuse to become engaged in a conversations with them.  This
** is done primarily by rejecting input that contains whitespace or
** special characters, or which contains only upper-case letters.
** This will handle the error messages produced by most commercial
** modems.  However, users may wish to add to the code that examines
** login ids and passwords, and add more checks for specific modems.
**
** You may notice that this program is full of Dx(...) debug calls.
** These are defined in dbg.h, and may be suppressed fairly easily.
** Most C compilers will react to, for instance:
** 	#define D9 if(0) dmsg
** by generating no code at all.  You might think this would make
** your program smaller and faster, and you'd be partly right.  It
** will definitely be smaller, but probably not much faster.  Anyhow,
** the speed of this program isn't much of an issue; if it gets into
** a feedback loop with a modem, you don't much care how efficiently
** it brings your system to its knees!  It's probably a wiser idea
** to leave most of the diagnostic junk in.  You'll be surprised at
** how useful it can be to have that stuff in an audit trail.
**
** Some of the options recognized are:
**
** -b<n>    use a baud rate of <n>.  Final "00" may be omitted.
**
** -c<n>    slow down by counting to <n> between output chunks.
**
** -d<n>    debug level <n>; default is -d1.
**
** -e<s>    exit string <s> to send when terminating.
**
** -f       fork a subprocess for starting shells.
**
** -i<s>    initialization string <s> is sent whenever it is
**          decided that the other end is insane or jabbering.
**
** -l       create uucp lockfiles before starting a shell; delete 
**          them afterwards.  This option implies the -f option.
**
** -n<s>    nudge string <s> is sent when this program wants a response.
**
** -p<f>    port to use, where <f> is usually /dev/tty??.  This is
**			the same as using <f> without any prefix.
**
** -s<n>    sleep <n> seconds between output chunks.
**
** -x<n>    debug level <n>.
**
** To be responsible, I should repeat the warning stated elsewhere:
** with debug level -d2 or higher, uutty writes everything it sees,
** including unencrypted login ids and passwords, to its diagnostic
** output stream (audit trail).  This makes it a Trojan Horse of the
** first kind.  If you are security-conscious, you might take steps
** to hide the audit trail, and purge it frequently.
**
** In any case, the audit trail grows without bounds, so you will want
** to take steps to keep it within bounds.  The easiest way is with an
** entry in crontab that starts up a daily cleanup script (/etc/cleanup
** is a real good name for it).  This script can move the uutty audit 
** trails to a different name, to produce one level of backup.
*/
main(ac,av)
	char**av;
{	int i, r;

	time(&currtime);		/* Note the start time */
	progname = av[0];		/* For debug output */
	prgnam = lastfield(progname,'/');
	D4("prgnam=\"%s\" progname=\"%s\"",prgnam,progname);
	euid = geteuid();
	egid = getegid();
	ruid = getuid();
	rgid = getgid();
	D3("euid=%d egid=%d ruid=%d rgid=%d",euid,egid,ruid,rgid);
	args(ac,av);			/* Process command-line args */
	if (debug) {			/* Generate first entries in audit trail */
		P("+--------------------------------------------------------------------+");
		P("%s %s started.",getime(),progname);
	}
	timeout = 255;			/* Use max timeout on reads */
	sig();				/* Intercept all the signals */
	slowfl = (count > 0) || (slow > 0);
	opendev();			/* Open the port which we are to use */
	if (dev != 0) {close(0); i = dup(dev); D3("File %d=\"%s\"",i,device);}
	if (dev != 1) {close(1); i = dup(dev); D3("File %d=\"%s\"",i,device);}
	lockwait();			/* First check to see if it's busy */
/*
** The code to handle raw, 8-bit communication get a bit weird from
** system to system.  Here, we try to trap the initial state of the
** port, so that when we die, we can first restore it.
*/
#ifdef SYS5
	if (raw && isatty(dev)) {	/* We will want to restore its initial state */
		D5("main:before ioctl(%d,%d,%06lX)",dev,TCGETA,&trminit);
		i = ioctl(dev,TCGETA,&trminit);  /* Note initial state of terminal */
		D5("main: after ioctl(%d,%d,%06lX)=%d",dev,TCGETA,&trminit,i);
		D7("main: %d:\tcflag=%06o",dev,trminit.c_cflag);
		D7("main: %d:\tiflag=%06o",dev,trminit.c_iflag);
		D7("main: %d:\tlflag=%06o",dev,trminit.c_lflag);
		D7("main: %d:\toflag=%06o",dev,trminit.c_oflag);
		termfl = 1;
/*	crlf = "\n\r";			** Not needed on Unix systems */
	}
#endif
	restdev();			/* Get port to desired (raw, very public) state */
#ifdef SYS5
/*
** The following functions suffice to find, update, and write
** the appropriate entry in the /etc/utmp file, which on SYS/V
** is the record of logged-in users.  This may well have to be
** changed for other systems.  The hope here is that this is a
** reasonable way to modularize the job, though the names may
** not be ideal...
*/
	D8("main:before findutmp()");
	up = findutmp();		/* Try to locate our /etc/utmp entry */
	D7("main: after findutmp()=%06lX",up);
	D8("main:before fillutmp(\"%s\",%X,\"%s\",%d)",prgnam,(char*)0,devfld,LOGIN_PROCESS);
	fillutmp(prgnam,(char*)0,devfld,LOGIN_PROCESS);
	D7("main: after fillutmp()");
	D8("pswd:before pututline(%06lX)",up);
	pututline(up);			/* Put modified line into /etc/utmp */
	D7("pswd: after pututline(%06lX)",up);
#endif
	r = talk();			/* Attempt a conversation */
	die(r);			/* If we should ever decide to stop */
	return 0;			/* Paranoia! */
}

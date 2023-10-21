#include "uutty.h"
/* 
** The -h option triggers this routine.  We should try to
** keep it up to date, so users can find out easily what
** options are currently supported.
*/
help()
{
	P("usage: %s [-options] [device] [2>auditfile]",progname);
	P("options are:");
	P("\t-b<n>\tSet baud rate to <n>.");
	P("\t-c<n>\tCount to <n> before reads.");
	P("\t-e<msg>\tMessage to send to port before exiting.");
	P("\t-f\tFork (create subprocess) to start shells.");
	P("\t-h\tHelp--display this information.");
	P("\t-i<msg>\tMessage to send for initalizing port.");
	P("\t-l\tCreate uucp lockfiles for successful logins.");
	P("\t-n<msg>\tMessage to send to nudge the port.");
	P("\t-p<f>\tPort name is <f>.");
	P("\t-r<n>\tRaw I/O [default=1=true].");
	P("\t-s<n>\tSleep <n> seconds before reads.");
	P("\t-x<n>\tDebug level of <n>.");
	P("Note that error output [file 2] is used for debugging.");
}

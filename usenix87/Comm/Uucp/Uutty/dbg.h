#ifndef dbg_h
#include <stdio.h>
#include <sys/types.h>
/*
** The global 'debug' variable controls the level of diagnostics
** to be produced.  Some suggested conventions are:
**	debug=0	fatal error messages only.
**	debug=1	diagnostic: serious messages and warnings, default.
**	debug=2	informative: major events and decisions.
**	debug=3	gabby: tracing of likely trouble spots.
**	debug=4	tracing of important events for troubleshooting.
**	debug=5	major function calls and returns.
**	debug=6	details of flow of control.
**	debug=7	major data tracing.
**	debug=8	detailed data tracing.
**	debug=9	everything of conceivable interest to anyone.
** Of course, it is entirely up to you how you use the debug 
** level, and many applications won't have a need for 10 levels. 
** It will help others trying to use your code if they can use
** the debug level easily to find out what's wrong.  At level
** 1, in particular, it is a good idea to give enough info to
** tell the user what went wrong.  For instance, don't just say
** that you can't write to a file; show the pathname that you
** failed to open for writing, and the errno code (even better,
** give the sys_errlist[errno] message).
*/
typedef          char  Flag;
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;
/*
** Main debug-message pseudo-functions:
*/
#define D  if(debug> 1)dmsg
#define D1 if(debug>=1)dmsg
#define D2 if(debug>=2)dmsg
#define D3 if(debug>=3)dmsg
#define D4 if(debug>=4)dmsg
#define D5 if(debug>=5)dmsg
#define D6 if(debug>=6)dmsg
#define D7 if(debug>=7)dmsg
#define D8 if(debug>=8)dmsg
#define D9 if(debug>=9)dmsg
#define E emsg
#define P pmsg
#define V if(debug>0)pmsg
#define W wmsg
/*
** These functions produce a 1-line dump, with non-printable
** characters converted to 2-char escape sequences.
*/
#define Ascdump(a,n,o,m) ascdump((char*)(a),(U32)(n),(U32)(o),(char*)(m))
#define Ascdp(a,p) 	Ascdump(a,p-a+1,a,0) 
#define Ascdn(a,n) 	Ascdump(a,n,a,0) 
#define Ascdpm(a,p,m)	Ascdump(a,p-a+1,0,m)
#define Ascdnm(a,n,m)	Ascdump(a,n,0,m)
/*
** These functions produce a 3-line dump, consisting of displayable
** characters, high-order hex digit, and low-order hex digit.  On
** the first line, non-printing characters are shown as '_'.
*/
#define Hexdump(a,n,o,m) hexdump((char*)(a),(U32)(n),(U32)(o),(char*)(m))
#define Hexdp(a,p) 	Hexdump(a,p-a+1,a,0) 
#define Hexdn(a,n) 	Hexdump(a,n,a,0) 
#define Hexdpm(a,p,m)	Hexdump(a,p-a+1,0,m)
#define Hexdnm(a,n,m)	Hexdump(a,n,0L,m)

extern int   ascdump();	/* The actual ASCII-dump routine */
extern char *crlf;	/* Line terminator string, usually "\n" */
extern int   debug;	/* Debug level, 0-9, default=1 */
extern int   dbgsleep;	/* Sleep time after output, default=0 */
extern FILE *dbgout;	/* Diagnostic output stream, default=stderr */
extern int   dsp();	/* Convert an int to a printable char */
extern char *dbgtimep;	/* Time to display in debug messages */
extern int   errno;	/* Unix global error code */
extern int   hexdump();	/* The actual hex-dump routine */
extern char *progname;	/* Name of running program, from argv[0] */

#define dbg_h
#endif

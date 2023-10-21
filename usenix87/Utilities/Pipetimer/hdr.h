/* imported definitions */
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>

/* constants */
#define NO_BYTES	(1024*1024)
#define MIN_CHUNK       128
#define MAX_CHUNK       4096

#define MAX_ERRORS      5

/* global variables */
char buf[MAX_CHUNK];
struct timeval start_time;
struct timeval end_time;
struct timezone time_zone;

/* function declarations */
void timesPrint();

/* dummy error handling macros */
#define ErrorMsg(File,Line,Format,Valuea,Valueb) \
		fprintf(stderr, "%s, line %d:", File, Line); \
		fprintf(stderr, Format, Valuea, Valueb); \
		fprintf(stderr, "\n" ); \
		fflush (stderr);

#define ErrorSystem(File,Line,Fileno,Rc) \
	    { \
		extern int errno; \
		extern char* sys_errlist[]; \
	        \
		if (Rc < 0) { \
		    ErrorMsg(File, Line, \
	    "negative return code (%d) returned by system call for fd %d", \
			     Rc, Fileno); \
		    ErrorMsg(File, Line, \
			     "errno was %d.  corresponding msg is '%s'", \
			     errno, sys_errlist[errno]); \
		    } \
	     }
/* 
 *  Note: I have a real error handling library but it is too difficult to
 *        distribute for something as small as this.  mwherman@watcgl
 */

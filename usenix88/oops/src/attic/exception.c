/* exception.c -- Ada -like exception handler library routines

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: {decvax!}seismo!elsie!cecil!keith
	February, 1986

Function:

Run-time support for Ada -like exception handling.
	
Modification History:
	
*/
#include <stream.h>
#include <osfcn.h>
#include <libc.h>
#include "exception.h"

ExceptionEnv* exception_env_stack_top;
ExceptionEnv lastResort;

void ExceptionEnv::raise(int exception)
{
	if (exception == 0) {
		cerr << "Tried to RAISE exception code 0\n";
		abort();
	}
	if (prev == 0) {	// i.e.,  this == &lastResort
		cerr << "Unhandled exception code " << exception << "\n";
		exit(1);
	}
	pop();
	longjmp(env,exception);
}

/* exception.h -- Ada -like exception handler

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
	
Declarations for Ada -like exception handling.

Modification History:
	
*/

#ifndef EXCEPTIONH
#define EXCEPTIONH

#include <setjmp.h>

class ExceptionEnv;
extern ExceptionEnv* exception_env_stack_top;

class ExceptionEnv {
	ExceptionEnv* prev;
	int exceptionCode;
	jmp_buf env;
public:
	ExceptionEnv() {	// MUST be inline
		prev = exception_env_stack_top;
		exception_env_stack_top = this;
		exceptionCode = setjmp(env);
	}
	~ExceptionEnv() { if (exception_env_stack_top == this) pop(); }
	int code()	{ return exceptionCode; }
	void pop()	{ exception_env_stack_top = prev; }
	void raise(int exception);
};

#define	EXCEPTION_CODE	exception_environment.code()

#define BEGINX {							\
	ExceptionEnv exception_environment;				\
	if (EXCEPTION_CODE == 0) {					\
		
// Statements in the scope of this exception handler block go here.

#define EXCEPTION							\
	}								\
	else switch(EXCEPTION_CODE) {					\

/*
Exception handlers go here; the syntax is that of a switch statement
body.  The exception code that caused this EXCEPTION block to be entered
may be accessed via the macro EXCEPTION_CODE.  The statement
"default:RAISE(EXCEPTION_CODE);" will propagate the current exception up
to the next exception handler block if the exception is not handled by
this block; otherwise, execution continues with the first statement
after this exception block.
*/

#define ENDX								\
	};								\
}									\

inline void RAISE(int exception)
{
	exception_env_stack_top->raise(exception);
}

#endif

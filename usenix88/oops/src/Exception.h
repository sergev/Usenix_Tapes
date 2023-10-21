#ifndef	EXCEPTION_H
#define	EXCEPTION_H

/* Exception.h -- declarations for OOPS exception handling

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Computer Systems Laboratory, DCRT
	National Institutes of Health
	Bethesda, MD 20892

$Log:	Exception.h,v $
 * Revision 1.2  88/01/16  23:38:56  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"
#include <setjmp.h>

typedef void (*exceptionTrapTy)(unsigned&, int& ...);
enum exceptionActionTy { OOPSERR_ABORT, OOPSERR_RAISE };

class ExceptionAction {
	unsigned error_code;
	exceptionActionTy old_action;
public:
	ExceptionAction(unsigned error);
	virtual ~ExceptionAction();
};

class RaiseException : public ExceptionAction {
public:
	RaiseException(unsigned error);
};

class AbortException : public ExceptionAction {
public:
	AbortException(unsigned error);
};

class ExceptionTrap {
	exceptionTrapTy old_trap;
public:
	ExceptionTrap(exceptionTrapTy xtrap =NULL);
	~ExceptionTrap();
};

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

class Catch {
	Catch*	next;
	Object*	obj;
	friend	ExceptionEnv;
public:
	Catch(Object*);
	Catch();		// for construction of catch_stack_top only!
	~Catch();
};

#endif

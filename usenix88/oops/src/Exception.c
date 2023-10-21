/* Exception.c -- OOPS exception handling

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	February, 1986

Function:

OOPS exception handling.
	
$Log:	Exception.c,v $
 * Revision 1.2  88/01/16  23:38:53  keith
 * Remove pre-RCS modification history
 * 
*/
#include "ExceptAct.h"
#include <osfcn.h>
#include <libc.h>
#include <errlib.h>
#include "oopsconfig.h"
#include "oopserrs.h"
#include "oopserrsx.h"

static char rcsid[] = "$Header: Exception.c,v 1.2 88/01/16 23:38:53 keith Exp $";

ExceptionEnv* exception_env_stack_top;	// top of exception frame stack
ExceptionEnv lastResort;
Catch catch_stack_top;			// top of catch frame stack

void ExceptionEnv::raise(int exception)
{
	if (exception == 0) {
		cerr << "Tried to RAISE exception code 0\n";
		abort();
	}
	if (prev == 0) {	// i.e.,  this == &lastResort
		cerr << "OOPS: fatal: Unhandled exception code " << exception << "\n";
		exit(1);
	}
	pop();
#if STACK_GROWS_DOWN
	for (register Catch* p = catch_stack_top.next; p < (Catch*)this && p != &catch_stack_top; p = p->next)
#else
	for (register Catch* p = catch_stack_top.next; p > (Catch*)this && p != &catch_stack_top; p = p->next)
#endif
		p->obj->destroyer();
	catch_stack_top.next = p;
	longjmp(env,exception);
}

Catch::Catch()
{
	next = this;
	obj = nil;
}

Catch::Catch(Object* ob)
{
	obj = ob;
	void* top[2];
#if STACK_GROWS_DOWN
	if ((void*)this < (void*)exception_env_stack_top && (void*)this > (void*)top) {	// this on stack
#else
	if ((void*)this > (void*)exception_env_stack_top && (void*)this < (void*)top) {	// this on stack
#endif
		next = catch_stack_top.next;
		catch_stack_top.next = this;
	}
	else next = 0;		// Catch object was allocated by new
}

Catch::~Catch()
{
	if (next != 0) catch_stack_top.next = next;
	obj->destroyer();
}

static ExceptionActionTbl mainExceptionActionTbl;
ExceptionActionTbl* oops_exception_action = &mainExceptionActionTbl;

void setOOPSerror(int error, int sev ...)
{
	oops_exception_action->act((unsigned)error,sev);
	register* ap = (int*)&sev;
	seterror(error,sev,ap[1],ap[2],ap[3],ap[4],ap[5],ap[6],ap[7],ap[8]);
	cerr << "OOPS: fatal: Tried to continue after error " << error << "\n";
	abort();
}

ExceptionActionTbl::ExceptionActionTbl()
{
	client_exception_trap = NULL;
	for (register int i=OOPS__LAST_ERROR-OOPS__FIRSTERROR; i>=0; i--)
		action[i] = OOPSERR_ABORT;
}

void ExceptionActionTbl::act(unsigned& error, int& sev)
{
	if (client_exception_trap != NULL) {
		(*client_exception_trap)(error,sev);
	}
	if (error >= OOPS__FIRSTERROR && error <= OOPS__LAST_ERROR) {
		switch (action[error-OOPS__FIRSTERROR]) {
			case OOPSERR_RAISE: RAISE(error);
			case OOPSERR_ABORT: return;
		}
	}
}

ExceptionAction::ExceptionAction(unsigned error)
{
	if ( (error < OOPS__FIRSTERROR)||(error > OOPS__LAST_ERROR) ) {
		setOOPSerror(OOPS_BADERRNUM,DEFAULT,"ExceptionAction",error);
		return;
	}
	error_code = error;
	old_action = oops_exception_action->action[error-OOPS__FIRSTERROR];
}

ExceptionAction::~ExceptionAction()
{
	oops_exception_action->action[error_code-OOPS__FIRSTERROR] = old_action;
}

AbortException::AbortException(unsigned error) : (error)
{
	oops_exception_action->action[error-OOPS__FIRSTERROR] = OOPSERR_ABORT;
}

RaiseException::RaiseException(unsigned error) : (error)
{
	oops_exception_action->action[error-OOPS__FIRSTERROR] = OOPSERR_RAISE;
}

ExceptionTrap::ExceptionTrap(exceptionTrapTy xtrap)
{
	old_trap = oops_exception_action->client_exception_trap;
	oops_exception_action->client_exception_trap = xtrap;
}

ExceptionTrap::~ExceptionTrap()
{
	oops_exception_action->client_exception_trap = old_trap;
}

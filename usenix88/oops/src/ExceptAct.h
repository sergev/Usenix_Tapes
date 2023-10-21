#ifndef	EXCEPTACT_H
#define	EXCEPTACT_H

/* ExceptAct.h -- declarations for exception actions

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

$Log:	ExceptAct.h,v $
 * Revision 1.2  88/01/16  23:38:51  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Exception.h"
#include "oopserrs.h"

class ExceptionActionTbl {
	exceptionTrapTy client_exception_trap;
	char action[OOPS__LAST_ERROR-OOPS__FIRSTERROR+1];
	friend ExceptionAction;
	friend AbortException;
	friend RaiseException;
	friend ExceptionTrap;
public:
	ExceptionActionTbl();
	void act(unsigned& error, int& sev);
};

extern ExceptionActionTbl* oops_exception_action;

#endif

/* Process.c -- implementation of processes

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
	December, 1985

Function:

Class Process serves as a base class for user processes (co-routines).

$Log:	Process.c,v $
 * Revision 1.3  88/02/04  12:52:06  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.2  88/01/16  23:40:32  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Process.h"
#include "Scheduler.h"
#include "ExceptAct.h"

#define	THIS	Process
#define	BASE	Link
DEFINE_CLASS(Process,Link,1,"$Header: Process.c,v 1.3 88/02/04 12:52:06 keith Exp $",NULL,NULL);

extern const int OOPS_STACKOV,OOPS_BADPRI,OOPS_RESRUN,OOPS_RESTERM,OOPS_INVALIDPS,
	OOPS_SUSTERM,OOPS_ILLEGALFCN,OOPS_NOTINIT;

Process::Process(int priority) // MAIN process constructor 
{
	saved_fp = 0;
	stack_base = 0;
	stack_end = 0;
	stack_size = 0;
	process_name = "MAIN";
	process_priority = priority;
	process_state = SUSPENDED;

#ifdef HAVE_SELECT
	rdmask = 0; wrmask = 0; exmask = 0;
#endif
}

Process::~Process()
{
	terminate();
#if STACK_GROWS_DOWN
	delete stack_end;
#else
	delete stack_base;
#endif
}

void Process::checkStack()
{
	if (stack_base == 0) return;	// don't check MAIN stack 
	if (*(stack_end) != (void*)UNINITIALIZED ||
#if STACK_GROWS_DOWN
		saved_fp < stack_end)
#else
		saved_fp > stack_end)
#endif
		setOOPSerror(OOPS_STACKOV,DEFAULT,name());
}

unsigned char Process::priority()	{ return process_priority; }

unsigned char Process::priority(unsigned char newPriority)
{
	register unsigned char oldPriority =process_priority;
	if (newPriority > MAXPRIORITY) setOOPSerror(OOPS_BADPRI,DEFAULT,newPriority,MAXPRIORITY);
	AST_DISABLE;
	process_priority = newPriority;
	if (newPriority != oldPriority && process_state == RUNNING) {
		scheduler.runList[oldPriority].remove(*this);
		scheduler.runList[newPriority].addLast(*this);
	}
	AST_ENABLE;
	return oldPriority;
}

unsigned Process::capacity()		{ return stack_size; }

void Process::resume()
{
	AST_DISABLE;
	switch (process_state) {
		case SUSPENDED: {
			process_state = RUNNING;
			scheduler.runList[process_priority].addLast(*this);
			scheduler.runCount++;
			if (AST_ACTIVE) {		// force any pending select to time out
				scheduler.selectTimeout.tv_sec = 0;
				scheduler.selectTimeout.tv_usec = 0; 
			}
			else schedule();
			break;
		}
		case RUNNING: setOOPSerror(OOPS_RESRUN,DEFAULT,name(),this);
		case TERMINATED: setOOPSerror(OOPS_RESTERM,DEFAULT,name(),this);
		default: setOOPSerror(OOPS_INVALIDPS,DEFAULT,name(),this,className(),process_state);
	}
	AST_ENABLE;
}

void Process::suspend()
{
	AST_DISABLE;
	switch (process_state) {
		case SUSPENDED: break;
		case RUNNING: {
			process_state = SUSPENDED;
			scheduler.runList[process_priority].remove(*this);
			scheduler.runCount--;
			break;
		}
		case TERMINATED: setOOPSerror(OOPS_SUSTERM,DEFAULT,name(),this);
		default: setOOPSerror(OOPS_INVALIDPS,DEFAULT,name(),this,className(),process_state);
	}
	AST_ENABLE;
}

void Process::terminate()
{
	AST_DISABLE;
		suspend();
		process_state = TERMINATED;
	AST_ENABLE;
	schedule();
}

#ifdef HAVE_SELECT
void Process::select(unsigned& readfds, unsigned& writefds, unsigned& exceptfds)
{
	rdmask = readfds;	// save fd masks
	wrmask = writefds;
	exmask = exceptfds;
	AST_DISABLE;
	suspend();
	scheduler.selectList.addLast(*this);
	AST_ENABLE;
	schedule();
	readfds = rdmask;	// return results of select
	writefds = wrmask;
	exceptfds = exmask;
}
#endif

int Process::compare(const Object& ob)	// compare process priorities 
{
	assertArgSpecies(ob,class_Process,"compare");
	return process_priority - ((Process*)&ob)->process_priority;
}

Object* Process::copy()
{
	shouldNotImplement("copy");
	return 0;
}

void Process::deepenShallowCopy()
{
	shouldNotImplement("deepCopy");
}

bool Process::forkCheck()
{
	if (!OOPSInitialized()) setOOPSerror(OOPS_NOTINIT,DEFAULT,"Scheduler",this,className(),"forkCheck");
	return scheduler.active_process != this;
}

unsigned Process::hash()	{ return (unsigned)this; }

void Process::init()	// initialize process context
{
	process_state = SUSPENDED;
	saved_AST_state = 0;
#ifdef HAVE_SELECT
	rdmask = wrmask = exmask = 0;	// masks for select(2)
#endif
	extern ExceptionEnv lastResort;
	saved_exception_env_stack_top = &lastResort;
	saved_exception_action = new ExceptionActionTbl;
}

bool Process::isEqual(const Object& ob)	{ return isSame(ob); }
	
void Process::printOn(ostream& strm)
{
	strm	<< className() << " " << process_name
		<< "  pri: " << process_priority
		<< "  state: ";
	switch (process_state) {
		case SUSPENDED: strm << "SUSPENDED"; break;
		case RUNNING:	strm << "RUNNING"; break;
		case TERMINATED: strm << "TERMINATED"; break;
		default: strm << "INVALID";
	}
	strm 	<< "  stack: 0x" << hex((int)stack_base)
		<< "  size: " << stack_size
		<< "  fp: 0x" << hex((int)saved_fp)

#ifdef HAVE_SELECT
		<< "  rd: 0x" << hex(rdmask)
		<< "  wr: 0x" << hex(wrmask)
		<< "  ex: 0x" << hex(exmask)
#endif

		<< "\n";
}

unsigned Process::size()		{ return stack_size; }

extern Catch catch_stack_top;

void Process::restore()
{
	exception_env_stack_top = saved_exception_env_stack_top;
	oops_exception_action = saved_exception_action;
	catch_stack_top = saved_catch_stack_top;
}

void Process::save()
{
	saved_exception_env_stack_top = exception_env_stack_top;
	saved_exception_action = oops_exception_action;
	saved_catch_stack_top = catch_stack_top;
}

void Process::storer(ostream&)
{
	shouldNotImplement("storeOn");
}

void Process::storer(fileDescTy&) 
{
	shouldNotImplement("storeOn");
}

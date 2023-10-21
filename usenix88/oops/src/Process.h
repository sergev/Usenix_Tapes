#ifndef	PROCESS_H
#define	PROCESS_H

/* Process.h -- declarations for processes

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

$Log:	Process.h,v $
 * Revision 1.3  88/02/04  12:52:10  keith
 * Make Class class_CLASSNAME const.
 * Make destructor non-inline.
 * 
 * Revision 1.2  88/01/16  23:40:35  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Link.h"
#include "Exception.h"
#include "String.h"
#include "oopsconfig.h"

const int MAXPRIORITY = 7;	// maximum process priority

#define	PROCESS_FORK	if (forkCheck()) { resume(); return; }

class Scheduler;
class Semaphore;
class ExceptionActionTbl;

extern const Class class_Process;
overload Process_reader;

enum processState { SUSPENDED, RUNNING, TERMINATED };

class Process : public Link {
	void* saved_fp;			// saved frame pointer -- ACCESSED BY MACHINE-DEPENDENT CODE! 
	void** stack_base;		// bottom of stack 
	void** stack_end;		// end of stack (stack_end < stack_base) if stack grows down 
	unsigned stack_size;		// stack size in void* 
	String process_name;
	processState process_state;	// SUSPENDED, RUNNING, or TERMINATED 
	unsigned char process_priority;
	int saved_AST_state;		// AST priority or signal mask (saved/restored by Scheduler)
	ExceptionEnv* saved_exception_env_stack_top;
	ExceptionActionTbl* saved_exception_action;
	Catch saved_catch_stack_top;

	void checkStack();
	void create(void** stack);	// machine dependent 
	void exchj();			// machine dependent 
	friend Scheduler;
	friend Semaphore;
	friend void schedule();
	friend void yield();
	friend bool selectfd();
	friend void init_main_process(const Class&);
	Process(int priority);		// MAIN process constructor 
protected:
#ifdef HAVE_SELECT
	unsigned rdmask, wrmask, exmask;  // masks for select(2)
#endif

	Process(const char* name ="", int priority =0, unsigned stacksize =1024);
	Process(fileDescTy&,Process&) {}
	Process(istream&,Process&) {}
	friend	void Process_reader(istream& strm, Object& where);
	friend	void Process_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	~Process();
	bool forkCheck();
	const char* name()		{ return process_name; }
	processState state()		{ return process_state; }

#ifdef HAVE_SELECT
	virtual void select(unsigned&, unsigned&, unsigned&);// add to select list
#endif

	virtual unsigned capacity();		// returns stack size 
	virtual int compare(const Object&);	// compare process priorities 
	virtual Object* copy();
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual void init();
	virtual const Class* isA();
	virtual bool isEqual(const Object& ob);
	virtual void printOn(ostream& strm);
	virtual unsigned char priority();
	virtual unsigned char priority(unsigned char newPriority);
	virtual void restore();
	virtual void resume();
	virtual void save();
	virtual unsigned size();
	virtual void suspend();
	virtual void terminate();
};

inline Process::Process(const char* name, int priority, unsigned stacksize)
	: process_name(name)
{
	process_priority = priority;
	stack_size = stacksize;
#if STACK_GROWS_DOWN
	stack_end = new void*[stacksize];
	stack_base = &stack_end[stacksize-1];
#else
	stack_base = new void*[stacksize];
	stack_end = &stack_base[stacksize-1];
#endif
	*stack_end = (void*)UNINITIALIZED;	// to detect stack overflow 
	init();					// initialize process context
	create(stack_base);			// initialize the new stack 
// The constructor for the derived class must PROCESS_FORK here! 
}

#endif

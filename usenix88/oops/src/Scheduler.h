#ifndef	SCHEDULER_H
#define	SCHEDULER_H

/* Scheduler.h -- declarations for the Process Scheduler

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

$Log:	Scheduler.h,v $
 * Revision 1.3  88/02/04  13:00:07  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:01  keith
 * Remove pre-RCS modification history
 * 
*/
#include "LinkedList.h"
#ifdef HAVE_SELECT
#include <sys/time.h>
#endif

extern const Class class_Scheduler;
overload Scheduler_reader;

class Process;

class Scheduler : public Object {
	Process* active_process;
	Process* previous_process;	// ACCESSED BY MACHINE-DEPENDENT CODE! 
	unsigned runCount;		// total # of runnable processes 
	LinkedList runList[MAXPRIORITY+1];
#ifdef HAVE_SELECT
	LinkedList selectList;		// Process waiting for select
	struct timeval selectTimeout;	// timeout on select
	int selectfd();			// do select(2) system call
#endif
	friend Process;
	friend void init_main_process(const Class&);
protected:
	Scheduler(fileDescTy&,Scheduler&) {}
	Scheduler(istream&,Scheduler&) {}
	friend	void Scheduler_reader(istream& strm, Object& where);
	friend	void Scheduler_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Scheduler();
	virtual unsigned capacity();
	virtual Object* copy();			// shouldNotImplement
	virtual void deepenShallowCopy();	// shouldNotImplement
	virtual const Class* isA();
	virtual void printOn(ostream& strm);
	virtual Object* shallowCopy();		// shouldNotImplement
	virtual unsigned size();
	
	friend unsigned char activePriority();
	friend Process& activeProcess();
	friend void schedule();
	friend void terminateActive();
	friend void yield();
#ifdef HAVE_SELECT
	void timeOutSelect()	{ selectTimeout.tv_sec = selectTimeout.tv_usec = 0; }
#endif
};

extern Scheduler scheduler;

inline unsigned char activePriority()	{ return scheduler.active_process->priority(); }
inline Process& activeProcess()		{ return *scheduler.active_process; }
inline void terminateActive()		{ activeProcess().terminate(); }

#endif

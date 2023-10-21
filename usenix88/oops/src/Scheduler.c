/* Scheduler.c -- implementation of the Process Scheduler

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

Class Scheduler provides an interface to the single instance of the
process scheduler, scheduler.

$Log:	Scheduler.c,v $
 * Revision 1.2  88/01/16  23:40:58  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Process.h"
#include "Scheduler.h"
#include <osfcn.h>

#ifdef HAVE_SELECT
#include <sys/errno.h>
#endif

#define	THIS	Scheduler
#define	BASE	Object
DEFINE_CLASS(Scheduler,Object,1,"$Header: Scheduler.c,v 1.2 88/01/16 23:40:58 keith Exp $",NULL,init_main_process);

extern const int OOPS_SCHEDCTOR,OOPS_ILLEGALFCN;

Scheduler scheduler; 	// the sole instance of the Scheduler object 
int ast_level =0;	// AST nesting level 

static void init_main_process(const Class&)
{
	scheduler.active_process = new Process(0);	// create MAIN process 
	scheduler.active_process->resume();		// run MAIN process 
}

Scheduler::Scheduler()
{
	if (this != &scheduler) setOOPSerror(OOPS_SCHEDCTOR,DEFAULT,this,className());
	previous_process = 0;
	runCount = 0;
}

void schedule()
{
	scheduler.active_process->checkStack();
	if (AST_ACTIVE) return;

	while (YES) {
		AST_DISABLE;
		while (scheduler.runCount>0) {
			for (register int i=MAXPRIORITY; i>=0; i--) {
				if (scheduler.runList[i].size() > 0) {
					Process* next_process = (Process*)scheduler.runList[i].first();
					if (next_process != scheduler.active_process) {
						AST_SAVE(scheduler.active_process->saved_AST_state);
						scheduler.active_process->save();
						scheduler.previous_process = scheduler.active_process;
						scheduler.active_process = next_process;
						next_process->exchj();
						scheduler.active_process->restore();
						AST_RESTORE(scheduler.active_process->saved_AST_state);
						scheduler.previous_process->checkStack();
					}
					else AST_ENABLE;
					return;
				}
			}
		}
#ifdef HAVE_SELECT
		if (scheduler.selectList.isEmpty())
#endif
			AST_PAUSE;		// wait for AST/signal
#ifdef HAVE_SELECT
		else scheduler.selectfd();	// wait for AST, signal, or I/O
#endif
	}
}

#ifdef HAVE_SELECT
int Scheduler::selectfd()
{
	int mcount =0;			// return value from select(2)
        unsigned rmask =0, wmask =0, xmask =0;
	selectTimeout.tv_sec = 10;
	selectTimeout.tv_usec = 0;
	
	if (selectList.isEmpty() ) return 0;
	register Process* p = (Process*)selectList.first();
	
	while (p != (Process*)nil) {   // build select masks
		rmask |= p->rdmask;
		wmask |= p->wrmask;
		xmask |= p->exmask;
		p = (Process*)p->nextLink();
	};

	AST_RESTORE(0);
/*
If a signal/AST occurs at this point that makes a process runnable (by
calling Process::resume()), it will clear selectTimeout and the
following select will return immediately.  There may still be a window
in which the runnable process will not be scheduled until after this
select returns.  Thus, the select must be done with a timeout.  A
better approach is to use asynchronous I/O instead of select, but this
is said to have bugs under 4.2 BSD.
*/
	mcount = ::select(getdtablesize(),
		(rmask)? &rmask:0,(wmask)? &wmask:0,(xmask)? &xmask:0,&selectTimeout);
	AST_DISABLE;

	if (mcount < 0) { // error
		if (errno == EINTR) return mcount;
		cerr << form("Scheduler: error in select (%d)\n",errno);
		exit(1);
		};

	if (mcount == 0) return 0; // timeout or no fd selected

	p = (Process*)selectList.first();
	while ( p != (Process*)nil ) {   // resume all selected processes
		if ( rmask&p->rdmask||wmask&p->wrmask||xmask&p->exmask ) {
			p->rdmask &= rmask;
			p->wrmask &= wmask;
			p->exmask &= xmask;
			selectList.remove(*p);
			p->process_state = RUNNING;
			runList[p->process_priority].addLast(*p);
			runCount++;
		};
		p = (Process*)p->nextLink();
	};
	return mcount;
}
#endif

void yield()
{
	if (scheduler.runList[activePriority()].size() == 1) schedule();
	else {
		activeProcess().suspend();
		activeProcess().resume();
	}
}

unsigned Scheduler::capacity()	{ return runCount; }

Object* Scheduler::copy()
{
	shouldNotImplement("copy");
	return 0;
}

Object* Scheduler::shallowCopy()
{
	shouldNotImplement("shallowCopy");
	return 0;
}

void Scheduler::deepenShallowCopy()
{
	shouldNotImplement("deepCopy");
}

unsigned Scheduler::size()	{ return runCount; }
	
void Scheduler::printOn(ostream& strm)
{
	AST_DISABLE;
	strm << className()
		<< "  active process: " << active_process->name()
		<< "  previous process: " << previous_process->name() << "\n";
	strm << "runList:\n";
	for (register i =MAXPRIORITY; i>=0; i--) {
		DO(runList[i],Process*,p) strm << "\t" << *p; OD
	}

#ifdef HAVE_SELECT
	if (!selectList.isEmpty()) {
		strm << "selectList:\n";
		DO(selectList,Process*,p) strm << "\t" << *p; OD
	}
#endif

	AST_ENABLE;
}

void Scheduler::storer(ostream&)
{
	shouldNotImplement("storeOn");
}

void Scheduler::storer(fileDescTy&) 
{
	shouldNotImplement("storeOn");
}

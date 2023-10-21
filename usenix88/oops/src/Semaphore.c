/* Semaphore.c -- implementation of Semaphore

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

Objects of class Semaphore are used to synchronize Processes.

$Log:	Semaphore.c,v $
 * Revision 1.3  88/01/27  15:44:31  keith
 * Correct problem in Semaphore::signal() introduced by previous
 * enhancement that added parameter.
 * 
 * 
 * Revision 1.2  88/01/16  23:41:03  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Semaphore.h"
#include "Process.h"
#include "Scheduler.h"
#include "oopsIO.h"

#define	THIS	Semaphore
#define	BASE	Object
DEFINE_CLASS(Semaphore,Object,1,"$Header: Semaphore.c,v 1.3 88/01/27 15:44:31 keith Exp $",NULL,NULL);

extern const int OOPS_ASTBLK,OOPS_DLSEMWAIT,OOPS_CPSEMWAIT,OOPS_STSEMWAIT;

void Semaphore::signal(unsigned n)
{
	if (n == 0) return;
	AST_DISABLE;
		register Process* next;
		while (count<0 && n>0) {
			next = (Process*)waitList.removeFirst();
			if (next->state() != TERMINATED) {
				next->resume();
				n--;
			}
			count++;
		}
		count += n;
	AST_ENABLE;
}

void Semaphore::wait()
{
	AST_DISABLE;
		if (--count < 0) {
			if (AST_ACTIVE) setOOPSerror(OOPS_ASTBLK,DEFAULT,this,className(),"wait");
			activeProcess().suspend();
			waitList.addLast(activeProcess());
		}
	AST_ENABLE;
	if (count < 0) schedule();
}

Semaphore::~Semaphore()
{
	if (count<0) setOOPSerror(OOPS_DLSEMWAIT,DEFAULT,-count,this,className());
}

void Semaphore::deepenShallowCopy()
{
	if (count < 0) setOOPSerror(OOPS_CPSEMWAIT,DEFAULT,-count,this,className());
	BASE::deepenShallowCopy();
}

unsigned Semaphore::hash()	{ return (unsigned)this; }

bool Semaphore::isEqual(const Object& ob)	{ return isSame(ob); }

void Semaphore::printOn(ostream& strm)
{
	strm << className() << "  count: " << count << "\n";
	DO(waitList,Process*,p) strm << "\t" << *p; OD
}

Object* Semaphore::shallowCopy()	{ shouldNotImplement("shallowCopy"); return 0; }

int Semaphore::value()		{ return count; }

Semaphore::Semaphore(istream& strm, Semaphore& where)
{
	this = &where;
	strm >> count;
}

void Semaphore::storer(ostream& strm)
{
	if (count < 0) setOOPSerror(OOPS_STSEMWAIT,DEFAULT,-count,this,className());
	BASE::storer(strm);
	strm << count << " ";
}

Semaphore::Semaphore(fileDescTy& fd, Semaphore& where)
{
	this = &where;
	readBin(fd,count);
}

void Semaphore::storer(fileDescTy& fd) 
{
	if ( count < 0 )
		setOOPSerror(OOPS_STSEMWAIT,DEFAULT,-count,this,className());
	BASE::storer(fd);
	storeBin(fd,count);
}

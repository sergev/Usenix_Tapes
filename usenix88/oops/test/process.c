/* Test class Process

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

Function:
	
Modification History:
	
$Log:	process.c,v $
 * Revision 1.1  88/01/17  22:24:57  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: process.c,v 1.1 88/01/17 22:24:57 keith Exp $";

#include "Process.h"
#include "SharedQueue.h"
#include "Scheduler.h"
#include "String.h"

class TestProcess : public Process {
public:
	TestProcess(const char* name, int pri, SharedQueue& in, SharedQueue& out);
};

TestProcess::TestProcess(const char* pname, int pri, SharedQueue& in, SharedQueue& out) : (pname,pri)
{
	PROCESS_FORK;
	while (YES) {
		Object* msg = in.next();
		cout << "process " << name() << " received " << *msg << "\n"; cout.flush();
		out.nextPut(*msg);
		cout << "process " << name() << " sent " << *msg << "\n"; cout.flush();
	}
}

main()
{
	SharedQueue* q0 = new SharedQueue(2);
	SharedQueue* qin = q0;
	SharedQueue* qout;
	for (register int i=1; i<=MAXPRIORITY; i++) {
		String pname = "P";
		pname &= (char)('0'+i);
		qout = new SharedQueue(2);
		new TestProcess(pname,i,*qin,*qout);
		qin = qout;
	}
	String& msg = *new String("THE MESSAGE");
	cerr << scheduler;
	cerr << *q0;
	yield();
	q0->nextPut(msg);
	cout << *qout->next() << "\n"; cout.flush();
}

/* SharedQueue.c -- implementation of shared queues

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

Class SharedQueue provides a queue that can be used for communication
between two separate processes.  Semaphores are used to block a process
if next() is called and there are no objects in the queue, or if
nextPut() is called and there is no space in the queue.

$Log:	SharedQueue.c,v $
 * Revision 1.3  88/01/27  16:18:51  keith
 * Fix SharedQueue::printOn() so it prints full queues.
 * 
 * Revision 1.2  88/01/16  23:41:17  keith
 * Remove pre-RCS modification history
 * 
*/

#include "SharedQueue.h"
#include "oopsIO.h"

#define	THIS	SharedQueue
#define	BASE	Object
DEFINE_CLASS(SharedQueue,Object,1,"$Header: SharedQueue.c,v 1.3 88/01/27 16:18:51 keith Exp $",NULL,NULL);

SharedQueue::SharedQueue(unsigned queueSize) :
	queue(queueSize), valueAvailable(0), spaceAvailable(queueSize)
{
	readPosition = writePosition = 0;
}

Object* SharedQueue::next()
{
	valueAvailable.wait();
	Object* ob = queue[readPosition];
	queue[readPosition++] = nil;
	if (readPosition >= queue.capacity()) readPosition = 0;
	spaceAvailable.signal();
	return ob;
}

Object* SharedQueue::nextPut(const Object& ob)
{
	spaceAvailable.wait();
	queue[writePosition++] = &ob;
	if (writePosition >= queue.capacity()) writePosition = 0;
	valueAvailable.signal();
	return &ob;
}

unsigned SharedQueue::capacity()	{ return queue.capacity(); }

bool SharedQueue::isEmpty()	{ return valueAvailable.value() == 0; }

unsigned SharedQueue::size()		{ return valueAvailable.value(); }

void SharedQueue::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	queue.deepenShallowCopy();
	valueAvailable.deepenShallowCopy();
	spaceAvailable.deepenShallowCopy();
}

void SharedQueue::printOn(ostream& strm)
{
	strm << className() << "(\n";
	strm << "valueAvailable " << valueAvailable;
	strm << "spaceAvailable " << spaceAvailable;
	strm << "queue:\n";
	int i = readPosition;
	for (int n = valueAvailable.value(); n>0; n--) {
		queue[i++]->printOn(strm);
		strm << "\n";
		if (i == queue.capacity()) i = 0;
	}
	strm << ")\n";
}

Object* SharedQueue::shallowCopy()	{ shouldNotImplement("shallowCopy"); return 0; }

static unsigned sharedqueue_capacity;

SharedQueue::SharedQueue(istream& strm, SharedQueue& where) : queue((strm >> sharedqueue_capacity, sharedqueue_capacity))
{
	this = &where;
	readFrom(strm,"Semaphore",valueAvailable);
	readFrom(strm,"Semaphore",spaceAvailable);
	readPosition = 0;
	writePosition = valueAvailable.value();
	for (register int i =0; i<writePosition; i++) queue[i] = readFrom(strm);
}

void SharedQueue::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << queue.capacity() << " ";
	valueAvailable.storeOn(strm);
	spaceAvailable.storeOn(strm);
	register int i = readPosition;
	while (i != writePosition) {
		queue[i++]->storeOn(strm);
		if (i == queue.capacity()) i = 0;
	}
}

SharedQueue::SharedQueue(fileDescTy& fd, SharedQueue& where) : queue((readBin(fd,sharedqueue_capacity), sharedqueue_capacity))
{
	this = &where;
	readFrom(fd,"Semaphore",valueAvailable);
	readFrom(fd,"Semaphore",spaceAvailable);
	readPosition = 0;
	writePosition = valueAvailable.value();
	for (register int i=0; i < writePosition; i++)
	  queue[i] = readFrom(fd);
}

void SharedQueue::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	storeBin(fd,queue.capacity());
	valueAvailable.storeOn(fd);
	spaceAvailable.storeOn(fd);
	register int i = readPosition;
	while (i != writePosition) {
	  queue[i++]->storeOn(fd);
	  if (i == queue.capacity()) i = 0;
	  };
}

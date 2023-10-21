#ifndef	SHAREDQUEUE_H
#define	SHAREDQUEUE_H

/* SharedQueue.h -- declarations for shared queues

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

$Log:	SharedQueue.h,v $
 * Revision 1.3  88/02/04  13:00:18  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:19  keith
 * Remove pre-RCS modification history
 * 
*/

#include "ArrayOb.h"
#include "Semaphore.h"

extern const Class class_SharedQueue;
overload SharedQueue_reader;

class SharedQueue : public Object {
	ArrayOb queue;
	int readPosition,writePosition;
	Semaphore valueAvailable;
	Semaphore spaceAvailable;
protected:
	SharedQueue(fileDescTy&,SharedQueue&);
	SharedQueue(istream&,SharedQueue&);
	friend	void SharedQueue_reader(istream& strm, Object& where);
	friend	void SharedQueue_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	SharedQueue(unsigned queueSize =CLTN_DEFAULT_CAPACITY);
	bool isFull()	{ return spaceAvailable.value() == 0; }
	virtual unsigned capacity();
	virtual void deepenShallowCopy();
	virtual const Class* isA();
	virtual bool isEmpty();
	virtual Object* next();
	virtual Object* nextPut(const Object&);
	virtual void printOn(ostream& strm);
	virtual Object* shallowCopy();		// shouldNotImplement
	virtual unsigned size();
};

#endif

#ifndef	SEMAPHORE_H
#define	SEMAPHORE_H

/* Semaphore.h -- declarations for Semaphore

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

$Log:	Semaphore.h,v $
 * Revision 1.3  88/02/04  13:00:09  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:06  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"
#include "LinkedList.h"

extern const Class class_Semaphore;
overload Semaphore_reader;

class Semaphore: public Object {
	LinkedList waitList;
	short count;
protected:
	Semaphore(fileDescTy&,Semaphore&);
	Semaphore(istream&,Semaphore&);
	friend	void Semaphore_reader(istream& strm, Object& where);
	friend	void Semaphore_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Semaphore(int initialCount =0)	{ count = initialCount; }
	~Semaphore();
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object& ob);
	virtual void printOn(ostream& strm);
	virtual Object* shallowCopy();	// shouldNotImplement
	virtual void signal(unsigned n=1);  // signal waiting process 
	virtual int value();		// read semaphore value 
	virtual void wait();		// wait on semaphore 
};

#endif

#ifndef	SEQCLTN_H
#define	SEQCLTN_H

/* SeqCltn.h -- declarations for abstract sequential collections

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

$Log:	SeqCltn.h,v $
 * Revision 1.3  88/02/04  13:00:12  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:41:10  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Collection.h"

extern const Class class_SeqCltn;
overload SeqCltn_reader;

class SeqCltn: public Collection {
protected:
	SeqCltn() {}
	SeqCltn(fileDescTy&,SeqCltn&) {}
	SeqCltn(istream&,SeqCltn&) {}
	friend	void SeqCltn_reader(istream& strm, Object& where);
	friend	void SeqCltn_reader(fileDescTy& fd, Object& where);
	void indexRangeErr();
public:
	virtual void atAllPut(const Object& ob);
	virtual void deepenShallowCopy();
	virtual Object* doNext(Iterator&);
	virtual Object* first();
 	virtual const Class* isA();
	virtual int indexOf(const Object& ob);
	virtual int indexOfSubCollection(const SeqCltn& cltn, int start=0);
	virtual Object* last();
	virtual void replaceFrom(int start, int stop, const SeqCltn& replacement, int startAt =0);
};

#endif

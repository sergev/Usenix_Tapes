#ifndef	LINKOB_H
#define	LINKOB_H

/* LinkOb.h -- declarations for singly-linked list Object element

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

$Log:	LinkOb.h,v $
 * Revision 1.3  88/02/04  12:59:31  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:33  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Link.h"

extern const Class class_LinkOb;
overload LinkOb_reader;

class LinkOb: public Link {
	Object* val;
protected:
	LinkOb(fileDescTy&,LinkOb&);
	LinkOb(istream&,LinkOb&);
	friend	void LinkOb_reader(istream& strm, Object& where);
	friend	void LinkOb_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	LinkOb(const Object& newval =*nil);
	virtual unsigned capacity();
	virtual int compare(const Object&);
	virtual void deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual unsigned size();
	virtual Object* value();
	virtual Object* value(const Object& newval);
};

#endif

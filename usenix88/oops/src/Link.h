#ifndef	LINK_H
#define	LINK_H

/* Link.h -- declarations for singly-linked list element

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

$Log:	Link.h,v $
 * Revision 1.3  88/02/04  12:59:28  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:29  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Link;
overload Link_reader;

class LinkedList;

class Link: public Object {
	Link* next;		// pointer to next Link or nil 
	friend LinkedList;
protected:
	Link(const Link& nextlink)	{ next = &nextlink; }
	Link()				{ next = (Link*)nil; }
	Link(fileDescTy&,Link&) {}
	Link(istream&,Link&) {}
	friend	void Link_reader(istream& strm, Object& where);
	friend	void Link_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	~Link();
	Link* nextLink()		{ return next; }
	Link* nextLink(Link* nextlink)	{ next = nextlink; return next; }
	virtual void deepenShallowCopy();
	virtual const Class* isA();
	virtual Object* shallowCopy();		// shouldNotImplement
};

#endif

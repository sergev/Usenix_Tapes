/* LinkedList.c -- implementation of singly-linked list

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
	October, 1985

Function:
	
LinkedLists are ordered by the sequence in which objects are added and removed
from them.  Object elements are accessible by index.

$Log:	LinkedList.c,v $
 * Revision 1.2  88/01/16  23:39:35  keith
 * Remove pre-RCS modification history
 * 
*/

#include "LinkedList.h"
#include "oopsIO.h"

#define	THIS	LinkedList
#define	BASE	SeqCltn
DEFINE_CLASS(LinkedList,SeqCltn,1,"$Header: LinkedList.c,v 1.2 88/01/16 23:39:35 keith Exp $",NULL,NULL);

extern const int OOPS_DBLLNK,OOPS_CLTNEMPTY,OOPS_OBNOTFOUND,OOPS_INDEXRANGE;

LinkedList::LinkedList()
{
	firstLink = lastLink = (Link*)nil;
	count = 0;
}

LinkedList::LinkedList(istream& strm, LinkedList& where)
{
	this = &where;
	firstLink = lastLink = (Link*)nil;
	count = 0;
	unsigned n;
	strm >> n;
	while (n--) add(*readFrom(strm));
}

bool LinkedList::operator==(const LinkedList& ll)
{
	if (count != ll.count) return NO;
	else {
		register Link* p = firstLink;
		register Link* q = ll.firstLink;
		while (p != (Link*)nil) {
			if (!(p->isEqual(*q))) return NO;
			p = p->nextLink();
			q = q->nextLink();
		}
	}
	return YES;
}

Object* LinkedList::add(const Object& ob)
{
	assertArgClass(ob,class_Link,"add");
	register Link* lnk = (Link*)&ob;
	if (lnk->nextLink() != (Link*)nil) errDblLnk("add",*lnk);
	if (count == 0) firstLink = lnk;
	else lastLink->nextLink(lnk);
	lastLink = lnk;
	count++;
	return &ob;
}

Collection& LinkedList::addContentsTo(Collection& cltn)
{
	register Link* p = firstLink;
	while (p != (Link*)nil) {
		register Link* t = (Link*)p->copy();
		t->nextLink((Link*)nil);
		cltn.add(*t);
		p = p->nextLink();
	}
	return cltn;
}

Object* LinkedList::addFirst(const Object& ob)
{
	assertArgClass(ob,class_Link,"addFirst");
	if (count == 0) return add(ob);
	else {
		register Link* lnk = (Link*)&ob;
		if (lnk->nextLink() != (Link*)nil) errDblLnk("addFirst",*lnk);
		lnk->nextLink(firstLink);
		firstLink = lnk;
		count++;
		return &ob;
	}
}

Object* LinkedList::addLast(const Object& ob) { return add(ob); }

Object*& LinkedList::operator[](int i)
{
	if ((unsigned)i >= count) indexRangeErr();
	if (i==0) return firstLink;
	else {
		register Link* p = firstLink;
		for (register int j=i-1; j>0; j--) p = p->nextLink();
		return p->next;
	}
}

Object*& LinkedList::at(int i)	{ return (*this)[i]; }

void LinkedList::atAllPut(const Object&) { shouldNotImplement("atAllPut"); }

void LinkedList::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	register Link* p = firstLink;
	firstLink = lastLink = (Link*)nil;
	count = 0;
	while (p != (Link*)nil) {
		add(*(p->deepCopy()));
		p = p->nextLink();
	}
}

Object* LinkedList::first()
{
	if (count==0) errEmpty("first");
	else return firstLink;
}

unsigned LinkedList::hash()
{
	register unsigned h = count;
	register Link* p = firstLink;
	while (p != (Link*)nil) {
		h^= p->hash();
		p = p->nextLink();
	}
	return h;
}

int LinkedList::indexOf(const Object& ob)
{
	register int i = 0;
	register Link* p = firstLink;
	while (p != (Link*)nil) {
		if (p->isEqual(ob)) return i;
		p = p->nextLink();
		i++;
	}
	return -1;
}

int LinkedList::indexOfSubCollection(const SeqCltn& /*cltn*/, int /*start*/)
{	shouldNotImplement("indexOfSubCollection"); return 0;	}

bool LinkedList::isEmpty()	{ return count==0; }

bool LinkedList::isEqual(const Object& a)
{
	return a.isSpecies(class_LinkedList) && *this==*(LinkedList*)&a;
}

const Class* LinkedList::species()	{ return &class_LinkedList; }

Object* LinkedList::last()
{
	if (count==0) errEmpty("last");
	else return lastLink;
}

Object* LinkedList::doNext(Iterator& pos)
{
	if (pos.ptr == 0) {
		if (firstLink == nil) return 0;
		else {
			pos.ptr = firstLink;
			pos.index = 1;
			return firstLink;
		}
	}
else	if ((Link*)pos.ptr == lastLink) return 0;
else		{
		pos.ptr = ((Link*)pos.ptr)->nextLink();
		pos.index++;
		return (Object*)pos.ptr;
	}
}

unsigned LinkedList::occurrencesOf(const Object& ob)
{
	register unsigned n=0;
	register Link* p = firstLink;
	while (p != (Link*)nil) {
		if (p->isEqual(ob)) n++;
		p = p->nextLink();
	}
	return n;
}

void LinkedList::printOn(ostream& strm)
{
	strm << className() << "[\n";
	register unsigned i = 0;
	register Link* p = firstLink;
	while (p != (Link*)nil) {
		if (i>0) strm << "\n";
		p->printOn(strm);
		p = p->nextLink();
		i++;
	}
	strm << "]\n";
}

Object* LinkedList::remove(const Object& ob)
{
	assertArgClass(ob,class_Link,"remove");
	if (count==0) errEmpty("remove");
	register Link* lnk = (Link*)&ob;
	if (lnk == firstLink) {
		firstLink = lnk->nextLink();
		if (lnk == lastLink) lastLink = (Link*)nil;
		goto wrapup;
	}
	else {
		register Link* p = firstLink;
		while (p != (Link*)nil) {
			if (lnk == p->nextLink()) {
				p->nextLink(lnk->nextLink());
				if (lnk == lastLink) lastLink = p;
				goto wrapup;
			}
			p = p->nextLink();
		}
		errNotFound("remove",ob);
	}
wrapup:
	lnk->nextLink((Link*)nil);
	count--;
	return &ob;
}

Object* LinkedList::removeFirst()	{ return remove(*firstLink); }

Object* LinkedList::removeLast()	{ return remove(*lastLink); }
	
void LinkedList::replaceFrom(int /*start*/, int /*stop*/, const SeqCltn& /*replacement*/, int /*startAt*/)
{
	shouldNotImplement("replaceFrom");
}

void LinkedList::reSize(unsigned newSize) {}

unsigned LinkedList::size()	{ return count; }
	
void LinkedList::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,size());
	DO(*this,Object*,ob) ob->storeOn(fd); OD
}

void LinkedList::storer(ostream& strm)
{
	Object::storer(strm);
	strm << size();
	DO(*this,Object*,ob) ob->storeOn(strm); OD
}

void LinkedList::errDblLnk(const char* fn, const Link& lnk)
{
	setOOPSerror(OOPS_DBLLNK,DEFAULT,lnk.className(),className(),this,className(),fn,lnk.className(),&lnk);
}

void LinkedList::errEmpty(const char* fn)
{
	setOOPSerror(OOPS_CLTNEMPTY,DEFAULT,this,className(),fn);
}

void LinkedList::errNotFound(const char* fn, const Object& ob)
{
	setOOPSerror(OOPS_OBNOTFOUND,DEFAULT,this,className(),fn,ob.className(),&ob);
}

LinkedList::LinkedList(fileDescTy& fd, LinkedList& where)
{
	this = &where;
	firstLink = lastLink = (Link*)nil;
	count = 0;
	unsigned n;
	readBin(fd,n);
	while (n--) add(*readFrom(fd));
}

/* Bag.c -- implementation of a Set of Objects with possible duplicates

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
	September, 1985

Function:
	
A Bag is like a Set, except Bags can contain multiple occurrences of
equal objects.  Bags are implemented by using a Dictionary to associate
each object in the Bag with its number of occurrences.

$Log:	Bag.c,v $
 * Revision 1.2  88/01/16  23:37:44  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Bag.h"
#include "AssocInt.h"
#include "Integer.h"
#include "oopsIO.h"

#define	THIS	Bag
#define	BASE	Collection
DEFINE_CLASS(Bag,Collection,1,"$Header: Bag.c,v 1.2 88/01/16 23:37:44 keith Exp $",NULL,NULL);

extern const int OOPS_REMOVEERR;

Bag::Bag(unsigned size) : contents(size)
{
	count = 0;
}

Bag::Bag(const Bag& b) : contents(b.contents)
{
	count = b.count;
}

void Bag::operator=(const Bag& b)
{
	contents = b.contents;
	count = b.count;
}
	
void Bag::reSize(unsigned newSize)
{
	contents.reSize(newSize);
}

Object* Bag::addWithOccurrences(const Object& ob, unsigned n)
{
	register AssocInt* a = (AssocInt*)&contents.assocAt(ob);
	Object* o = &ob;
	if (a==nil) contents.add(*new AssocInt(ob,n));
	else {
		register Integer& i = *((Integer*)(a->value()));
		o = a->key();
		i.value(i.value()+n);
	}
	count += n;
	return o;
}

Object* Bag::add(const Object& ob)
{
	return addWithOccurrences(ob,1);
}

Object*& Bag::at(int i)	{ return contents.at(i); }

unsigned Bag::capacity()	{ return contents.capacity(); }

Collection& Bag::addContentsTo(Collection& cltn)
{
	DO(*this,Object*,ob) cltn.add(*ob); OD
	return cltn;
}

void Bag::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	contents.deepenShallowCopy();
}

Object* Bag::doNext(Iterator& pos)
{
	AssocInt* a;
	while (YES) {
		if (pos.num == 0) {
			a = (AssocInt*)contents.doNext(pos);
			if (a == NULL) return NULL;
		}
		else a = (AssocInt*)contents.at(pos.index-1);
		if (pos.num++ < ((Integer*)a->value())->value())
			return a->key();
		pos.num = 0;
	}
}

Object* Bag::remove(const Object& ob)
{
	register AssocInt* a = (AssocInt*)&contents.assocAt(ob);
	Object* rob = &ob;
	if (a==nil) setOOPSerror(OOPS_REMOVEERR,DEFAULT,this,className(),ob.className(),&ob);
	register Integer* i = (Integer*)(a->value());
	register unsigned n = i->value();
	if (--n == 0) {
		rob = a->key();
		delete contents.remove(*a);
	}
	else i->value(n);
	--count;
	return rob;
}

bool Bag::operator==(const Bag& b)
{
	return count==b.count && contents==b.contents;
}

unsigned Bag::hash()	{ return count^contents.hash(); }

bool Bag::isEqual(const Object& p)
{
	return p.isSpecies(class_Bag) && *this==*(Bag*)&p;
}

const Class* Bag::species()	{ return &class_Bag; }

unsigned Bag::occurrencesOf(const Object& ob)
{
	register AssocInt* a = (AssocInt*)&contents.assocAt(ob);
	if (a==nil) return 0;
	else return ((Integer*)a->value())->value();
}

void Bag::printOn(ostream& strm)
{
	unsigned n=0;
	strm << className() << "[\n";
	DO(contents,Object*,o)
		if (n>0) strm << "\n";
		o->printOn(strm);
		n++;
	OD
	strm << "]\n";
}

unsigned Bag::size()	{ return count; }

Bag Collection::asBag()
{
	Bag cltn(MAX(size(),CLTN_DEFAULT_CAPACITY));
	addContentsTo(cltn);
	return cltn;
}

static unsigned bag_capacity;

Bag::Bag(istream& strm, Bag& where) : contents((strm >> bag_capacity, bag_capacity))
{
	this = &where;
	count = 0;
	unsigned i,n;
	strm >> n;		// read bag size 
	while (n--) {
		strm >> i;
		addWithOccurrences(*readFrom(strm),i);
	}
}

void Bag::storer(ostream& strm)
{
	Object::storer(strm);
	strm << contents.capacity() << " " << contents.size() << " ";
	DO(contents,AssocInt*,a)
		strm << ((Integer*)(a->value()))->value();
		(a->key())->storeOn(strm);
	OD
}

 Bag::Bag(fileDescTy& fd, Bag& where) : contents((readBin(fd,bag_capacity), bag_capacity))
{
	this = &where;
	count = 0;
	unsigned i,n;
	readBin(fd,n);
	while ( n-- ) {
		readBin(fd,i);
		addWithOccurrences(*readFrom(fd),i);
	}
}

void Bag::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	storeBin(fd,contents.capacity());
	storeBin(fd,contents.size());
	DO(contents,AssocInt*,a)
		storeBin(fd,((Integer*)(a->value()))->value());
		(a->key())->storeOn(fd);
	OD
}

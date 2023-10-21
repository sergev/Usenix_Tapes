/* Set.c -- implemenation of hash tables

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
	
A Set is an unordered collection of objects in which no object is
duplicated.  Duplicate objects are defined by the function isEqual.
Sets are implemented using a hash table.  The capacity() function
returns the 1/2 the capacity of the hash table and the size() function
returns the number of objects currently in the Set.  For efficiency, the
capacity is always a power of two and is doubled whenever the table
becomes half full.

$Log:	Set.c,v $
 * Revision 1.2  88/01/16  23:41:12  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Set.h"
#include "oopsIO.h"

#define	THIS	Set
#define	BASE	Collection
DEFINE_CLASS(Set,Collection,1,"$Header: Set.c,v 1.2 88/01/16 23:41:12 keith Exp $",NULL,NULL);

extern const int OOPS_ALLOCSIZE,OOPS_REMOVEERR;

unsigned Set::setCapacity(unsigned int size)
/*
	Establish the Set capacity.  Round size up to the next highest
	power of two, if necessary.
*/
{
	if (size==0) setOOPSerror(OOPS_ALLOCSIZE,DEFAULT,this,className());
	count = 0;
	nbits = 0;
	for (register unsigned s=size; s!=0; s>>=1, nbits++);
	if (size == 1<<(nbits-1)) --nbits;
	size = 1<<nbits;
	mask = size-1;
	return size*CLTN_EXPANSION_FACTOR;		// return hash table capacity 
}

Set::Set(unsigned size) : contents(setCapacity(size)) {}

Set::Set(const Set& s) : contents(s.contents)
{
	count = s.count;
	mask = s.mask;
	nbits = s.nbits;
}

void Set::operator=(const Set& s)
{
	count = s.count;
	mask = s.mask;
	nbits = s.nbits;
	contents = s.contents;
}
	
int Set::h(unsigned long K)
/*
multiplicative hash function

Enter:
	K = key to be hashed
	
Returns:
	hash table index
	
Knuth Vol. 3, Section 6.4, pp. 508-512
*/
{
	const unsigned long Aw = 2654435769L;	
//	const unsigned long Aw = 40503;		use for 16 bit machines? 
	return ((Aw*K)>>((8*sizeof(unsigned))-nbits)) & mask;
}

int Set::findIndexOf(const Object& ob)
/*
Search this set for the specified object

Enter:
	ob = pointer to object to search for

Returns:
	index of object if found or of nil slot if not found
	
Algorithm L, Knuth Vol. 3, p. 519
*/
{
	register int i;
	for (i = h(ob.hash()); contents[i]!=nil; i = (i-1)&mask) {
		if (contents[i]->isEqual(ob)) return i;
	}
	return i;
}

void Set::reSize(unsigned newSize)
/*
	Change the capacity of this Set to newSize.
*/
{
	if (newSize <= size()) return;
	ArrayOb oldcontents = asArrayOb();
	*this = Set(newSize);
	addAll(oldcontents);
}

Object* Set::add(const Object& ob)
/*
	Add an object to this Set, making the Set larger if it
	becomes half full.
*/
{
	register int i = findIndexOf(ob);
	if (contents[i]==nil) {		// add new object to set 
		contents[i] = &ob;
		if (++count*CLTN_EXPANSION_FACTOR > capacity()) reSize(count*CLTN_EXPANSION_FACTOR);
		return &ob;		// successful add 
	}
	else return contents[i];	// object already in set 
}

Collection& Set::addContentsTo(Collection& cltn)
/*
	Add all of the objects in the specified Collection to
	this Set.
*/
{
	DO(*this,Object*,o) cltn.add(*o); OD
	return cltn;
}

Object* Set::remove(const Object& ob)
/*
remove object from set

Enter:
	ob = reference to object to be removed

Returns:
	pointer to removed object

Algorithm R, Knuth Vol. 3 p. 527
*/
{
	register int i = findIndexOf(ob);
	Object* rob = contents[i];
	if (rob==nil) setOOPSerror(OOPS_REMOVEERR,DEFAULT,this,className(),ob.className(),&ob);
	else {
		register int j,r;
		while (YES) {
			contents[j=i] = nil;
			do {
				i = (i-1)&mask;
				if (contents[i]==nil) {
					count--;
					return rob;
				}
				r = h(contents[i]->hash());
			} while ((i<=r&&r<j) || (r<j&&j<i) || (j<i&&i<=r));
			contents[j] = contents[i];
		}
	}
}

bool Set::operator==(const Set& s)
/*
	Return YES if the specified Set equals this Set.
*/
{
	if (count!=s.count) return NO;
	for (register int i=0; i<capacity(); i++) {
		if (contents[i]!=nil && !s.includes(*contents[i])) return NO;
	}
	return YES;
}

Set Set::operator-(const Set& s)
/*
	Returns a Set of all of the objects that are contained in this
	Set but not in the specified Set.
*/
{
	Set diff = *this;
	for (register int i=0; i<capacity(); i++) {
		if (contents[i]!=nil && s.includes(*contents[i])) diff.remove(*contents[i]);
	}
	return diff;
}

Set Set::operator&(const Set& s)
/*
	Returns a Set of all objects that are in both this Set and
	the specified Set.
*/
{
	Set intersection = *this;
	for (register int i=0; i<capacity(); i++) {
		if (contents[i]!=nil && !s.includes(*contents[i])) intersection.remove(*contents[i]);
	}
	return intersection;
}

Set Set::operator|(const Set& s)
/*
	Returns a Set of all objects that are in either this Set
	or the specified Set.
*/
{
	Set u = *this;
	u.addAll(s);
	return u;
}

Object*& Set::at(int i)	{ return contents[i]; }

bool Set::isEqual(const Object& p)
/*
	Returns YES if this Set equals the specified object.
*/
{
	return p.isSpecies(class_Set) && *this==*(Set*)&p;
}

const Class* Set::species()	{ return &class_Set; }

void Set::deepenShallowCopy()
{
	BASE::deepenShallowCopy();
	contents.deepenShallowCopy();
}

Object* Set::doNext(Iterator& pos)
{
	Object* ob;
	while (pos.index < capacity()) {
		if ((ob = contents[pos.index++]) != nil) return ob;
	}
	return 0;
}

unsigned Set::capacity()  { return contents.capacity()/CLTN_EXPANSION_FACTOR; }

unsigned Set::hash()
{
	unsigned h = 0;
	DO(*this,Object*,o) h ^= o->hash(); OD
	return h;
}

unsigned Set::occurrencesOf(const Object& ob)
/*
	Return the number of occurences of thw specified object
	in this Set (either 0 or 1).
*/
{
	if (contents[findIndexOf(ob)]!=nil) return 1;
	else return 0;
}

Object* Set::findObjectWithKey(const Object& ob) { return contents[findIndexOf(ob)]; }

void Set::printOn(ostream& strm)
{
	unsigned n=0;
	strm << className() << "[\n";
	DO(*this,Object*,o)
		if (n>0) strm << "\n";
		o->printOn(strm);
		n++;
	OD
	strm << "]\n";
}
 
unsigned Set::size()		{ return count; }

Set Collection::asSet()
/*
	Convert this Collection to a Set.
*/
{
	Set cltn(MAX(size(),CLTN_DEFAULT_CAPACITY));
	addContentsTo(cltn);
	return cltn;
}

static unsigned set_capacity;

Set::Set(istream& strm, Set& where) : contents((strm >> set_capacity, setCapacity(set_capacity)))
{
	this = &where;
	unsigned n;
	strm >> n;		// read Set size 
	while (n--) add(*readFrom(strm));
}

 Set::Set(fileDescTy& fd, Set& where) : contents((readBin(fd,set_capacity),setCapacity(set_capacity) ))
{
	this = &where;
	unsigned n;
	readBin(fd,n);
	while (n--) add(*readFrom(fd));
}

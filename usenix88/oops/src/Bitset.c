/* Bitset.c -- implementation of set of small integers

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
	
A Bitset is a set of small integers.  It is implemented very efficiently
using a single word.  Each bit of the word indicates if the integer
associated with the bit position is in the set.  Bitsets are
particularly useful in conjunction with enum constants.

$Log:	Bitset.c,v $
 * Revision 1.2  88/01/16  23:38:00  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Bitset.h"
#include "oopsIO.h"

#define	THIS	Bitset
#define	BASE	Object
DEFINE_CLASS(Bitset,Object,1,"$Header: Bitset.c,v 1.2 88/01/16 23:38:00 keith Exp $",NULL,NULL);

unsigned Bitset::capacity()	{ return sizeof(int)*8; }

Object* Bitset::copy()		{ return shallowCopy(); }

void Bitset::deepenShallowCopy()	{}

unsigned Bitset::hash()	{ return m; }
	
bool Bitset::isEmpty()	{ return m==0; }
	
bool Bitset::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Bitset) && *this==*(Bitset*)&ob;
}

const Class* Bitset::species()	{ return &class_Bitset; }

void Bitset::printOn(ostream& s)
{
	Bitset t = *this;
	s << "[";
	for (register unsigned i =0; i<capacity() && !t.isEmpty(); i++) {
		if (t.includes(i)) {
			s << i;
			t -= i;
			if (!t.isEmpty()) s << ",";
		}
	}
	s << "]";
}

unsigned Bitset::size()
{
	register unsigned l=m;
	register unsigned n=0;
	while (l != 0) {
		l &= (l-1);	// removes rightmost 1 
		n++;
	}
	return n;
}

Bitset::Bitset(istream& strm, Bitset& where)
{
	this = &where;
	strm >> m;
}

void Bitset::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << m << " ";
}

Bitset::Bitset(fileDescTy& fd, Bitset& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Bitset::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}

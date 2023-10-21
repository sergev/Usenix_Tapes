/* Range.c  -- implementation of OOPS class Range

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
        C. J. Eppich
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5361
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov
	September, 1987

Function:

Class Range implements an ordered pair of ints that can be used to indicate
a segment of some array (possibly a character string or vector).

$Log:	Range.c,v $
 * Revision 1.2  88/01/16  23:31:58  keith
 * Remove pre-RCS modification history
 * Replace operator()(int,int) with int length(int), int firstIndex(int),
 * 	and int lastIndex(int).
 * 
*/

#include "Range.h"
#include "oopsIO.h"

#define	THIS	Range
#define	BASE	Object
DEFINE_CLASS(Range,Object,1,"$Header: Range.c,v 1.2 88/01/16 23:31:58 keith Exp $",NULL,NULL);

//======= Protected member functions:

Range::Range(istream& strm, Range& where)
{
	this = &where;
	strm >> first >> len;
}

void Range::storer(ostream& strm)
{
	Object::storer(strm);
	strm << first << " " << len << " ";
}

Range::Range(fileDescTy& fd, Range& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Range::storer(fileDescTy& fd) 
{
	Object::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}


//======= Public member functions:

Object* Range::copy()
{
	return shallowCopy();
}

void Range::deepenShallowCopy()  {}

unsigned Range::hash()
{
	return (first^len);
}

bool Range::isEqual(const Object& p)
// Test two objects for equality
{
	return p.isSpecies(class_Range) && *this==*(Range*)&p;
}

void Range::printOn(ostream& strm)
{
	strm << first << ":" << len;
}

const Class* Range::species()
// Return a pointer to the descriptor of the species of this class
{
	return &class_Range;
}

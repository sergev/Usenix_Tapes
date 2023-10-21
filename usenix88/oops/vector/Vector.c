/* Vector.c -- abstract base class for vectors

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
	uucp: {decvax!}seismo!elsie!cecil!keith
	February, 1987

Function:
	
Modification History:

$Log:	Vector.c,v $
 * Revision 1.1  88/01/17  09:47:19  keith
 * Initial revision
 * 

*/

#include "Vector.h"
#include "oopsIO.h"

#define	THIS	Vector
#define	BASE	Object
DEFINE_CLASS(Vector,Object,1,"$Header: Vector.c,v 1.1 88/01/17 09:47:19 keith Exp $",NULL,NULL);

extern const int OOPS_RDABSTCLASS;
extern const int OOPS_VECTOREMPTY;
extern const int OOPS_VECTORLENGTH;

unsigned Vector::capacity()	{ return n; }

unsigned Vector::size() { return n; }

void Vector::free() {}	// used by Slice classes to free TempVecs

void Vector::emptyErr(const char* fname)
{
	setOOPSerror(OOPS_VECTOREMPTY,DEFAULT,className(),fname,(int)this);
}

void lengthErr(const Vector& U, const Vector& V)
{
	setOOPSerror(OOPS_VECTORLENGTH,DEFAULT,(int)&U,U.className(),U.length(),(int)&V,V.className(),V.length());
}

void Vector::deepenShallowCopy()
// Called by deepCopy() to convert a shallow copy to a deep copy.
{
}

Vector::Vector(istream& strm, Vector& where)
// Construct an object from istream "strm" at address "where".
{
	this = &where;
	strm >> n;
}

void Vector::storer(ostream& strm)
// Store the member variables of this object on ostream "strm".
{
	BASE::storer(strm);
	strm << n << " ";
}

Vector::Vector(fileDescTy& fd, Vector& where)
// Construct an object from file descriptor "fd" at address "where".
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Vector::storer(fileDescTy& fd) 
// Store an object on file descriptor "fd".
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}

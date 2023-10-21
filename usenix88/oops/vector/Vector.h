#ifndef	VECTOR_H
#define	VECTOR_H

/* Vector.h -- declarations for generic vector objects

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
	May, 1986


Function:


Modification History:

$Log:	Vector.h,v $
 * Revision 1.1  88/01/17  09:47:11  keith
 * Initial revision
 * 
	
*/

#include "Object.h"

overload abs;
overload acos;
overload asin;
overload atan;
overload atan2;
overload ceil;
overload cos;
overload cosh;
overload cross;
overload cumsum;
overload delta;
overload dot;
overload exp;
overload floor;
overload log;
overload max;
overload min;
overload pow;
overload prod;
overload reverse;
overload sin;
overload sinh;
overload sqrt;
overload sum;
overload tan;
overload tanh;

#include <math.h>

typedef double (*mathFunTy)(double);

class BitVec;
class ByteVec;
class ComplexVec;
class DoubleVec;
class FloatVec;
class IntVec;
class LongVec;
class ShortVec;

extern const Class class_Vector;
overload Vector_reader;

class Vector : public Object {
protected:
	unsigned n;		// number of elements in vector
	int reserved;		// reserved for future use
protected:
	Vector(unsigned len=0)	{ n = len; }
	Vector(fileDescTy&,Vector&);
	Vector(istream&,Vector&);
	friend	void Vector_reader(istream& strm, Object& where);
	friend	void Vector_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	unsigned length()	{ return n; }
	void emptyErr(const char* fname);
	virtual unsigned capacity();
	virtual void	deepenShallowCopy();	// {}
	virtual void	free();			// {}
	virtual const Class* isA();
	virtual unsigned size();
friend	void lengthErr(const Vector&, const Vector&);
};

#endif

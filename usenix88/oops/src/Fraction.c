/* Fraction.c -- implementation of fractions

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
	December, 1985

Function:
	
Implements a fraction as two integers, the numerator and the
denominator.  WARNING -- this implementation is not suitable for serious
numeric applications.  Reference: Knuth, "The Art of Computer
Programming", Vol. 2, Section 4.5.

$Log:	Fraction.c,v $
 * Revision 1.2  88/01/16  23:39:02  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Fraction.h"
#include <libc.h>
#include <math.h>
#include "oopsIO.h"

#define	THIS	Fraction
#define	BASE	Object
DEFINE_CLASS(Fraction,Object,1,"$Header: Fraction.c,v 1.2 88/01/16 23:39:02 keith Exp $",NULL,NULL);

extern const int OOPS_ZERODEN,OOPS_FCTNOV,OOPS_FCTNUN;

static int gcd(int uu, int vv)
/* gcd -- binary greatest common divisor algorithm

Algorithm B, p. 321.

*/
{
	register int u=ABS(uu), v=ABS(vv);
	register int k=0;
	register int t;
	if (u == 0) return v;
	if (v == 0) return u;
	while ((u&1) == 0 && (v&1) == 0) { u>>=1; v>>=1; k++; }
	if (u&1) { t = -v; goto B4; }
	else t = u;
	do {
B3:		t>>=1;
B4:		if ((t&1) == 0) goto B3;
		if (t>0) u = t;
		else v = -t;
		t = u-v;
	} while (t != 0);
	return u<<k;
}

Fraction::Fraction(int num, int den)
/*
	Construct a Fraction from the specified numerator and denominator.
*/
{
	n = num; d = den;
	if (d == 0) setOOPSerror(OOPS_ZERODEN,DEFAULT,this,num,den);
	if (n == 0) { d = 1; return; }
	if (d < 0) { n = -n; d = -d; }
	reduce();
}

Fraction::Fraction(double x)
/*
	Construct a Fraction from a double.
*/
{
	char buf[20];
	int exp;
	double m = frexp(x,&exp);
	register int k;
	if (exp>=0) {
		if (exp > (sizeof(int)*8-2)) setOOPSerror(OOPS_FCTNOV,DEFAULT,this,gcvt(x,20,buf));
		k = (sizeof(int)*8-2);
	}
	else {
		k = exp+(sizeof(int)*8-2);
		if (k < 0) setOOPSerror(OOPS_FCTNUN,DEFAULT,this,gcvt(x,20,buf));
	}
	n = (int)(m*(1<<k));
	d = 1 << (k-exp);
	reduce();
}

void Fraction::parseFraction(istream& strm)
/*
	Read a Fraction from an istream.
*/
{
	n = 0; d = 1;
	strm >> n;
	char slash;
	strm >> slash;
	if (slash == '/') {
		strm >> d;
		reduce();
	}
	else strm.putback(slash);
}

Fraction::Fraction(istream& strm)	{ parseFraction(strm); }

void Fraction::reduce()
/*
	Reduce a Fraction to lowest terms by dividing the numerator and
	denominator by their gcd.
*/
{
	register int d1 = gcd(n,d);
	if (d1 == 1) return;
	n /= d1; d /= d1;
}

Fraction operator+(const Fraction& u, const Fraction& v)
{
	register int d1 = gcd(u.d,v.d);
	if (d1 == 1) return Fraction(u.n*v.d+u.d*v.n, u.d*v.d, 0);
	register int t = u.n*(v.d/d1) + v.n*(u.d/d1);
	register int d2 = gcd(t,d1);
	return Fraction(t/d2, (u.d/d1)*(v.d/d2), 0);
}

Fraction operator-(const Fraction& u, const Fraction& v)
{
	register int d1 = gcd(u.d,v.d);
	if (d1 == 1) return Fraction(u.n*v.d-u.d*v.n, u.d*v.d, 0);
	register int t = u.n*(v.d/d1) - v.n*(u.d/d1);
	register int d2 = gcd(t,d1);
	return Fraction(t/d2, (u.d/d1)*(v.d/d2), 0);
}

bool operator<(const Fraction& u, const Fraction& v)
{
	register int d1 = gcd(u.d,v.d);
	if (d1 == 1) return u.n*v.d < u.d*v.n;
	return u.n*(v.d/d1) < v.n*(u.d/d1);
}

bool operator<=(const Fraction& u, const Fraction& v)
{
	if (u==v) return YES;
	return u<v;
}

Fraction operator*(const Fraction& u, const Fraction& v)
{
	register int d1 = gcd(u.n, v.d);
	register int d2 = gcd(u.d, v.n);
	return Fraction((u.n/d1)*(v.n/d2), (u.d/d2)*(v.d/d1), 0);
}

Fraction operator/(const Fraction& u, const Fraction& v)
{
	if (v.n < 0) return u*Fraction(-v.d,-v.n, 0);
	return u*Fraction(v.d,v.n,0);
}

bool Fraction::between(const Fraction& min, const Fraction& max)
/*
	Return YES if this Fraction is <= to max and >= to min.
*/
{
	return *this >= min && *this <= max;
}

Fraction Fraction::max(const Fraction& f)
{
	if (f < *this) return *this;
	else return f;
}

Fraction Fraction::min(const Fraction& f)
{
	if (f > *this) return *this;
	else return f;
}

unsigned Fraction::hash()	{ return n^d; }

bool Fraction::isEqual(const Object& ob)
{
	return ob.isSpecies(class_Fraction) && *this==*(Fraction*)&ob;
}

const Class* Fraction::species()	{ return &class_Fraction; }

int Fraction::compare(const Object& ob)
{
	assertArgSpecies(ob,class_Fraction,"compare");
	if (*this == *(Fraction*)&ob) return 0;
	if (*this < *(Fraction*)&ob) return -1;
	return 1;
}

Object* Fraction::copy()		{ return shallowCopy(); }

void Fraction::deepenShallowCopy()	{}

void Fraction::printOn(ostream& strm)
{
	if (n == 0 || d == 1) strm << n;
	else strm << n << "/" << d;
}

void Fraction::scanFrom(istream& strm)	{ parseFraction(strm); }

Fraction::Fraction(istream& strm, Fraction& where)
{
	this = &where;
	strm >> n >> d;
}

void Fraction::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << n << " " << d << " ";
}

Fraction::Fraction(fileDescTy& fd, Fraction& where)
{
	this = &where;
	READ_OBJECT_AS_BINARY(fd);
}

void Fraction::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	STORE_OBJECT_AS_BINARY(fd);
}

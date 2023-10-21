#ifndef	FRACTION_H
#define	FRACTION_H

/* Fraction.h -- declarations for fractions

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

$Log:	Fraction.h,v $
 * Revision 1.3  88/02/04  12:59:15  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:39:05  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_Fraction;
overload Fraction_reader;

class Fraction: public Object {
	int n,d;
	Fraction(int num, int den, int dum) {
		n = (dum,num); d = den;
	}
	void parseFraction(istream&);
	void reduce();
protected:
	Fraction(fileDescTy&,Fraction&);
	Fraction(istream&,Fraction&);
	friend	void Fraction_reader(istream& strm, Object& where);
	friend	void Fraction_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Fraction(int num =0, int den =1);
	Fraction(double);
	Fraction(istream&);
	operator double()		{ return (double)n/d; }
	int denominator()		{ return d; }
	int numerator()			{ return n; }
	
	friend Fraction	operator+(const Fraction&, const Fraction&);
	friend Fraction	operator-(Fraction u)			{ return Fraction(-u.n,u.d); }
	friend Fraction operator-(const Fraction&, const Fraction&);
	friend Fraction operator*(const Fraction&, const Fraction&);
	friend Fraction operator/(const Fraction&, const Fraction&);
	friend bool	operator<(const Fraction& u, const Fraction& v);
	friend bool	operator>(Fraction u, Fraction v)	{ return v<u; }
	friend bool	operator<=(const Fraction& u, const Fraction& v);
	friend bool	operator>=(Fraction u, Fraction v)	{ return v<=u; }
	friend bool	operator==(Fraction u, Fraction v)	{ return u.n == v.n && u.d == v.d; }
	friend bool	operator!=(Fraction u, Fraction v)	{ return !(u==v); }
	
	void operator+=(Fraction u)	{ *this = *this + u; }
	void operator-=(Fraction u)	{ *this = *this - u; }
	void operator*=(Fraction u)	{ *this = *this * u; }
	void operator/=(Fraction u)	{ *this = *this / u; }
	
	bool between(const Fraction& min, const Fraction& max);
	Fraction max(const Fraction&);
	Fraction min(const Fraction&);
	
	virtual int compare(const Object&);
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool isEqual(const Object&);
	virtual void printOn(ostream& strm);
	virtual void scanFrom(istream& strm);
	virtual const Class* species();
};

#endif

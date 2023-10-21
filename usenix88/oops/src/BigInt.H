#ifndef BIGINTH
#define BIGINTH

/* BigInt.h -- Specification of multi-precision integers

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
	December, 1987

Modification History:

*/

#include "Object.h"

extern Class class_BigInt;
overload BigInt_reader;

class BigInt: public Object {
	unsigned len;
	short*	n;
protected:
	BigInt(fileDescTy&,BigInt&);
	BigInt(istream&,BigInt&);
	friend	void BigInt_reader(fileDescTy& fd, Object& where);
	friend	void BigInt_reader(istream& strm, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	BigInt(const char*);
	BigInt(int i=0);
	BigInt(const BigInt&);
	~BigInt()		{ delete n.val; }
	operator int();
	BigInt operator-();
	BigInt operator!();
	BigInt operator~();
	BigInt operator++();
	BigInt operator--();
	friend BigInt operator*(const BigInt&, const BigInt&);
	friend BigInt operator/(const BigInt&, const BigInt&);
	friend BigInt operator%(const BigInt&, const BigInt&);
	friend BigInt operator>>(const BigInt&, const BigInt&);
	friend BigInt operator<<(const BigInt&, const BigInt&);
	friend BigInt operator+(const BigInt&, const BigInt&);
	friend BigInt operator-(const BigInt&, const BigInt&);
	friend bool operator<(const BigInt&, const BigInt&);
	friend bool operator>(const BigInt&, const BigInt&);
	friend bool operator<=(const BigInt&, const BigInt&);
	friend bool operator>=(const BigInt&, const BigInt&);
	friend bool operator!=(const BigInt&, const BigInt&);
	friend BigInt operator^(const BigInt&, const BigInt&);
	friend BigInt operator&(const BigInt&, const BigInt&);
	friend BigInt operator|(const BigInt&, const BigInt&);
	BigInt operator=(const BigInt&);
	BigInt operator+=(const BigInt&);
	BigInt operator-=(const BigInt&);
	BigInt operator*=(const BigInt&);
	BigInt operator/=(const BigInt&);
	BigInt operator%=(const BigInt&);
	BigInt operator>>=(const BigInt&);
	BigInt operator<<=(const BigInt&);
	BigInt operator&=(const BigInt&);
	BigInt operator^=(const BigInt&);
	BigInt operator|=(const BigInt&);
	virtual	int	compare(const Object&);
	virtual Object*	copy();		// do not define to default deepCopy()
	virtual void	deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream& strm);
	virtual const Class* species();
};

#endif

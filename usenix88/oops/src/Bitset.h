#ifndef	BITSET_H
#define	BITSET_H

/* BitSet.h -- declarations for set of small integers

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

$Log:	Bitset.h,v $
 * Revision 1.3  88/02/04  12:55:03  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:38:03  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

inline unsigned BIT(unsigned i)	{ return 1<<i; }

extern const Class class_Bitset;
overload Bitset_reader;

class Bitset: public Object {
protected:
	unsigned m;
	Bitset(int i, float dum)	{ (dum, m = i); }
protected:
	Bitset(fileDescTy&,Bitset&);
	Bitset(istream&,Bitset&);
	friend	void Bitset_reader(istream& strm, Object& where);
	friend	void Bitset_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	Bitset()			{ m = 0; }
	Bitset(int i1)			{ m = BIT(i1); }
	Bitset(int i1, int i2)		{ m = BIT(i1)|BIT(i2); }
	Bitset(int i1, int i2, int i3)
		{ m = BIT(i1)|BIT(i2)|BIT(i3); }
	Bitset(int i1, int i2, int i3, int i4)
		{ m = BIT(i1)|BIT(i2)|BIT(i3)|BIT(i4); }
	Bitset(int i1, int i2, int i3, int i4, int i5)
		{ m = BIT(i1)|BIT(i2)|BIT(i3)|BIT(i4)|BIT(i5); }
	Bitset(int i1, int i2, int i3, int i4, int i5, int i6)
		{ m = BIT(i1)|BIT(i2)|BIT(i3)|BIT(i4)|BIT(i5)|BIT(i6); }
	Bitset(int i1, int i2, int i3, int i4, int i5, int i6, int i7)
		{ m = BIT(i1)|BIT(i2)|BIT(i3)|BIT(i4)|BIT(i5)|BIT(i6)|BIT(i7); }
	Bitset(const Bitset& n)		{ m = n.m; }
	Bitset operator~()		{ return Bitset(~m, 0.0); }
	Bitset operator-(Bitset n)	{ return Bitset(m & ~n.m, 0.0); }
	bool operator>(Bitset n)	{ return m == (m|n.m) && m != n.m; }
	bool operator<(Bitset n)	{ return n.m == (m|n.m) && m != n.m; }
	bool operator>=(Bitset n)	{ return m == (m|n.m); }
	bool operator<=(Bitset n)	{ return n.m == (m|n.m); }
	bool operator==(Bitset n)	{ return m == n.m; }
	bool operator!=(Bitset n)	{ return m != n.m; }
	Bitset operator&(Bitset n)	{ return Bitset(m & n.m, 0.0); }
	Bitset operator^(Bitset n)	{ return Bitset(m ^ n.m, 0.0); }
	Bitset operator|(Bitset n)	{ return Bitset(m | n.m, 0.0); }
	void operator=(Bitset n)	{ m = n.m; }
 	void operator-=(Bitset n)	{ m &= ~n.m; }
	void operator&=(Bitset n)	{ m &= n.m; }
	void operator^=(Bitset n)	{ m ^= n.m; }
	void operator|=(Bitset n)	{ m |= n.m; }
	int asMask()			{ return m; }
	bool includes(int i)		{ return (m & BIT(i)) != 0; }
	virtual unsigned capacity();
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned hash();
	virtual const Class*	isA();
	virtual bool	isEmpty();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream&);
	virtual unsigned size();
	virtual const Class*	species();
};

#endif

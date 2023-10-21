#ifndef	INTVEC_H
#define	INTVEC_H

/* IntVec.h -- Integer Vectors

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

Modification History:

$Log:	IntVec.h,v $
 * Revision 1.1  88/01/17  09:47:09  keith
 * Initial revision
 * 
	
*/
#include "Vector.h"
#include "BitVec.h"

class IntSlice;
class IntPick;
class IntSlct;

extern const Class class_IntVec;
overload IntVec_reader;

class IntVec : public Vector {
	int* v;		// pointer to data, NULL if empty vector
	void indexRangeErr();
protected:
	IntVec(fileDescTy&,IntVec&);
	IntVec(istream&,IntVec&);
	friend	void IntVec_reader(istream& strm, Object& where);
	friend	void IntVec_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	IntVec(register unsigned len =0);
	IntVec(register unsigned len, int from, int by =1);
	IntVec(const int*, unsigned len);
	IntVec(const IntVec&);
	IntVec(const IntSlice&);
	~IntVec()			{ delete v; }
	IntSlice	operator()(int pos, unsigned lgt, int stride =1);
	operator int*()		{ return v; }
	operator IntSlice();
	operator DoubleVec();
//	operator LongVec();
	int&		operator[](int i) {	// vector element
		if ((unsigned)i >= n) indexRangeErr();
		return v[i];
	}
	int&		operator()(int i)		{ return v[i]; }
	IntPick		operator[](const IntVec&);
	IntSlct		operator[](const BitVec&);
	void /*IntVec::*/operator=(const IntVec&);
	void /*IntVec::*/operator=(const IntSlice&);
	void /*IntVec::*/operator=(const IntSlct&);
	void /*IntVec::*/operator=(const IntPick&);
	void /*IntVec::*/operator=(int);
	void /*IntVec::*/lengthErr(const IntSlice&);
	void selectErr(const BitVec&);
	virtual void	deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream& strm);
	virtual void	scanFrom(istream& strm);
	virtual Object*	shallowCopy();
	virtual void	sort();
	virtual const Class* species();
};

inline unsigned BitPick::length()	{ return X->length(); }
	
class TempIntVec : public IntVec {
	friend IntSlice;
	friend IntPick;
	friend IntSlct;
	TempIntVec(register unsigned len =0) : (len) {}
	virtual void free();
};

class IntSlice {
	IntVec* V;	// vector pointer
	int* p;		// slice pointer
	unsigned l;	// slice length
	int k;		// slice stride
	IntSlice(IntVec& v, int pos, unsigned lgt, int stride =1);
	IntSlice(IntVec& v, unsigned lgt) {
		V = &v;  p = v;  l = lgt;  k = 1;
	}
	friend IntVec;
public:
	IntSlice(const IntPick&);
	IntSlice(const IntSlct&);
	~IntSlice()		{ V->free(); }
	operator int*()		{ return p; }
	unsigned length()	{ return l; }
	int stride()		{ return k; }
	void /*IntSlice::*/operator=(const IntVec&);
	void /*IntSlice::*/operator=(const IntPick&);
	void /*IntSlice::*/operator=(const IntSlct&);
	void /*IntSlice::*/operator=(const IntSlice&);
	void /*IntSlice::*/operator=(int);
	void /*IntSlice::*/lengthErr(const IntVec&);
	void /*IntSlice::*/lengthErr(const IntSlice&);
	void selectErr(const BitVec&);
friend	IntVec	operator-(const IntSlice&);
friend	IntVec	operator!(const IntSlice&);
friend	IntVec	operator~(const IntSlice&);
friend	IntVec	operator++(IntSlice&);
friend	IntVec	operator--(IntSlice&);
friend	IntVec	operator*(const IntSlice&,const IntSlice&);
friend	IntVec	operator/(const IntSlice&,const IntSlice&);
friend	IntVec	operator%(const IntSlice&,const IntSlice&);
friend	IntVec	operator+(const IntSlice&,const IntSlice&);
friend	IntVec	operator-(const IntSlice&,const IntSlice&);
friend	IntVec	operator&(const IntSlice&,const IntSlice&);
friend	IntVec	operator^(const IntSlice&,const IntSlice&);
friend	IntVec	operator|(const IntSlice&,const IntSlice&);
friend	IntVec	operator*(const IntSlice&,int);
friend	IntVec	operator*(int s,const IntSlice& V)  { return V*s; }
friend	IntVec	operator/(const IntSlice&,int);
friend	IntVec	operator/(int,const IntSlice&);
friend	IntVec	operator%(const IntSlice&,int);
friend	IntVec	operator%(int,const IntSlice&);
friend	IntVec	operator+(const IntSlice&,int);
friend	IntVec	operator+(int s,const IntSlice& V)  { return V+s; }
friend	IntVec	operator-(const IntSlice&,int);
friend	IntVec	operator-(int,const IntSlice&);
friend	IntVec	operator&(const IntSlice&,int);
friend	IntVec	operator&(int s,const IntSlice& V)  { return V&s; }
friend	IntVec	operator^(const IntSlice&,int);
friend	IntVec	operator^(int s,const IntSlice& V)  { return V^s; }
friend	IntVec	operator|(const IntSlice&,int);
friend	IntVec	operator|(int s,const IntSlice& V)  { return V|s; }
friend	BitVec	operator<(const IntSlice&,const IntSlice&);
friend	BitVec	operator>(const IntSlice& U,const IntSlice& V)	{ return V < U; }
friend	BitVec	operator<=(const IntSlice&,const IntSlice&);
friend	BitVec	operator>=(const IntSlice& U,const IntSlice& V) { return V <= U; }
friend	BitVec	operator==(const IntSlice&,const IntSlice&);
friend	BitVec	operator!=(const IntSlice&,const IntSlice& V);
friend	BitVec	operator<(const IntSlice&,int);
friend	BitVec	operator<(int s,const IntSlice& V)  { return V > s; }
friend	BitVec	operator>(const IntSlice&,int);
friend	BitVec	operator>(int s,const IntSlice& V)  { return V < s; }
friend	BitVec	operator<=(const IntSlice&,int);
friend	BitVec	operator<=(int s,const IntSlice& V) { return V >= s; }
friend	BitVec	operator>=(const IntSlice&,int);
friend	BitVec	operator>=(int s,const IntSlice& V) { return V <= s; }
friend	BitVec	operator==(const IntSlice&,int);
friend	BitVec	operator==(int s,const IntSlice& V) { return V == s; }
friend	BitVec	operator!=(const IntSlice&,int);
friend	BitVec	operator!=(int s,const IntSlice& V) { return V != s; }
friend	void	operator+=(IntSlice&,const IntSlice&);
friend	void	operator+=(IntSlice&,int);
friend	void	operator-=(IntSlice&,const IntSlice&);
friend	void	operator-=(IntSlice&,int);
friend	void	operator*=(IntSlice&,const IntSlice&);
friend	void	operator*=(IntSlice&,int);
friend	void	operator/=(IntSlice&,const IntSlice&);
friend	void	operator/=(IntSlice&,int);
friend	void	operator%=(IntSlice&,const IntSlice&);
friend	void	operator%=(IntSlice&,int);
friend	void	operator&=(IntSlice&,const IntSlice&);
friend	void	operator&=(IntSlice&,int);
friend	void	operator^=(IntSlice&,const IntSlice&);
friend	void	operator^=(IntSlice&,int);
friend	void	operator|=(IntSlice&,const IntSlice&);
friend	void	operator|=(IntSlice&,int);
friend	IntVec	abs(const IntSlice& V);
friend	IntVec	cumsum(const IntSlice&);
friend	IntVec	delta(const IntSlice&);
friend	int	dot(const IntSlice&,const IntSlice&);
friend	int	max(const IntSlice&);
friend	int	min(const IntSlice&);
friend	int	prod(const IntSlice&);
friend	IntVec	reverse(const IntSlice&);
friend	int	sum(const IntSlice&);
};

class IntPick {
	IntVec* V;
	IntVec* X;
	IntPick(IntVec& v,const IntVec& x)	{ V = &v;  X = &x; }
	friend IntVec;
	friend IntSlice;
	friend IntSlct;
public:
	void /*IntPick::*/operator=(const IntVec&);
	void /*IntPick::*/operator=(const IntPick&);
	void /*IntPick::*/operator=(const IntSlct&);
	void /*IntPick::*/operator=(const IntSlice&);
	void /*IntPick::*/operator=(int);
	unsigned length()	{ return X->length(); }
};

class IntSlct {
	IntVec* V;
	BitVec* B;
	IntSlct(IntVec& v, const BitVec& b)	{ V = &v;  B = &b; }
	friend IntVec;
	friend IntSlice;
	friend IntPick;
public:
	void /*IntSlct::*/operator=(const IntVec&);
	void /*IntSlct::*/operator=(const IntPick&);
	void /*IntSlct::*/operator=(const IntSlct&);
	void /*IntSlct::*/operator=(const IntSlice&);
	void /*IntSlct::*/operator=(int);
	unsigned length()	{ return B->length(); }
};

static /*inline*/ IntSlice IntVec::operator()(int pos, unsigned lgt, int stride) // static due to cfront bug
{
	IntSlice s(*this,pos,lgt,stride);
	return s;
}

static /*inline*/ IntVec::operator IntSlice() // static due to cfront bug
{
	IntSlice s(*this,length());
	return s;
}

static /*inline*/ IntPick IntVec::operator[](const IntVec& I) // static due to cfront bug
{
	return IntPick(*this,I);
}

static /*inline*/ IntSlct IntVec::operator[](const BitVec& B) // static due to cfront bug
{
	return IntSlct(*this,B);
}

#endif

#ifndef	BITVEC_H
#define	BITVEC_H

/* BitVec.h -- Bit Vectors

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

$Log:	BitVec.h,v $
 * Revision 1.1  88/01/17  09:46:57  keith
 * Initial revision
 * 
	
*/

#include "Vector.h"

typedef unsigned char bitVecByte;

class BitSlice;
class BitPick;
class BitSlct;
class BitSliceIstream;
class BitSliceOstream;
class BitPickIstream;
class BitPickOstream;

extern const Class class_BitVec;
overload BitVec_reader;

class BitRef {
	bitVecByte* p;	// pointer to byte containing bit
	bitVecByte m;	// mask for bit
	BitRef(bitVecByte* v, int i) {
		p = v + (i>>3);
		m = 1 << (i&7);
	}
	friend BitVec;
public:
	operator bool()		{ return ((*p & m) != 0); }
	bool operator=(bool b) {
		if (b) *p |= m;
		else *p &= ~m;
		return b;
	}
	void operator&=(bool b) { if (!b) *p &= ~m; }
	void operator|=(bool b) { if (b) *p |= m; }
	void operator^=(bool b) { if (b) *p ^= m; }
};

class BitVec : public Vector {
	bitVecByte* v;	// pointer to data, NULL if empty vector
	void indexRangeErr();
protected:
	BitVec(fileDescTy&,BitVec&);
	BitVec(istream&,BitVec&);
	friend	void BitVec_reader(istream& strm, Object& where);
	friend	void BitVec_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	BitVec(register unsigned len =0);
	BitVec(register unsigned len, bool init);
	BitVec(const bitVecByte*, unsigned len);
	BitVec(const BitVec&);
	BitVec(const BitSlice&);
	~BitVec()			{ delete v; }
	BitSlice	operator()(int pos, unsigned lgt, int stride =1);
	operator bitVecByte*()		{ return v; }
	operator BitSlice();
	BitRef		operator[](int i);
	BitRef		operator()(int i)		{ return BitRef(v,i); }
	BitPick	operator[](const IntVec&);
	BitSlct	operator[](const BitVec&);
	void /*BitVec::*/operator=(const BitVec&);
	void /*BitVec::*/operator=(const BitSlice&);
	void /*BitVec::*/operator=(const BitSlct&);
	void /*BitVec::*/operator=(const BitPick&);
	void /*BitVec::*/operator=(bool);
	unsigned nbytes()	{ return (n+7) >> 3; }
	void /*BitVec::*/lengthErr(const BitSlice&);
	void selectErr(const BitVec&);
friend	BitVec	operator!(const BitVec&);
friend	BitVec	operator&(const BitVec&,const BitVec&);
friend	BitVec	operator^(const BitVec&,const BitVec&);
friend	BitVec	operator|(const BitVec&,const BitVec&);
friend	void	operator&=(BitVec&,const BitVec&);
friend	void	operator^=(BitVec&,const BitVec&);
friend	void	operator|=(BitVec&,const BitVec&);
friend	int	sum(const BitVec&);
	virtual void	deepenShallowCopy();
	virtual unsigned hash();
	virtual const Class* isA();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream& strm);
	virtual void	scanFrom(istream& strm);
	virtual Object*	shallowCopy();
	virtual const Class* species();
};

static /*inline*/ BitRef BitVec::operator[](int i) // static due to cfront bug
{
	if ((unsigned)i >= n) indexRangeErr();
	return BitRef(v,i);
}
	
class TempBitVec : public BitVec {
	friend BitSlice;
	friend BitPick;
	friend BitSlct;
	TempBitVec(register unsigned len =0) : (len) {}
	virtual void free() { delete this; }
};

class BitSlice {
	BitVec* V; // vector pointer
	int p;		// slice bit number
	unsigned l;	// slice length
	int k;		// slice stride
	BitSlice(BitVec& v, int pos, unsigned lgt, int stride =1);
	BitSlice(BitVec& v, unsigned lgt) {
		V = &v;  p = 0;  l = lgt;  k = 1;
	}
	friend BitVec;
	friend BitSliceIstream;
	friend BitSliceOstream;
public:
	BitSlice(const BitPick&);
	BitSlice(const BitSlct&);
	~BitSlice()		{ V->free(); }
	int pos()		{ return p; }
	unsigned length()	{ return l; }
	int stride()		{ return k; }
	void /*BitSlice::*/operator=(const BitVec&);
	void /*BitSlice::*/operator=(const BitPick&);
	void /*BitSlice::*/operator=(const BitSlct&);
	void /*BitSlice::*/operator=(const BitSlice&);
	void /*BitSlice::*/operator=(bool);
	void /*BitSlice::*/lengthErr(const BitVec&);
	void /*BitSlice::*/lengthErr(const BitSlice&);
	void /*BitSlice::*/lengthErr(const IntVec&);
	void selectErr(const BitVec&);
friend	BitVec	operator!(const BitSlice&);
friend	BitVec	operator&(const BitSlice&,const BitSlice&);
friend	BitVec	operator^(const BitSlice&,const BitSlice&);
friend	BitVec	operator|(const BitSlice&,const BitSlice&);
friend	void	operator&=(BitSlice&,const BitSlice&);
friend	void	operator^=(BitSlice&,const BitSlice&);
friend	void	operator|=(BitSlice&,const BitSlice&);
friend	BitVec	reverse(const BitSlice&);
friend	int	sum(const BitSlice&);
};

class BitPick {
	BitVec* V;
	IntVec* X;
	BitPick(BitVec& v,const IntVec& x)	{ V = &v;  X = &x; }
	friend BitVec;
	friend BitSlice;
	friend BitSlct;
	friend BitPickIstream;
	friend BitPickOstream;
public:
	void /*BitPick::*/operator=(const BitVec&);
	void /*BitPick::*/operator=(const BitPick&);
	void /*BitPick::*/operator=(const BitSlct&);
	void /*BitPick::*/operator=(const BitSlice&);
	void /*BitPick::*/operator=(bool);
	unsigned length();
};

class BitSlct {
	BitVec* V;
	BitVec* B;
	BitSlct(BitVec& v, const BitVec& b)	{ V = &v;  B = &b; }
	friend BitVec;
	friend BitSlice;
	friend BitPick;
public:
	void /*BitSlct::*/operator=(const BitVec&);
	void /*BitSlct::*/operator=(const BitPick&);
	void /*BitSlct::*/operator=(const BitSlct&);
	void /*BitSlct::*/operator=(const BitSlice&);
	void /*BitSlct::*/operator=(bool);
	unsigned length()	{ return B->length(); }
};

static /*inline*/ BitSlice BitVec::operator()(int pos, unsigned lgt, int stride) // static due to cfront bug
{
	BitSlice s(*this,pos,lgt,stride);
	return s;
}

static /*inline*/ BitVec::operator BitSlice() // static due to cfront bug
{
	BitSlice s(*this,length());
	return s;
}

static /*inline*/ BitPick BitVec::operator[](const IntVec& I) // static due to cfront bug
{
	return BitPick(*this,I);
}

static /*inline*/ BitSlct BitVec::operator[](const BitVec& B) // static due to cfront bug
{
	return BitSlct(*this,B);
}

#endif

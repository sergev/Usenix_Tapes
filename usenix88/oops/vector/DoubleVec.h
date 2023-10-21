#ifndef	DOUBLEVEC_H
#define	DOUBLEVEC_H

/* DoubleVec.h -- Double Precision Vectors

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

Function:
	
Modification History:

$Log:	DoubleVec.h,v $
 * Revision 1.1  88/01/17  09:47:05  keith
 * Initial revision
 * 
	
*/
#include "Vector.h"
#include "BitVec.h"
#include "IntVec.h"

class DoubleSlice;
class DoublePick;
class DoubleSlct;

extern const Class class_DoubleVec;
overload DoubleVec_reader;

class DoubleVec : public Vector {
	double* v;	// pointer to data, NULL if empty vector
	void indexRangeErr();
protected:
	DoubleVec(fileDescTy&,DoubleVec&);
	DoubleVec(istream&,DoubleVec&);
	friend	void DoubleVec_reader(istream& strm, Object& where);
	friend	void DoubleVec_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	DoubleVec(register unsigned len =0);
	DoubleVec(register unsigned len, double from, double by =1.0);
	DoubleVec(const double*, unsigned len);
	DoubleVec(const DoubleVec&);
	DoubleVec(const DoubleSlice&);
	~DoubleVec()		{ delete v; }
	DoubleSlice operator()(int pos, unsigned lgt, int stride =1);
	operator double*()	{ return v; }
	operator DoubleSlice();
//	operator ComplexVec();
	double&		operator[](int i) {	// vector element
		if ((unsigned)i >= n) indexRangeErr();
		return v[i];
	}
	double&		operator()(int i)		{ return v[i]; }
	DoublePick	operator[](const IntVec&);
	DoubleSlct	operator[](const BitVec&);
	void /*DoubleVec::*/operator=(const DoubleVec&);
	void /*DoubleVec::*/operator=(const DoubleSlice&);
	void /*DoubleVec::*/operator=(const DoubleSlct&);
	void /*DoubleVec::*/operator=(const DoublePick&);
	void /*DoubleVec::*/operator=(double);
	void /*DoubleVec::*/lengthErr(const DoubleSlice&);
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

class TempDoubleVec : public DoubleVec {
	friend DoubleSlice;
	friend DoublePick;
	friend DoubleSlct;
	TempDoubleVec(register unsigned len =0) : (len) {}
	virtual void free();
};

class DoubleSlice {
	DoubleVec* V;	// vector pointer
	double* p;	// slice pointer
	unsigned l;	// slice length
	int k;		// slice stride
	DoubleSlice(DoubleVec& v, int pos, unsigned lgt, int stride =1);
	DoubleSlice(DoubleVec& v, unsigned lgt) {
		V = &v;  p = v;  l = lgt;  k = 1;
	}
	friend DoubleVec;
public:
	DoubleSlice(const DoublePick&);
	DoubleSlice(const DoubleSlct&);
	~DoubleSlice()		{ V->free(); }
	operator double*()	{ return p; }
	unsigned length()	{ return l; }
	int stride()		{ return k; }
	void /*DoubleSlice::*/operator=(const DoubleVec&);
	void /*DoubleSlice::*/operator=(const DoublePick&);
	void /*DoubleSlice::*/operator=(const DoubleSlct&);
	void /*DoubleSlice::*/operator=(const DoubleSlice&);
	void /*DoubleSlice::*/operator=(double);
	void /*DoubleSlice::*/lengthErr(const DoubleVec&);
	void /*DoubleSlice::*/lengthErr(const DoubleSlice&);
	void /*DoubleSlice::*/lengthErr(const IntVec&);
	void selectErr(const BitVec&);
friend	DoubleVec	operator-(const DoubleSlice&);
friend	DoubleVec	operator++(DoubleSlice&);
friend	DoubleVec	operator--(DoubleSlice&);
friend	DoubleVec	operator*(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	operator/(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	operator+(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	operator-(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	operator*(const DoubleSlice&,double);
friend	DoubleVec	operator*(double s,const DoubleSlice& V)  { return V*s; }
friend	DoubleVec	operator/(const DoubleSlice&,double);
friend	DoubleVec	operator/(double,const DoubleSlice&);
friend	DoubleVec	operator+(const DoubleSlice&,double);
friend	DoubleVec	operator+(double s,const DoubleSlice& V)  { return V+s; }
friend	DoubleVec	operator-(const DoubleSlice&,double);
friend	DoubleVec	operator-(double,const DoubleSlice&);
friend	BitVec		operator<(const DoubleSlice&,const DoubleSlice&);
friend	BitVec		operator>(const DoubleSlice& U,const DoubleSlice& V)  { return V < U; }
friend	BitVec		operator<=(const DoubleSlice&,const DoubleSlice&);
friend	BitVec		operator>=(const DoubleSlice& U,const DoubleSlice& V)  { return V <= U; }
friend	BitVec		operator==(const DoubleSlice&,const DoubleSlice&);
friend	BitVec		operator!=(const DoubleSlice&,const DoubleSlice&);
friend	BitVec		operator<(const DoubleSlice&,double);
friend	BitVec		operator<(double s,const DoubleSlice& V)  { return V>s; }
friend	BitVec		operator>(const DoubleSlice&,double);
friend	BitVec		operator>(double s,const DoubleSlice& V)  { return V<s; }
friend	BitVec		operator<=(const DoubleSlice&,double);
friend	BitVec		operator<=(double s,const DoubleSlice& V)  { return V>=s; }
friend	BitVec		operator>=(const DoubleSlice&,double);
friend	BitVec		operator>=(double s,const DoubleSlice& V)  { return V<=s; }
friend	BitVec		operator==(const DoubleSlice&,double);
friend	BitVec		operator==(double s,const DoubleSlice& V)  { return V==s; }
friend	BitVec		operator!=(const DoubleSlice&,double);
friend	BitVec		operator!=(double s,const DoubleSlice& V)  { return V!=s; }
friend	void		operator+=(DoubleSlice&,const DoubleSlice&);
friend	void		operator+=(DoubleSlice&,double);
friend	void		operator-=(DoubleSlice&,const DoubleSlice&);
friend	void		operator-=(DoubleSlice&,double);
friend	void		operator*=(DoubleSlice&,const DoubleSlice&);
friend	void		operator*=(DoubleSlice&,double);
friend	void		operator/=(DoubleSlice&,const DoubleSlice&);
friend	void		operator/=(DoubleSlice&,double);
	DoubleVec	apply(mathFunTy);
friend	DoubleVec	abs(const DoubleSlice& V);
friend	DoubleVec	acos(const DoubleSlice& V)	{ return V.apply(acos); }
friend	DoubleVec	asin(const DoubleSlice& V)	{ return V.apply(asin); }
friend	DoubleVec	atan(const DoubleSlice& V)	{ return V.apply(atan); }
friend	DoubleVec	atan2(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	ceil(const DoubleSlice& V)	{ return V.apply(ceil); }
friend	DoubleVec	cos(const DoubleSlice& V)	{ return V.apply(cos); }
friend	DoubleVec	cosh(const DoubleSlice& V)	{ return V.apply(cosh); }
friend	DoubleVec	cumsum(const DoubleSlice&);
friend	DoubleVec	delta(const DoubleSlice&);
friend	double		dot(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	exp(const DoubleSlice& V)	{ return V.apply(exp); }
friend	DoubleVec	floor(const DoubleSlice& V)	{ return V.apply(floor); }
friend	DoubleVec	log(const DoubleSlice& V)	{ return V.apply(log); }
friend	int		max(const DoubleSlice&);
friend	int		min(const DoubleSlice&);
friend	double		prod(const DoubleSlice&);
friend	DoubleVec	pow(const DoubleSlice&,const DoubleSlice&);
friend	DoubleVec	reverse(const DoubleSlice&);
friend	DoubleVec	sin(const DoubleSlice& V)	{ return V.apply(sin); }
friend	DoubleVec	sinh(const DoubleSlice& V)	{ return V.apply(sinh); }
friend	DoubleVec	sqrt(const DoubleSlice& V)	{ return V.apply(sqrt); }
friend	double		sum(const DoubleSlice&);
friend	DoubleVec	tan(const DoubleSlice& V)	{ return V.apply(tan); }
friend	DoubleVec	tanh(const DoubleSlice& V)	{ return V.apply(tanh); }
};

class DoublePick {
	DoubleVec* V;
	IntVec* X;
	DoublePick(DoubleVec& v,const IntVec& x)	{ V = &v;  X = &x; }
	friend DoubleVec;
	friend DoubleSlice;
	friend DoubleSlct;
public:
	void /*DoublePick::*/operator=(const DoubleVec&);
	void /*DoublePick::*/operator=(const DoublePick&);
	void /*DoublePick::*/operator=(const DoubleSlct&);
	void /*DoublePick::*/operator=(const DoubleSlice&);
	void /*DoublePick::*/operator=(double);
	unsigned length()	{ return X->length(); }
};

class DoubleSlct {
	BitVec* B;
	DoubleVec* V;
	DoubleSlct(DoubleVec& v, const BitVec& b)	{ V = &v;  B = &b; }
	friend DoubleVec;
	friend DoubleSlice;
	friend DoublePick;
public:
	void /*DoubleSlct::*/operator=(const DoubleVec&);
	void /*DoubleSlct::*/operator=(const DoublePick&);
	void /*DoubleSlct::*/operator=(const DoubleSlct&);
	void /*DoubleSlct::*/operator=(const DoubleSlice&);
	void /*DoubleSlct::*/operator=(double);
	unsigned length()	{ return B->length(); }
};

static /*inline*/ DoubleSlice DoubleVec::operator()(int pos, unsigned lgt, int stride) // static due to cfront bug
{
	DoubleSlice s(*this,pos,lgt,stride);
	return s;
}

static /*inline*/ DoubleVec::operator DoubleSlice() // static due to cfront bug
{
	DoubleSlice s(*this,length());
	return s;
}

static /*inline*/ DoublePick DoubleVec::operator[](const IntVec& I) // static due to cfront bug
{
	return DoublePick(*this,I);
}

static /*inline*/ DoubleSlct DoubleVec::operator[](const BitVec& B) // static due to cfront bug
{
	return DoubleSlct(*this,B);
}

#endif

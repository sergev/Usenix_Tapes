/* m4Vec.h -- M4 code templates for OOPS Vector classes

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
	
This file contains M4 macro definitions for almost all of the functions
needed to implement the OOPS vector classes.

Modification History:

$Log:	Vec.m4,v $
Revision 1.1  88/01/17  09:47:23  keith
Initial revision

	
*/
// WARNING: Modify the M4 macros, not the C++ code they generate!
// WARNING: Assumes 8 bits per character.

define(CONCAT,$1$2$3$4$5$6$7$8$9)

define(CAP,`CONCAT(translit(substr($1,0,1),abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ),substr($1,1))')

define(PROLOGUE,

``#include'' "$1Vec.h"
``#include'' "oopsIO.h"

DEFINE_CLASS($1Vec,Vector,1,"$Header: Vec.m4,v 1.1 88/01/17 09:47:23 keith Exp $",NULL,NULL);
/*
typedef float Float;
typedef double Double;
typedef unsigned char Byte;
typedef short Short;
typedef int Int;
typedef long Long;
*/
``#define''	Float	float
``#define''	Double	double
``#define''	Byte	unsigned char
``#define''	Short	short
``#define''	Int	int
``#define''	Long	long

inline void $1Copy(const $1* src, $1* dst, unsigned n)
// Copy n $1 elements from src to dst.
{
	register const $1* sp = src;
	register $1* dp = dst;
	register unsigned i = n;
	while (i--) *dp++ = *sp++;
}

void Temp$1Vec::free()		{ delete this; }

bool $1Vec::isEqual(const Object& u)
// Test $1Vecs equal.
{
	register $1Vec& U = *($1Vec*)&u;
	if (!u.isSpecies(class_$1Vec) || n!=U.n) return NO;
	register unsigned i = n;
	register $1* vp = v;
	register $1* up = U.v;
	while (i--) if (*vp++ != *up++) return NO;
	return YES;
}
									
const Class* $1Vec::species()	{ return &class_$1Vec; }

Object* $1Vec::shallowCopy()
{
	shouldNotImplement("shallowCopy");
	return 0;
}

void $1Vec::deepenShallowCopy()
// Make a deep copy of this $1Vec.
{
	if (n != 0) {
		$1* src = v;
		v = new $1[n];
		$1Copy(src,v,n);
	}
}

$1Vec::$1Vec(fileDescTy& fd, $1Vec& where) : (fd,where)
{
	this = &where;
	v = new $1[n];
	readBin(fd,v,n);
}

void $1Vec::storer(fileDescTy& fd)
{
	Vector::storer(fd);
	storeBin(fd,v,n);
}

extern const int OOPS_INDEXRANGE;
extern const int OOPS_SLICERANGE;
extern const int OOPS_VECTORLENGTH;
extern const int OOPS_VECTORSELECT;

void $1Vec::indexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}

void $1Vec::selectErr(const BitVec& V)
{
	setOOPSerror(OOPS_VECTORSELECT,DEFAULT,this,"$1Vec",length(),sum(V),&V,V.length());
}

void $1Slice::selectErr(const BitVec& V)
{
	setOOPSerror(OOPS_VECTORSELECT,DEFAULT,this,"$1Slice",length(),sum(V),&V,V.length());
}

)

define(TYPE1_lengthErr_TYPE2,
void $1::lengthErr(const $2& V)
{
	setOOPSerror(OOPS_VECTORLENGTH,DEFAULT,this,"$1",length(),&V,"$2",V.length());
}
)

define(TYPEVec_CTOR_I,
$1Vec::$1Vec(register unsigned lngth) : (lngth)
// Construct an uninitialized $1Vec of the length specified.
{
	v = NULL;
	if (lngth != 0) v = new $1[lngth];
}
)

define(TYPEVec_CTOR_I_TYPE_TYPE,
$1Vec::$1Vec(register unsigned lngth, $1 from, $1 by) : (lngth)
// Construct a $1Vec of the length specified and initialize it to
// (v[0] = from, v[i] = v[i-1]+by for i = 1...n-1)
{
	v = NULL;
	if (lngth != 0) {
		v = new $1[lngth];
		register $1* vp =v;
		for (register $1 x =from; lngth-- >0; x += by) *vp++ = x;
	}
}
)

define(TYPEVec_CTOR_TYPEPTR_I,
$1Vec::$1Vec(const $1* src, unsigned lngth) : (lngth)
// Construct a $1Vec and initialize it from the specified $1 vector.
{
	v = NULL;
	if (lngth != 0) {
		v = new $1[lngth];
		$1Copy(src,v,lngth);
	}
}
)

define(TYPEVec_CTOR_TYPEVec,
$1Vec::$1Vec(const $1Vec& U) : (U.n)
// Construct a $1Vec and initialize it from the specified $1Vec U.
{
	v = NULL;
	if (n != 0) {
		v = new $1[n];
		$1Copy(U.v,v,n);
	}
}
)

define(TYPEVec_CTOR_TYPESlice,
$1Vec::$1Vec(const $1Slice& s) : (s.length())
// Construct a $1Vec from a slice of another $1Vec.
{
	v = NULL;
	if (n != 0) {
		v = new $1[n];
		register $1* sp = s;
		register $1* dp = v;
		register i = s.length();
		register j = s.stride();
		while (i--) { *dp++ = *sp; sp += j; }
	}
}
)

define(TYPESlice_CTOR_TYPEVec_I_I_I,
$1Slice::$1Slice($1Vec& v, int pos, unsigned lgt, int stride)
// Construct a $1Slice from a $1Vec.
{
	if ((unsigned)(pos + (lgt-1)*stride) > v.length())
		setOOPSerror(OOPS_SLICERANGE,DEFAULT,&v,v.className(),v.length(),pos,lgt,stride);
	V = &v;  p = &v[pos];  l = lgt;  k = stride;
}
)

define(TYPESlice_CTOR_TYPEPick,
$1Slice::$1Slice(const $1Pick& s)
// Construct a $1Vec slice from IntVec-subscripted elements of $1Vec.
{
	$1Vec& T = *new Temp$1Vec();
	T = (*s.V)[*s.X];
	V = &T;  p = T;  l = T.length();  k = 1;
}
)

define(TYPESlice_CTOR_TYPESlct,
$1Slice::$1Slice(const $1Slct& s)
// Construct a $1Vec slice from $1Vec-selected elements of $1Vec.
{
	$1Vec& T = *new Temp$1Vec();
	T = (*s.V)[*s.B];
	V = &T;  p = T;  l = T.length();  k = 1;
}
)

define(TYPEVec_ASN_TYPEVec,
void $1Vec::operator=(const $1Vec& U)
// Assign the argument $1Vec U to this $1Vec.
{
	if (v != U.v) {
		delete v;
		v = NULL;
		if ((n = U.n) != 0) {
			v = new $1[n];
			$1Copy(U.v,v,n);
		}
	}
}
)

define(TYPEVec_ASN_TYPESlice,
void $1Vec::operator=(const $1Slice& s)
// Assign a slice of a $1Vec to this $1Vec.
{
	register i = s.length();	// slice length
	if ((n = i) == 0) {		// empty slice
		delete v;
		v = NULL;
		return;
	}
	register $1* sp = s;
	register j = s.stride();
	if (this == s.V) {	// V = V(i,j,k)
		$1* t = v;
		register $1* dp = new $1[i];
		v = dp;
		while (i--) { *dp++ = *sp; sp += j; }
		delete t;
	}
	else {
		delete v;
		register $1* dp = new $1[i];
		v = dp;
		while (i--) { *dp++ = *sp; sp += j; }
	}
}
)

define(TYPEVec_ASN_TYPESlct,
void $1Vec::operator=(const $1Slct& s)
// Assign the BitVec-selected elements of a $1Vec to this $1Vec.
{
	if ((n = sum(*s.B)) == 0) {	// return zero length result
		delete v;
		v = NULL;
		return;
	}
	register $1* sp = *s.V;
	$1* t = v;
	if (this != s.V) delete t;	// not V = V[BitVec&]
	register $1* dp = new $1[n];
	v = dp;
	BITVECSCAN(*s.B, s.length(), *dp++ = *sp;  sp++)
	if (this == s.V) delete t;	// case V = V[BitVec&]
}
)

define(TYPEVec_ASN_TYPEPick,
void $1Vec::operator=(const $1Pick& s)
// Assign the IntVec-subscripted elements of a $1Vec to this $1Vec.
{
	register unsigned i = s.length();
	unsigned l;
	if ((l = i) == 0) {	// return zero length result
		delete v;
		v = NULL;  n = 0;
		return;
	}
	$1* t = v;
	if (this != s.V) delete t;	// not V = V[IntVec&]
	register $1* dp = new $1[l];
	$1* u = dp;
	register int* xp = *s.X;
	register $1Vec& S = *s.V;
	while (i--) { *dp++ = S[*xp++]; }
	v = u;  n = l;
	if (this == s.V) delete t;	// case V = V[IntVec&]
}
)

define(TYPEVec_ASN_TYPE,
void $1Vec::operator=($1 scalar)
// Assign a scalar to all elements of this $1Vec.
{
	register unsigned i = n;
	register $1* dp = v;
	register $1 c = scalar;
	while (i--) *dp++ = c;
}
)

define(TYPESlice_ASN_TYPEVec,
void $1Slice::operator=(const $1Vec& U)
// Assign a $1Vec to this $1Vec slice.
{
	register i = l;
	if (i != U.length()) lengthErr(U);
	register $1* dp = p;
	register $1* sp = U;
	register j = k;
	while (i--) { *dp = *sp++; dp += j; }
}
)

define(TYPESlice_ASN_TYPESlice,
void $1Slice::operator=(const $1Slice& s)
// Assign a $1Vec slice to this $1Vec slice.
{
	register i = l;
	if (i != s.l) lengthErr(s);
	register $1* dp = p;
	register $1* sp = s.p;
	register dj = k;
	register sj = s.k;
	while (i--) { *dp = *sp; dp += dj; sp += sj; }
}
)

define(TYPESlice_ASN_TYPEPick,
void $1Slice::operator=(const $1Pick& s)
// Assign the IntVec-subscripted elements of a $1Vec to this $1Vec slice.
{
	register i = l;
	if (i != s.length()) lengthErr(*s.X);
	register $1* dp = p;
	register int* xp = *s.X;
	register j = k;
	register $1Vec& S = *s.V;
	while (i--) { *dp = S[*xp++]; dp += j; }
}
)

define(TYPESlice_ASN_TYPESlct,
void $1Slice::operator=(const $1Slct& s)
// Assign the BitVec-selected elements of a $1Vec to this $1Vec slice.
{
	if (l != sum(*s.B)) lengthErr(s);
	register $1* sp = *s.V;
	register $1* dp = p;
	register j = k;
	BITVECSCAN(*s.B, s.length(), { *dp = *sp;  dp += j; };  sp++)
}
)

define(TYPESlice_ASN_TYPE,
void $1Slice::operator=($1 scalar)
// Assign a scalar to all elements of this $1Vec slice.
{
	register i = l;
	register j = k;
	register $1* dp = p;
	register $1 c = scalar;
	while (i--) { *dp = c; dp += j; }
}
)

define(FRIEND_OP_TYPESlice__TYPEVec,
$1Vec operator$2(const $1Slice& s)
// Unary operator on $1Vec slice.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register j = s.stride();
	while (i--) { *dp++ = $2(*sp);  sp += j; }
	return T;
}
)

define(FRIEND_INCDECOP_TYPESlice__TYPEVec,
$1Vec operator$2($1Slice& s)
// Unary inc/dec operator on $1Vec slice.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register j = s.stride();
	while (i--) { *dp++ = $2(*sp);  sp += j; }
	return T;
}
)

define(FRIEND_TYPESlice_OP_TYPESlice__TYPEVec,
$1Vec operator$2(const $1Slice& u, const $1Slice& v)
// Binary arithmetic operator on two $1Vec slices
{
	register i = u.length();
	if (i != v.length()) u.lengthErr(v);
	$1Vec T(i);
	register $1* up = u;
	register $1* vp = v;
	register $1* dp = T;
	register uj = u.stride();
	register vj = v.stride();
	while (i--) { *dp++ = *up $2 *vp;  up += uj; vp += vj; }
	return T;
}
)

define(FRIEND_TYPESlice_OP_TYPE__TYPEVec,
$1Vec operator$2(const $1Slice& s, $1 scalar)
// Binary arithmetic operator on $1Vec slice and scalar.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register j = s.stride();
	register $1 c = scalar;
	while (i--) { *dp++ = *sp $2 c; sp += j; }
	return T;
}
)

define(FRIEND_TYPE_OP_TYPESlice__TYPEVec,
$1Vec operator$2($1 scalar, const $1Slice& s)
// Binary arithmetic operator on scalar and $1Vec slice.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register j = s.stride();
	register $1 c = scalar;
	while (i--) { *dp++ = c $2 *sp; sp += j; }
	return T;
}
)

define(FRIEND_TYPESlice_OP_TYPESlice__BitVec,
BitVec operator$2(const $1Slice& u, const $1Slice& v)
// Relational operator on two $1Vec slices.
{
	if (u.length() != v.length()) u.lengthErr(v);
	BitVec B(u.length());
	register $1* up = u;
	register $1* vp = v;
	register uj = u.stride();
	register vj = v.stride();
	BITVECGEN(B, u.length(), *up $2 *vp, up += uj; vp += vj)
	return B;
}
)

define(FRIEND_TYPESlice_OP_TYPE__BitVec,
BitVec operator$2(const $1Slice& s, $1 scalar)
// Relational operator on $1Vec slice and scalar.
{
	BitVec B(s.length());
	register $1* vp = s;
	register vj = s.stride();
	register $1 c = scalar;
	BITVECGEN(B, s.length(), *vp $2 c, vp += vj)
	return B;
}
)

define(FRIEND_TYPE_OP_TYPESlice__BitVec,
BitVec operator$2($1 scalar, const $1Slice& s)
// Relational operator on scalar and $1Vec slice.
{
	BitVec B(s.length());
	register $1* vp = s;
	register vj = s.stride();
	register $1 c = scalar;
	BITVECGEN(B, s.length(), c $2 *vp, vp += vj)
	return B;
}
)

define(FRIEND_TYPESlice_ASNOP_TYPESlice,
void operator$2($1Slice& u, const $1Slice& v)
// Assignment arithmetic operator on two $1Vec slices.
{
	register i = u.length();
	if (i != v.length()) u.lengthErr(v);
	register $1* up = u;
	register $1* vp = v;
	register uj = u.stride();
	register vj = v.stride();
	while (i--) { *up $2 *vp;  up += uj; vp += vj; }
}
)

define(FRIEND_TYPESlice_ASNOP_TYPE,
void operator$2($1Slice& s, $1 scalar)
// Assignment arithmetic operator on scalar and $1Vec slice.
{
	register i = s.length();
	register $1* dp = s;
	register j = s.stride();
	register $1 c = scalar;
	while (i--) { *dp $2 c; dp += j; }
}
)

define(TYPEPick_ASN_TYPEVec,
void $1Pick::operator=(const $1Vec& U)
// Assign $1Vec to IntVec-subscripted elements of $1Vec.
{
	register i = length();
	if (i != U.length()) lengthErr(*X,U);
	register int* xp = *X;
	register $1* up = U;
	register $1Vec& D = *V;
	while (i--) { D[*xp++] = *up++; }
}
)

define(TYPEPick_ASN_TYPEPick,
void $1Pick::operator=(const $1Pick& s)
// Assign IntVec-subscripted elements of $1Vec to IntVec-subscripted elements of $1Vec.
{
	register i = length();
	if (i != s.length()) lengthErr(*X,*s.X);
	register int* xp = *X;
	register int* yp = *s.X;
	register $1Vec& D = *V;
	register $1Vec& S = *s.V;
	while (i--) { D[*xp++] = S[*yp++]; }
}
)

define(TYPEPick_ASN_TYPESlct,
void $1Pick::operator=(const $1Slct& s)
// Assign BitVec-selected elements of $1Vec to IntVec-subscripted elements of $1Vec.
{
	if (length() != sum(*s.B)) X->selectErr(*s.B);
	register int* xp = *X;
	register $1* sp = *s.V;
	register $1Vec& D = *V;
	BITVECSCAN(*s.B, length(), D[*xp++] = *sp;  sp++)
}
)

define(TYPEPick_ASN_TYPESlice,
void $1Pick::operator=(const $1Slice& s)
// Assign $1Vec slice to IntVec-subscripted elements of $1Vec.
{
	register i = length();
	if (i != s.length()) s.lengthErr(*X);
	register int* xp = *X;
	register $1* sp = s;
	register int j = s.stride();
	register $1Vec& D = *V;
	while (i--) { D[*xp++] = *sp;  sp += j; }
}
)

define(TYPEPick_ASN_TYPE,
void $1Pick::operator=($1 scalar)
// Assign scalar to all IntVec-subscripted elements of $1Vec.
{
	register i = length();
	register int* xp = *X;
	register $1 c = scalar;
	register $1Vec& D = *V;
	while (i--) { D[*xp++] = c; }
}
)

define(TYPESlct_ASN_TYPEVec,
void $1Slct::operator=(const $1Vec& U)
// Assign a $1Vec to BitVec-selected elements of $1Vec.
{
	if (U.length() != sum(*B)) U.selectErr(*B);
	register $1* dp = *V;
	register $1* sp = U;
	BITVECSCAN(*B, U.length(), *dp = *sp++;  dp++)
}
)

define(TYPESlct_ASN_TYPEPick,
void $1Slct::operator=(const $1Pick& s)
// Assign IntVec-subscripted elements of $1Vec to BitVec-selected elements of $1Vec.
{
	if (s.length() != sum(*B)) s.X->selectErr(*B);
	register $1* dp = *V;
	register int* xp = *s.X;
	register $1Vec& W = *s.V;
	BITVECSCAN(*B, length(), *dp = W[*xp++];  dp++)
}
)

define(TYPESlct_ASN_TYPESlct,
void $1Slct::operator=(const $1Slct& s)
// Assign BitVec-selected elements of $1Vec to BitVec-selected elements of $1Vec.
{
	$1Vec T;
	T = (*s.V)[*s.B];
	(*V)[*B] = T;
}
)

define(TYPESlct_ASN_TYPESlice,
void $1Slct::operator=(const $1Slice& s)
// Assign $1Vec slice to BitVec-selected elements of $1Vec.
{
	if (s.length() != sum(*B)) s.selectErr(*B);
	register $1* dp = *V;
	register $1* sp = s;
	register int j = s.stride();
	BITVECSCAN(*B, length(), { *dp = *sp;  sp += j; };  dp++)
}
)

define(TYPESlct_ASN_TYPE,
void $1Slct::operator=($1 scalar)
// Assign scalar to all BitVec-selected elements of $1Vec.
{
	register $1* dp = *V;
	register $1 c = scalar;
	BITVECSCAN(*B, length(), *dp = c;  dp++)
}
)

define(TYPESlice_APPLY_FUN__TYPEVec,
$1Vec $1Slice::apply(mathFunTy f)
// Apply function to each element of vector slice
{
	register i = l;
	$1Vec T(i);
	register $1* sp = p;
	register $1* dp = T;
	register j = k;
	while (i--) { *dp++ = f(*sp);  sp += j; }
	return T;
}
)

define(FRIEND_abs_TYPESlice,
$1Vec abs(const $1Slice& s)
// Absolute value of $1Vec slice.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register j = s.stride();
	while (i--) { *dp++ = ABS(*sp);  sp += j; }
	return T;
}
)

define(FRIEND_atan2_TYPESlice_TYPESlice,
$1Vec atan2(const $1Slice& u,const $1Slice& v)
// Arctangent of u/v.
{
	register i = u.length();
	if (i != v.length()) u.lengthErr(v);
	$1Vec T(i);
	register $1* up = u;
	register $1* vp = v;
	register $1* dp = T;
	register uj = u.stride();
	register vj = v.stride();
	while (i--) { *dp++ = $2(*up,*vp);  up += uj;  vp += vj; }
	return T;
}
)

define(FRIEND_pow_TYPESlice_TYPESlice,
$1Vec pow(const $1Slice& u,const $1Slice& v)
// u to the v power.
{
	register i = u.length();
	if (i != v.length()) u.lengthErr(v);
	$1Vec T(i);
	register $1* up = u;
	register $1* vp = v;
	register $1* dp = T;
	register uj = u.stride();
	register vj = v.stride();
	while (i--) { *dp++ = $2(*up,*vp);  up += uj;  vp += vj; }
	return T;
}
)

define(FRIEND_cumsum_TYPESlice,
$1Vec cumsum(const $1Slice& s)
// Cumulative sum of $1Vec slice.
// Note: V == delta(cumsum(V)) == cumsum(delta(V))
{
	unsigned i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register $1 c = 0;
	register j = s.stride();
	while (i--) { *dp++ = c += *sp;  sp += j; }
	return T;
}
)

define(FRIEND_delta_TYPESlice,
$1Vec delta(const $1Slice& s)
// Element differences of $1Vec slice.
// Note: V == delta(cumsum(V)) == cumsum(delta(V))
{
	unsigned i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = T;
	register $1 c = 0;
	register j = s.stride();
	while (i--) { *dp++ = *sp - c;  c = *sp;  sp += j; }
	return T;
}
)

define(FRIEND_dot_TYPESlice_TYPESlice,
$1 dot(const $1Slice& u,const $1Slice& v)
// Vector dot product.
{
	register i = u.length();
	if (i != v.length()) u.lengthErr(v);
	register $1 t =0;
	register $1* up = u;
	register $1* vp = v;
	register uj = u.stride();
	register vj = v.stride();
	while (i--) { t += *up * *vp;  up += uj;  vp += vj; }
	return t;
}
)

define(FRIEND_max_TYPESlice,
int max(const $1Slice& s)
// Index of maximum element.
{
	if (s.length() == 0) s.V->emptyErr("max");
	register $1* sp = s;
	register $1 t = *sp;
	register j = s.stride();
	register x = 0;
	for (register i =0; i<s.length(); i++) {
		if (*sp > t) { t = *sp;  x = i; }
		sp += j;
	}
	return x;
}
)

define(FRIEND_min_TYPESlice,
int min(const $1Slice& s)
// Index of minimum element.
{
	if (s.length() == 0) s.V->emptyErr("min");
	register $1* sp = s;
	register $1 t = *sp;
	register j = s.stride();
	register x = 0;
	for (register i =0; i<s.length(); i++) {
		if (*sp < t) { t = *sp;  x = i; }
		sp += j;
	}
	return x;
}
)

define(FRIEND_prod_TYPESlice,
$1 prod(const $1Slice& s)
// Product of $1Vec slice elements.
{
	register i = s.length();
	register $1* sp = s;
	register $1 t = 1;
	register j = s.stride();
	while (i--) { t *= *sp;  sp += j; }
	return t;
}
)

define(FRIEND_reverse_TYPESlice,
$1Vec reverse(const $1Slice& s)
// Reverse vector elements.
{
	register i = s.length();
	$1Vec T(i);
	register $1* sp = s;
	register $1* dp = &T(i);
	register j = s.stride();
	while (i--) { *(--dp) = *sp;  sp += j; }
	return T;
}
)

define(FRIEND_sum_TYPESlice,
$1 sum(const $1Slice& s)
// Sum of vector elements.
{
	register i = s.length();
	register $1* sp = s;
	register $1 t = 0;
	register j = s.stride();
	while (i--) { t += *sp;  sp += j; }
	return t;
}
)

define(FRIEND_acos_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_asin_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_atan_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_ceil_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_cos_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_cosh_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_exp_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_floor_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_log_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_sin_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_sinh_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_sqrt_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_tan_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })
define(FRIEND_tanh_TYPESlice,$1Vec $2(const $1Slice& V) { return V.apply($2); })

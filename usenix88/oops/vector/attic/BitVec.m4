/* m4Vec.h -- M4 code templates for OOPS BitVec class

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
	June, 1986

Function:
	
This file contains M4 macro definitions for some of the functions
needed to implement the OOPS bit vector class.

Modification History:
	
WARNING: Modify the M4 macros, not the C++ code they generate!

WARNING: Assumes 8 bits per character.

*/

`#include' "IntVec.h"
`#include' "bitstreams.h"
`#include' "oopsconfig.h"
`#include' "oopsIO.h"

`#define'	THIS	BitVec
`#define'	BASE	Vector
DEFINE_CLASS(BitVec,Vector,1,NULL,NULL);

extern const unsigned char char_bit_mask[sizeof(char)];
extern const unsigned char bit_count[256];

extern const int OOPS_INDEXRANGE;
extern const int OOPS_SLICERANGE;
extern const int OOPS_VECTORLENGTH;
extern const int OOPS_VECTORSELECT;

inline unsigned nbytes(unsigned n)	{ return (n+7) >> 3; }

inline void byteCopy(const bitVecByte* src, bitVecByte* dst, unsigned n)
// Copy n bytes from src to dst.
{
	register const bitVecByte* sp = src;
	register bitVecByte* dp = dst;
	register unsigned i = n;
	while (i--) *dp++ = *sp++;
}

static union hash_byte_mask {
	unsigned in[sizeof(int)];
	char ch[sizeof(int)*sizeof(int)];
	hash_byte_mask();
} mask;

static hash_byte_mask::hash_byte_mask()
{
	for (register unsigned i=0; i<sizeof(int); i++) {
		for (register unsigned j=0; j<sizeof(int); j++) ch[sizeof(int)*i+j] = j<i ? 0xff : 0;
	}
}

unsigned THIS::hash()
{
	register unsigned h = nbytes();
`#ifdef' LOG2_INT
	register unsigned i = nbytes() >> LOG2_INT;
`#else'
	register unsigned i = nbytes()/sizeof(int);
`#endif'
	register unsigned* vp = (unsigned*)v;
	while (i--) h ^= *vp++;
`#ifdef' LOG2_INT
	if ((i = nbytes()&(sizeof(int)-1)) != 0)
`#else'
	if ((i = nbytes()%sizeof(int)) != 0)
`#endif'
		h ^= *vp & mask.in[i];
	return h;
}

void THIS::printOn(ostream& strm)
{
	for (register unsigned i=0; i<length(); i++) {
		if (i>0 && (i%25 == 0)) strm << "\n\t";
		strm << ((*this)(i) ? "1 " : "0 ");
	}
}

void THIS::scanFrom(istream& strm)
{
	extern const int OOPS_NYET;
	setOOPSerror(OOPS_NYET,DEFAULT,className(),"scanFrom");
}

THIS::THIS(istream& strm, THIS& where) : (strm,where)
{
	this = &where;
	v = new bitVecByte[nbytes()];
	for (register unsigned i =0; i<nbytes(); i++) { int _i; strm >> _i; v[i] = _i; }
}

void THIS::storer(ostream& strm)
{
	BASE::storer(strm);
	for (register unsigned i=0; i<nbytes(); i++) strm << (unsigned int)(unsigned char)v[i] << " ";
}

THIS::THIS(fileDescTy& fd, THIS& where) : (fd,where)
{
	this = &where;
	v = new bitVecByte[nbytes()];
	readBin(fd,v,nbytes());
}

void THIS::storer(fileDescTy& fd)
{
	BASE::storer(fd);
	storeBin(fd,v,nbytes());
}

bool BitVec::isEqual(const Object& u)
// Test for two BitVecs equal.
{
	register BitVec& U = *(BitVec*)&u;
	if (!u.isSpecies(class_BitVec) || n!=U.n) return NO;
	register unsigned i = nbytes();
	register bitVecByte* vp = v;
	register bitVecByte* up = U.v;
	while (i--) if (*vp++ != *up++) return NO;
	return YES;
}
									
const Class* BitVec::species()	{ return &class_BitVec; }

Object* BitVec::shallowCopy()
{
	shouldNotImplement("shallowCopy");
	return 0;
}

void BitVec::deepenShallowCopy()
// Make a deep copy of a BitVec.
{
	unsigned l = nbytes();
	if (l != 0) {
		bitVecByte* src = v;
		v = new bitVecByte[l];
		byteCopy(src,v,l);
	}
}

void BitVec::indexRangeErr()
{
	setOOPSerror(OOPS_INDEXRANGE,DEFAULT,this,className());
}

void BitVec::selectErr(const BitVec& V)
{
	setOOPSerror(OOPS_VECTORSELECT,DEFAULT,this,"BitVec",length(),sum(V),&V,V.length());
}

void BitSlice::selectErr(const BitVec& V)
{
	setOOPSerror(OOPS_VECTORSELECT,DEFAULT,this,"BitSlice",length(),sum(V),&V,V.length());
}

BitVec::BitVec(register unsigned lngth) : (lngth)
// Construct an uninitialized BitVec of the length specified.
{
	v = NULL;
	if (lngth != 0) v = new bitVecByte[nbytes()];
}

BitVec::BitVec(register unsigned lngth, bool init) : (lngth)
// Construct a BitVec of the length specified and initialize it.
{
	v = NULL;
	if ((lngth) != 0) {
		v = new bitVecByte[nbytes()];
		*this = init;
	}
}

BitVec::BitVec(const bitVecByte* src, unsigned lngth) : (lngth)
// Construct a BitVec and initialize it from the specified byte vector.
{
	v = NULL;
	if (lngth != 0) {
		register l = nbytes();
		v = new bitVecByte[l];
		byteCopy(src,v,l);
	}
}

BitVec::BitVec(const BitVec& U) : (U.n)
// Construct a BitVec and initialize it from the specified BitVec U.
{
	v = NULL;
	if (n != 0) {
		register l = nbytes();
		v = new bitVecByte[l];
		byteCopy(U.v,v,l);
	}
}

BitVec::BitVec(const BitSlice& s) : (0)
// Construct a BitVec from a slice of another BitVec.
{
	v = NULL;
	*this = s;
}

BitSlice::BitSlice(BitVec& v, int pos, unsigned lgt, int stride)
// Construct a BitSlice from a BitVec.
{
	if ((unsigned)(pos + (lgt-1)*stride) > v.length())
		setOOPSerror(OOPS_SLICERANGE,DEFAULT,&v,v.className(),v.length(),pos,lgt,stride);
	V = &v;  p = pos;  l = lgt;  k = stride;
}

BitSlice::BitSlice(const BitSlct& s)
// Construct a BitVec slice from IntVec-subscripted elements of BitVec
{
	BitVec& T = *new TempBitVec();
	T = (*s.V)[*s.B];
	V = &T;  p = 0;  l = T.length();  k = 1;
}

BitSlice::BitSlice(const BitPick& s)
// Construct a BitVec slice from IntVec-subscripted elements of BitVec
{
	BitVec& T = *new TempBitVec();
	T = (*s.V)[*s.X];
	V = &T;  p = 0;  l = T.length();  k = 1;
}

void BitVec::operator=(const BitVec& U)
// Assign the argument BitVec U to this BitVec.
{
	if (v != U.v) {
		delete v;
		v = NULL;
		if ((n = U.n) != 0) {
			register l = nbytes();
			v = new bitVecByte[l];
			byteCopy(U,v,l);
		}
	}
}

void BitVec::operator=(const BitSlice& s)
// Assign a slice of a BitVec to this BitVec.
{
	if ((n = s.length()) == 0) {		// empty slice
		delete v;
		v = NULL;
		return;
	}
	bitVecByte* t = v;
	if (this != s.V) delete t;	// not V = V(i,j,k)
	bitVecByte* u = new bitVecByte[nbytes()];
	BitSliceIstream src(s);
	BITVECGEN(u, length(), src)
	v = u;
	if (this == s.V) delete t;	// case V = V[BitVec&]
}

void BitVec::operator=(const BitSlct& s)
// Assign the BitVec-selected elements of a BitVec to this BitVec.
{
	if ((n = sum(*s.B)) == 0) {	// return zero length result
		delete v;
		v = NULL;
		return;
	}
	BitVecIstream src(*s.V);
	BitVecIstream slct(*s.B);
	bitVecByte* t = v;
	if (this != s.V) delete t;	// not V = V[BitVec&]
	v = new bitVecByte[nbytes()];
	BitVecOstream dst(*this);
	register unsigned i = s.length();
	while(i--) {
		if (slct) dst << src;
		slct++;
		src++;
	}
	if (this == s.V) delete t;	// case V = V[BitVec&]
}

void BitVec::operator=(const BitPick& s)
// Assign the IntVec-subscripted elements of a BitVec to this BitVec.
{
	unsigned l = s.length();
	if (l == 0) {		// empty IntVec
		delete v;
		n = 0;  v = NULL;
		return;
	}
	bitVecByte* t = v;
	if (this != s.V) delete t;	// not V = V[IntVec]
	bitVecByte* u = new bitVecByte[::nbytes(l)];
	BitPickIstream src(s);
	BITVECGEN(u, l, src)
	n = l;  v = u;
	if (this == s.V) delete t;	// case V = V[IntVec&]
}

void BitVec::operator=(bool scalar)
// Assign a scalar Boolean to all elements of this BitVec.
{
	register i = nbytes();
	register bitVecByte* dp = v;
	if (i>0) {
		if (scalar) {
			while (--i) *dp++ = 0xff;
			*dp = char_bit_mask[n&7]-1;
		}
		else while (i--) *dp++ = 0;
	}
}

BitVec operator!(const BitVec& U)
// Unary bitwise operator on a BitVec.
{
	register i = U.nbytes();
	BitVec T(U.length());
	register bitVecByte* dp = T;
	register bitVecByte* up = U;
	while (i--) *dp++ = ~(*up++);
	if ((i = U.length()&7) != 0) {
		--dp;
		*dp &= char_bit_mask[i]-1;
	}
	return T;
}

void BitSlice::operator=(const BitVec& U)
// Assign a BitVec to this BitVec slice.
{
	if (length() != U.length()) lengthErr(U);
	BitSliceOstream dst(*this);
	register bitVecByte m;
	register bitVecByte* bp = U;
	register unsigned i = length() >> 3;
	while (i--) {
		m = *bp++;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);
	}
	i = length() & 7;
	m = *bp;
	while (i--) {
		dst << (m&1);  m >>= 1;
	}
}

void BitSlice::operator=(const BitSlice& s)
// Assign a BitVec slice to this BitVec slice.
{
	register unsigned i = length();
	if (i != s.length()) lengthErr(s);
	BitSliceIstream src(s);
	BitSliceOstream dst(*this);
	while (i--) dst << src;
}

void BitSlice::operator=(const BitPick& s)
// Assign the IntVec-subscripted elements of a BitVec to this BitVec slice.
{
	register i = length();
	if (i != s.length()) lengthErr(*s.X);
	BitSliceOstream dst(*this);
	BitPickIstream src(s);
	while (i--) dst << src;
}

void BitSlice::operator=(const BitSlct& s)
// Assign the BitVec-selected elements of a BitVec to this BitVec slice.
{
	if (l != sum(*s.B)) selectErr(*s.B);
	BitVecIstream src(*s.V);
	BitVecIstream slct(*s.B);
	BitSliceOstream dst(*this);
	register unsigned i = s.length();
	while(i--) {
		if (slct) { dst << src; }
		slct++;
		src++;
	}
}

void BitSlice::operator=(bool scalar)
// Assign a scalar to all elements of this BitVec slice.
{
	register i = length();
	register j = stride();
	register x = pos();
	register BitVec& D = *V;
	register bool c = scalar;
	while (i--) { D(x) = c; x += j; }
}

BitVec operator!(const BitSlice& s)
// Unary operator on BitVec slice.
{
	BitVec T(s.length());
	BitSliceIstream src(s);
	BITVECGEN(T, s.length(), !src)
	return T;
}

void BitPick::operator=(const BitVec& U)
// Assign a BitVec to the IntVec-subscripted elements of this BitVec.
{
	if (length() != U.length()) lengthErr(*X,U);
	BitPickOstream dst(*this);
	register bitVecByte m;
	register bitVecByte* bp = U;
	register unsigned i = length() >> 3;
	while (i--) {
		m = *bp++;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
		dst << (m&1);  m >>= 1;
	}
	i = length() & 7;
	m = *bp;
	while (i--) {
		dst << (m&1);  m >>= 1;
	}
}

void BitPick::operator=(const BitPick& s)
// Assign the IntVec-subscripted elements of a BitVec to the IntVec-subscripted elements of this BitVec.
{
	register i = length();
	if (i != s.length()) lengthErr(*X,*s.X);
	BitPickIstream src(s);
	BitPickOstream dst(*this);
	while (i--) dst << src;
}

void BitPick::operator=(const BitSlct& s)
// Assign the BitVec-selected elements of a BitVec to the IntVec-subscripted elements of this BitVec.
{
	if (length() != sum(*s.B)) X->selectErr(*s.B);
	BitVecIstream src(*s.V);
	BitVecIstream slct(*s.B);
	BitPickOstream dst(*this);
	register unsigned i = s.length();
	while(i--) {
		if (slct) { dst << src; }
		slct++;
		src++;
	}
}

void BitPick::operator=(const BitSlice& s)
// Assign a BitVec slice to the IntVec-subscripted elements of this BitVec.
{
	register i = length();
	if (i != s.length()) s.lengthErr(*X);
	BitSliceIstream src(s);
	BitPickOstream dst(*this);
	while (i--) dst << src;
}

void BitPick::operator=(bool scalar)
// Assign a scalar to the IntVec-subscripted elements of this BitVec.
{
	register i = length();
	register bool c = scalar;
	BitPickOstream dst(*this);
	while (i--) { dst << c; }
}

void BitSlct::operator=(const BitVec& U)
// Assign a BitVec to the BitVec-selected elements of this BitVec.
{
	if (U.length() != sum(*B)) U.selectErr(*B);
	BitVecIstream src(U);
	BitVecOstream dst(*V);
	BITVECSCAN(*B, length(), { dst << src;  src++; }  else dst++)
}

void BitSlct::operator=(const BitPick& s)
// Assign the IntVec-subscripted elements of a BitVec to the BitVec-selected elements of this BitVec.
{
	if (s.length() != sum(*B)) s.X->selectErr(*B);
	BitPickIstream src(s);
	BitVecOstream dst(*V);
	BITVECSCAN(*B, length(), dst << src;  else dst++)
}

void BitSlct::operator=(const BitSlct& s)
// Assign the BitVec-selected elements of a BitVec to the BitVec-selected elements of this BitVec.
{
	BitVec T;
	T = (*s.V)[*s.B];
	(*V)[*B] = T;
}

void BitSlct::operator=(const BitSlice& s)
// Assign a BitVec slice to the BitVec-selected elements of this BitVec.
{
	if (s.length() != sum(*B)) s.selectErr(*B);
	BitSliceIstream src(s);
	BitVecOstream dst(*V);
	BITVECSCAN(*B, length(), dst << src;  else dst++)
}

void BitSlct::operator=(bool scalar)
// Assign a scalar to the BitVec-selected elements of this BitVec.
{
	BitVecOstream dst(*V);
	BITVECSCAN(*B, length(), dst << scalar;  else dst++)
}

BitVec reverse(const BitSlice& s)
// Reverse bit order of BitSlice.
{
	register i = s.length();
	BitVec T(i);
	register k = s.pos();
	register j = s.stride();
	register BitVec& S = *s.V;
	while (i--) { T(i) = S[k];  k += j; }
	return T;
}

int sum(const BitVec& U)
// Count of ones in the specified BitVec.
{
	register i = U.nbytes();
	register bitVecByte* sp = U;
	register int t = 0;
	while (i--) t += bit_count[*sp++];
	return t;
}

int sum(const BitSlice& s)
// Count of ones in the specified BitSlice.
{
	register i = s.length();
	register int t = 0;
	register k = s.pos();
	register j = s.stride();
	register BitVec& S = *s.V;
	while (i--) { if (S[k]) t++;  k += j; }
	return t;
}

define(CONCAT,$1$2$3$4$5$6$7$8$9)

define(CAP,`CONCAT(translit(substr($1,0,1),abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ),substr($1,1))')

define(TYPE1_lengthErr_TYPE2,
void $1::lengthErr(const $2& V)
{
	setOOPSerror(OOPS_VECTORLENGTH,DEFAULT,this,"$1",length(),&V,"$2",V.length());
}
)

define(FRIEND_BitVec_OP_BitVec__BitVec,
BitVec operator$2(const BitVec& U, const BitVec& V)
// Binary bitwise operator on two BitVecs.
{
	if (U.length() != V.length()) lengthErr(U,V);
	register i = U.nbytes();
	BitVec T(U.length());
	register bitVecByte* dp = T;
	register bitVecByte* up = U;
	register bitVecByte* vp = V;
	while (i--) *dp++ = *up++ $2 *vp++;
	return T;
}
)

define(BitVec_ASNOP_BitVec,
void BitVec::operator$1(const BitVec& V)
// Binary bitwise assignment operator on two BitVecs.
{
	if (length() != V.length()) ::lengthErr(*this,V);
	register i = nbytes();
	register bitVecByte* up = *this;
	register bitVecByte* vp = V;
	while (i--) *up++ $1 *vp++;
}
)

define(FRIEND_BitSlice_OP_BitSlice__BitVec,
BitVec operator$2(const BitSlice& u, const BitSlice& v)
// Binary arithmetic operator on two BitVec slices
{
	if (u.length() != v.length()) u.lengthErr(v);
	BitVec T(u.length());
	BitSliceIstream usrc(u);
	BitSliceIstream vsrc(v);
	BITVECGEN(T, u.length(), (bool)usrc $2 (bool)vsrc)
	return T;
}
)

define(BitSlice_ASNOP_BitSlice,
void BitSlice::operator$1(const BitSlice& v)
// Assignment arithmetic operator on two BitVec slices.
{
	register unsigned i;
	if ((i=length()) != v.length()) lengthErr(v);
	register BitVec& U = *V;
	register BitVec& V = *v.V;
	register uj = pos();
	register vj = v.pos();
	register uk = stride();
	register vk = v.stride();
	while (i--) {
		U[uj] $1 V[vj];
		uj += uk;  vj += vk;
	}
}
)

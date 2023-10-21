#ifndef	BITSTREAMS_H
#define	BITSTREAMS_H

/* bitstreams.h -- Utility functions for manipulating bit streams

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

$Log:	bitstreams.h,v $
 * Revision 1.1  88/01/17  09:47:13  keith
 * Initial revision
 * 
	
*/
#include "BitVec.h"

class BitVecIstream {
	bitVecByte* p;		// pointer to current byte
	unsigned i;		// current bit index
public:
	BitVecIstream(const BitVec& B)	{ p = B;  i = 0; }
	operator bool() 	{ return (p[i>>3] >> (i&7)) & 1; }
	void operator++()	{ i++; }
};

class BitVecOstream {
	bitVecByte* p;		// pointer to current byte
	unsigned i;		// current bit index
	BitVecOstream(BitVec& B)	{ p = B;  B = 0;  i = 0; }
	void operator<<(bool b) { if (b) p[i>>3] |= 1 << (i&7);  i++; }
	void operator++()	{ i++; }
	friend BitVec;
	friend BitSlice;
	friend BitPick;
	friend BitSlct;
};

class BitSliceIstream {
	bitVecByte* p;		// pointer to current byte
	unsigned i;		// current bit index
	int k;			// stride
public:
	BitSliceIstream(const BitSlice& s)	{ p = *s.V;  i = s.pos();  k = s.stride(); }
	operator bool()	{ 
		register bool b = (p[i>>3] >> (i&7)) & 1;
		i += k;
		return b;
	}
};

class BitSliceOstream {
	bitVecByte* p;		// pointer to current byte
	unsigned i;		// current bit index
	int k;			// stride
	BitSliceOstream(BitSlice& s)	{ p = *s.V;  i = s.pos();  k = s.stride(); }
	void operator<<(bool b) {
		register bitVecByte* q = &p[i>>3];
		register bitVecByte m = 1 << (i&7);
		if (b) *q |= m;
		else *q &= ~m;
		i += k;
	}
	friend BitVec;
	friend BitSlice;
	friend BitPick;
	friend BitSlct;
};

class BitPickIstream {
	BitVec* V;		// pointer to input BitVec
	int* p;			// pointer to current index element
public:
	BitPickIstream(const BitPick& s)	{ V = s.V;  p = *s.X; }
	operator bool() 	{ return (bool)(*V)[*p++]; } // (bool) needed due to cfront bug
};

class BitPickOstream {
	BitVec* V;		// pointer to input BitVec
	int* p;			// pointer to current index element
	BitPickOstream(BitPick& s)	{ V = s.V;  p = *s.X; }
	void operator<<(bool b) { (*V)[*p++] = b; }
	friend BitVec;
	friend BitSlice;
	friend BitPick;
	friend BitSlct;
};

#endif

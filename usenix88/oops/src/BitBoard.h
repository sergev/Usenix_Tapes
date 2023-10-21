#ifndef	BITBOARD_H
#define	BITBOARD_H

/* BitBoard.h -- Declarations for class BitBoard

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

$Log:	BitBoard.h,v $
 * Revision 1.3  88/02/04  12:54:59  keith
 * Make Class class_CLASSNAME const.
 * 
 * Revision 1.2  88/01/16  23:37:58  keith
 * Remove pre-RCS modification history
 * 
*/

#include "Object.h"

extern const Class class_BitBoard;
overload BitBoard_reader;

class BitBoard: public Object {
	union {
		unsigned long m[2];
		unsigned char c[8];
	};
protected:
	BitBoard(fileDescTy&,BitBoard&);
	BitBoard(istream&,BitBoard&);
	friend	void BitBoard_reader(istream& strm, Object& where);
	friend	void BitBoard_reader(fileDescTy& fd, Object& where);
	virtual void storer(fileDescTy&);
	virtual void storer(ostream&);
public:
	BitBoard()			{}
	BitBoard(unsigned i);
	BitBoard(unsigned long i, unsigned long j) { m[0] = i;  m[1] = j; }
	BitBoard operator~()		{ return BitBoard(~m[0], ~m[1]); }
	BitBoard operator-(BitBoard n)	{ return BitBoard(m[0]&~n.m[0], m[1]&~n.m[1]); }
	bool operator>(BitBoard n)	{ return m[0] == (m[0]|n.m[0])
						&& m[0] != n.m[0]
						&& m[1] == (m[1]|n.m[1])
						&& m[1] != n.m[1]; }
	bool operator<(BitBoard n)	{ return n > *this; }
	bool operator>=(BitBoard n)	{ return m[0] == (m[0]|n.m[0])
						&& m[1] == (m[1]|n.m[1]); }
	bool operator<=(BitBoard n)	{ return n >= *this; }
	bool operator==(BitBoard n)	{ return m[0] == n.m[0] && m[1] == n.m[1]; }
	bool operator!=(BitBoard n)	{ return m[0] != n.m[0] || m[1] != n.m[1]; }
	BitBoard operator|(BitBoard n)	{ return BitBoard(m[0]|n.m[0], m[1]|n.m[1]); }
	BitBoard operator&(BitBoard n)	{ return BitBoard(m[0]&n.m[0], m[1]&n.m[1]); }
	BitBoard operator^(BitBoard n)	{ return BitBoard(m[0]^n.m[0], m[1]^n.m[1]); }
 	void operator-=(BitBoard n)	{ m[0] &= ~n.m[0]; m[1] &= ~n.m[1]; }
	void operator&=(BitBoard n)	{ m[0] &= n.m[0]; m[1] &= n.m[1]; }
	void operator^=(BitBoard n)	{ m[0] ^= n.m[0]; m[1] ^= n.m[1]; }
	void operator|=(BitBoard n)	{ m[0] |= n.m[0]; m[1] |= n.m[1]; }
	unsigned count();
	bool	includes(unsigned i);
	virtual unsigned	capacity();
	virtual Object*	copy();			// { return shallowCopy(); }
	virtual void	deepenShallowCopy();	// {}
	virtual unsigned	hash();
	virtual const Class*	isA();
	virtual bool	isEmpty();
	virtual bool	isEqual(const Object&);
	virtual void	printOn(ostream& s);
	virtual unsigned	size();
	virtual const Class*	species();
};

extern BitBoard squareBitBoard[64];
extern BitBoard rankBitBoard[8];
extern BitBoard fileBitBoard[8];
extern unsigned char bit_count[256];

inline BitBoard::BitBoard(unsigned i)	{ *this = squareBitBoard[i]; }
	
inline unsigned BitBoard::count()
{
	register unsigned char* p = c;
	register unsigned n = 0;
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	n += bit_count[*p++];
	return n;
}

inline bool BitBoard::includes(unsigned i)
{
	return (m[0] & squareBitBoard[i].m[0]) || (m[1] & squareBitBoard[i].m[1]);
}

#endif

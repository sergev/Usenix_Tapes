/* BitBoard.c -- member functions of class BitBoard

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
	January, 1986

Function:
	

$Log:	BitBoard.c,v $
 * Revision 1.2  88/01/16  23:37:53  keith
 * Remove pre-RCS modification history
 * 
*/

#include "BitBoard.h"
#include "oopsIO.h"

#define	THIS	BitBoard
#define	BASE	Object
static void bitBoardInit(const Class&);
DEFINE_CLASS(BitBoard,Object,1,"$Header: BitBoard.c,v 1.2 88/01/16 23:37:53 keith Exp $",bitBoardInit,NULL);

BitBoard squareBitBoard[64];
BitBoard rankBitBoard[8];
BitBoard fileBitBoard[8];
unsigned char bit_count[256];

static void bitBoardInit(const Class&)
{
	register BitBoard* p = squareBitBoard;
	register unsigned i,j,k;
	for (i=0; i<8; i++) {
		for (j=0, k=1; j<8; j++, k+=k) {
			(*p++).c[i] = k;
		}
	}
	p = rankBitBoard;
	register BitBoard* q = fileBitBoard;
	for (i=0, k=1; i<8; i++, k+=k) {
		(*p++).c[i] = 0xFF;
		for (j=0; j<8; j++) {
			(*q).c[j] = k;
		}
		q++;
	}
	for (i=0; i<256; i++) {
		bit_count[i] = 0;
		j = i;
		while (j != 0) {
			bit_count[i]++;
			j &= j-1;
		}
	}
}

unsigned BitBoard::capacity()	{ return sizeof(BitBoard)*8; }

Object* BitBoard::copy()		{ return shallowCopy(); }

void BitBoard::deepenShallowCopy()	{}

unsigned BitBoard::hash()		{ return m[0]^m[1]; }
	
bool BitBoard::isEmpty()	{ return m[0]==0 && m[1]==0; }
	
bool BitBoard::isEqual(const Object& ob)
{
	return (ob.isSpecies(class_BitBoard)) && (*this == *(BitBoard*)&ob);
}

const Class* BitBoard::species()	{ return &class_BitBoard; }

void BitBoard::printOn(ostream& strm)
{
	for (register i=0; i<8; i++) {
		strm << "\n";
		for (register j=7; j>=0; j--) {
			if (includes(8*i+j)) strm << " *";
			else strm << " .";
		}
	}
}		

unsigned BitBoard::size()	{ return count(); }

BitBoard::BitBoard(istream& strm, BitBoard& where)
{
	this = &where;
	strm >> m[0] >> m[1];
}

void BitBoard::storer(ostream& strm)
{
	BASE::storer(strm);
	strm << m[0] << " " << m[1] << " ";
}

BitBoard::BitBoard(fileDescTy& fd, BitBoard& where)
{
	this = &where;
	readBin(fd,c,8);
}

void BitBoard::storer(fileDescTy& fd) 
{
	BASE::storer(fd);
	storeBin(fd,c,8);
}

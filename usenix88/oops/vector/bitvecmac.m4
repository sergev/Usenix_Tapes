/* bitvecmac.m4 -- m4 macro templates for BitVecs

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

$Log:	bitvecmac.m4,v $
Revision 1.1  88/01/17  09:47:26  keith
Initial revision

	
*/

// BITVECGEN(BitVec,Length,Condition,Advance)
define(BITVECGEN,
{
	register bitVecByte m;
	bitVecByte* bp = $1;
	register unsigned i = ($2) >> 3;
	while (i--) {
		m = 0;
		if ($3) m |= 0x01;  $4;
		if ($3) m |= 0x02;  $4;
		if ($3) m |= 0x04;  $4;
		if ($3) m |= 0x08;  $4;
		if ($3) m |= 0x10;  $4;
		if ($3) m |= 0x20;  $4;
		if ($3) m |= 0x40;  $4;
		if ($3) m |= 0x80;  $4;
		*bp++ = m;		 
	}				 
	if ((i = ($2)&7) != 0) {
		m = 0;
		register int mm = 1;
		while (i--) {
			if ($3) m |= mm;
			mm <<= 1;
			$4;
		}
		*bp = m;
	};
}
)

// BITVECSCAN(BitVec,Length,Statement)
define(BITVECSCAN,
{
	register bitVecByte m;
	bitVecByte* bp = $1;
	register unsigned i = ($2) >> 3;
	while(i--) {
		m = *bp++;
		if (m&0x01) $3;
		if (m&0x02) $3;
		if (m&0x04) $3;
		if (m&0x08) $3;
		if (m&0x10) $3;
		if (m&0x20) $3;
		if (m&0x40) $3;
		if (m&0x80) $3;
	}	
	m = *bp;
	i = ($2) & 7;
	while (i--) {
		if (m&0x01) $3;
		m >>= 1;
	}
}
)

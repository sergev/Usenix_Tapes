/* Test class BitBoard

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
	
$Log:	bitboard.c,v $
 * Revision 1.1  88/01/17  22:24:27  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: bitboard.c,v 1.1 88/01/17 22:24:27 keith Exp $";

#include "BitBoard.h"

main()
{
	cout << "Test class BitBoard\n";
	BitBoard a(0,0),b(0,0);
	for (register i=0; i<8; i++) {
		for (register j=0; j<=i; j++) {
			a |= BitBoard(8*i+j);
		}
	}
	cout << "a:" << a << "\n";
	b = ~a;
	cout << "b:" << b << "\n";
	cout << "a-rankBitBoard[4]:" << (a-rankBitBoard[4]) << "\n";
	cout << "a|fileBitBoard[5]:" << (a|fileBitBoard[5]) << "\n";
	cout << "a.count() = " << a.count() << "\n";
}

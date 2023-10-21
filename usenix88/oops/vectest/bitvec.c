/* Test class BitVec

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
	
$Log:	bitvec.c,v $
 * Revision 1.1  88/01/17  10:15:02  keith
 * Initial revision
 * 

*/
#include "IntVec.h"
static char rcsid[] = "$Header: bitvec.c,v 1.1 88/01/17 10:15:02 keith Exp $";

static int initI[] = { 1,3,5 };
static bitVecByte initA[] = { 0x0a, 0x03 };
static bitVecByte initB[] = { 0x0c, 0x02 };

main()
{
	cerr << "Test class BitVec\n";
	IntVec I(initI,3);		// I = { 1 3 5 }
	cerr << "IntVec I(3,1,2): " << I << "\n";
	BitVec A(initA,10);		// A = { 0 1 0 1 0 0 0 0 1 1 }
// BitVec(bitVecByte*,unsigned)
	cerr << "BitVec A(initA,10): " << A << "\n";
	BitVec B(initB,10);		// B = { 0 0 1 1 0 0 0 0 0 1 }
	cerr << "BitVec B(initB,10): " << B << "\n";
// BitVec(unsigned)
	BitVec C;			// C = {}
// BitVec(const BitVec&)
	BitVec T(A);			// T = A
	cerr << "BitVec T(A): " << T << "\n";
// BitVec(const BitSlice&)
	BitVec U(A(0,5,2));		// U = { 0 0 0 0 1 }
	cerr << "BitVec U(A(0,5,2)): " << U << "\n";
// BitVec(unsigned,bool)
	BitVec V(10,YES);		// V = { 1 1 1 1 1 1 1 1 1 1 }
	cerr << "BitVec V(10,YES): " << V << "\n";
// BitVec::operator=(const BitVec&)
	C = U;				// C = { 0 0 0 0 1 }
	cerr << "C = U: " << C << "\n";
	C = C;
	cerr << "C = C: " << C << "\n";
// BitVec::operator=(const BitSlice&)
	C = A(0,5,2);			// C = { 0 0 0 0 1 }
	cerr << "C = A(0,5,2): " << C << "\n";
	C = A;  C = C(0,5,2);		// C = { 0 0 0 0 1 }
	cerr << "C = A; C = C(0,5,2): " << C << "\n";
// BitVec::operator=(const BitSlct&)
	C = A[B];			// C = { 0 1 1 }
	cerr << "C = A[B]: " << C << "\n";
	C = A;  C = C[B];		// C = { 0 1 1 }
	cerr << "C = A;  C = C[B]: " << C << "\n";
// BitVec::operator=(const BitPick&)
	C = A[I];			// C = { 1 1 0 }
	cerr << "C = A[I]: " << C << "\n";
	C = A;  C = C[I];		// C = { 1 1 0 }
	cerr << "C = A;  C = C[I]: " << C << "\n";
// BitVec::operator=(bool)
	C = 0;				// C = { 0 0 0 }
	cerr << "C = 0: " << C << "\n";
// BitVec::operator!(const BitVec&)
	C = !A;				// C = { 1 0 1 0 1 1 1 1 0 0 }
	cerr << "C = !A: " << C << "\n";
// FRIEND_BitVec_OP_BitVec__BitVec
	C = A|B;			// C = { 0 1 1 1 0 0 0 0 1 1 }
	cerr << "C = A|B: " << C << "\n";
// FRIEND_BitVec_ASNOP_BitVec
	C = A;  C ^= B;			// C = { 0 1 1 0 0 0 0 0 1 0 }
	cerr << "C = A;  C ^= B: " << C << "\n";
// BitSlice::operator=(const BitVec&)
	C = A;  C(0,5,2) = BitVec(5,YES);	// C = { 1 1 1 1 1 0 1 0 1 1 }
	cerr << "C = A;  C(0,5,2) = BitVec(5,YES): " << C << "\n";
// BitSlice::operator=(const BitSlice&)
	C(1,5,2) = C(0,5,2);		// C = { 1 1 1 1 1 1 1 1 1 1 }
	cerr << "C(1,5,2) = C(0,5,2): " << C << "\n";
// BitSlice::operator=(const BitPick&)
	C(0,3,1) = A[I];		// C = { 1 1 0 1 1 1 1 1 1 1 }
	cerr << "C(0,3,1) = A[I]: " << C << "\n";
// BitSlice::operator=(const BitSlct&)
	C(3,3,1) = A[B];		// C = { 1 1 0 0 1 1 1 1 1 1 }
	cerr << "C(3,3,1) = A[B]: " << C << "\n";
// BitSlice::operator=(bool)
	C(0,6,1) = 0;			// C = { 0 0 0 0 0 0 1 1 1 1 }
	cerr << "C(0,6,1) = 0: " << C << "\n";
// BitVec operator!(const BitSlice&)
	C = !A(0,5,2);			// C = { 1 1 1 1 0 }
	cerr << "C = !A(0,5,2): " << C << "\n";
// FRIEND_BitSlice_OP_BitSlice__Bitvec
	C = A(0,5,2)^B(1,5,2);		// C = { 0 1 0 0 0 }
	cerr << "C = A(0,5,2)^B(1,5,2): " << C << "\n";
// FRIEND_BitSlice_ASNOP_BitSlice
	C = A;  C(0,5,2) ^= B(1,5,2);	// C = { 0 1 1 1 0 0 0 0 0 1 }
	cerr << "C = A;  C(0,5,2) ^= B(1,5,2): " << C << "\n";
// BitPick::operator=(const BitVec&)
	C = A;  C[I] = BitVec(initA,3);	// C = { 0 0 0 1 0 0 0 0 1 1 }
	cerr << "C = A; C[I] =  BitVec(initA,3): " << C << "\n";
// BitPick::operator=(const BitPick&)
	C[I] = A[I];			// C = { 0 1 0 1 0 0 0 0 1 1 }
	cerr << "C[I] = A[I]: " << C << "\n";
// BitPick::operator=(const BitSlct&)
	C[I] = A[B];			// C = { 0 0 0 1 0 1 0 0 1 1 }	
	cerr << "C[I] = A[B]: " << C << "\n";
// BitPick::operator=(const BitSlice&)
	C = A;  C[I] = A(0,3,2);	// C = { 0 0 0 0 0 0 0 0 1 1 }
	cerr << "C[I] = A(0,3,2): " << C << "\n";
// BitPick::operator=(bool scalar)
	C[I] = YES;			// C = { 0 1 0 1 0 1 0 0 1 1 }
	cerr << "C[I] = YES: " << C << "\n";
// BitPick::operator BitSlice()
	C = A[I]|B[I];			// C = { 1 1 0 }
	cerr << "C = A[I]|B[I]: " << C << "\n";
// BitSlct::operator=(const BitVec&)
	C = A;  C[A] = BitVec(4,NO);
	cerr << "C = A;  C[A] = BitVec(4,NO): " << C << "\n";	// C = { 0 0 0 0 0 0 0 0 0 0 }
// BitSlct::operator=(const BitPick&)
	C = B;  C[B] = A[I];
	cerr << "C = B;  C[B] = A[I]: " << C << "\n";		// C = { 0 0 1 1 0 0 0 0 0 0 }
// BitSlct::operator=(const BitSlct&)
	C = A;  C[A] = B[A];					// C = { 0 0 0 1 0 0 0 0 0 1 }
	cerr << "C = A;  C[A] = B[A]: " << C << "\n";
// BitSlct::operator=(const BitSlice&)
	C = A;  C[A] = B(0,4,1);				// C = { 0 0 0 0 0 0 0 0 1 1 }
	cerr << "C = A;  C[A] = B(0,4,1): " << C << "\n";
// BitSlct::operator=(bool)
	C = A;  C[A] = NO;					// C = { 0 0 0 0 0 0 0 0 0 0 }
	cerr << "C = A;  C[A] = NO: " << C << "\n";
// BitSlct::operator BitSlice()
	C = A[B]|B(0,3,1);					// C = { 0 1 1 }
	cerr << "C = A[B]|B(0,3,1): " << C << "\n";
// BitVec reverse(const BitSlice&)
	C = reverse(B(0,4,1));
	cerr << "reverse(B(0,4,1)): " << C << "\n";		// { 1 1 0 0 }
// int sum(const BitSlice&)
	cerr << "sum(A(1,5,2)): " << sum(A(1,5,2)) << "\n";	// 3
}

/* Test Vector classes, part 1

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
	
$Log:	vector1.c,v $
 * Revision 1.1  88/01/17  10:15:05  keith
 * Initial revision
 * 

*/
#include "IntVec.h"
#include "DoubleVec.h"
static char rcsid[] = "$Header: vector1.c,v 1.1 88/01/17 10:15:05 keith Exp $";

static int initI[] = { 1,3,5 };

main()
{
	cerr << "Test Vector classes\n";
	BitVec L;
// TYPEVec_CTOR_TYPEPTR_I
	IntVec I(initI,3);		// I = { 1 3 5 }
	cerr << "IntVec I(3,1,2): " << I << "\n";
// TYPEVec_CTOR_I_TYPE_TYPE
	DoubleVec A(10,1.0);		// A = { 1. 2. 3. 4. 5. 6. 7. 8. 9. 10. }
	cerr << "DoubleVec A(10,1.0): " << A << "\n";
// TYPEVec_CTOR_I_TYPE_TYPE
	DoubleVec B(10,10.0,-1.0);	// B = { 10. 9. 8. 7. 6. 5. 4. 3. 2. 1. }
	cerr << "DoubleVec B(10,10.0,-1.0): " << B << "\n";
// TYPEVec_CTOR_I
	DoubleVec C;			// C = {}
// TYPEVec_CTOR_TYPEVec	
	DoubleVec T(A);			// T = A
	cerr << "DoubleVec T(A): " << T << "\n";
// TYPEVec_CTOR_TYPESlice
	DoubleVec U(A(0,5,2));		// U = { 1. 3. 5. 7. 9. }
	cerr << "DoubleVec U(A(0,5,2)): " << U << "\n";
// TYPEVec_ASN_TYPEVec
	C = U;
	cerr << "C = U: " << C << "\n";
// TYPEVec_ASN_TYPEVec
	C = C;
	cerr << "C = C: " << C << "\n";
// TYPEVec_ASN_TYPESlice
	C = A(0,5,2);			// C = { 1. 3. 5. 7. 9. }
	cerr << "C = A(0,5,2): " << C << "\n";
// TYPEVec_ASN_TYPESlice
	C = A;  C = C(0,5,2);		// C = { 1. 3. 5. 7. 9. }
	cerr << "C = A; C = C(0,5,2): " << C << "\n";
// TYPEVec_ASN_TYPESlct
	C = A[A>B];			// C = { 6. 7. 8. 9. 10. }
	cerr << "C = A[A>B]: " << C << "\n";
// TYPEVec_ASN_TYPESlct
	C = A;  C = C[C>B];		// C = { 6. 7. 8. 9. 10. }
	cerr << "C = A;  C = C[C>B]: " << C << "\n";
// TYPEVec_ASN_TYPEPick
	C = A[I];			// C = { 2. 4. 6. }
	cerr << "C = A[I]: " << C << "\n";
// TYPEVec_ASN_TYPEPick
	C = A;  C = C[I];		// C = { 2. 4. 6. }
	cerr << "C = A;  C = C[I]: " << C << "\n";
// TYPEVec_ASN_TYPE
	C = 0.;
	cerr << "C = 0.: " << C << "\n";
// TYPESlice_ASN_TYPEVec
	C = A;  C(0,5,2) = DoubleVec(5,-1.,0.);	// C = { -1. 2. -1. 4. -1. 6. -1. 8. -1. 10. }
	cerr << "C = A;  C(0,5,2) = DoubleVec(5,-1.,0.): " << C << "\n";
// TYPESlice_ASN_TYPESlice
	C(1,5,2) = C(0,5,2);		// C = { -1. -1. -1. -1. -1. -1. -1. -1. -1. -1. }
	cerr << "C(1,5,2) = C(0,5,2): " << C << "\n";
// TYPESlice_ASN_TYPEPick
	C(0,3,1) = A[I];		// C = { 2. 4. 6. -1. -1. -1. -1. -1. -1. -1. }
	cerr << "C(0,3,1) = A[I]: " << C << "\n";
// TYPESlice_ASN_TYPESlct
	C(3,3,1) = A[A<=3.];		// C = { 2. 4. 6. 1. 2. 3. -1. -1. -1. -1. }
	cerr << "C(3,3,1) = A[A<=3.]: " << C << "\n";
// TYPESlice_ASN_TYPE
	C(0,6,1) = 0.;			// C = { 0. 0. 0. 0. 0. 0. -1. -1. -1. -1. }
	cerr << "C(0,6,1) = 0.:" << C << "\n";
// FRIEND_OP_TYPESlice__TYPEVec
	C = -A;				// C = { -1. -2. -3. -4. -5. -6. -7. -8. -9. -10 }
	cerr << "C = -A: " << C << "\n";
// FRIEND_INCDECOP_TYPESlice__TYPEVec
//	C++;				// C = { 0. -1. -2. -3. -4. -5. -6. -7. -8. -9. }
	cerr << "C++: "; (C++).printOn(cerr); cerr << "\n";
// FRIEND_TYPESlice_OP_TYPESlice__TYPEvec
	C = A-B;			// C = { -9. -7. -5. -3. -1. 1. 3. 5. 7. 9. }
	cerr << "C = A-B: " << C << "\n";
// FRIEND_TYPESlice_OP_TYPE__TYPEVec
	C = A-1.;			// C = { 0. 1. 2. 3. 4. 5. 6. 7. 8. 9. }
	cerr << "C = A-1: " << C << "\n";
// FRIEND_TYPE_OP_TYPESlice__TYPEVec
	C = 1. + A;			// C = { 2. 3. 4. 5. 6. 7. 8. 9. 10. 11. }
	cerr << "C = 1.+A: " << C << "\n";
// FRIEND_TYPESlice_OP_TYPESlice__BitVec
	L = A>B;
	cerr << "A>B: " << L << "\n";	// { 0 0 0 0 0 1 1 1 1 1 }
// FRIEND_TYPESlice_OP_TYPE__BitVec
	L = A<=5.;			// { 1 1 1 1 1 0 0 0 0 0 }
	cerr << "A<=5.: " << L << "\n";
// FRIEND_TYPE_OP_TYPESlice__BitVec
	L = 5.<A;			// { 0 0 0 0 0 1 1 1 1 1 }
	cerr << "5.<A: " << L << "\n";
// FRIEND_TYPESlice_ASNOP_TYPESlice
	C = A;  C += A;			// { 2. 4. 6. 8. 10. 12. 14. 16. 18. 20. }
	cerr << "C = A;  C += A: " << C << "\n";
// FRIEND_TYPESlice_ASNOP_TYPE
	C -= 1.;			// { 1. 3. 5. 7. 9. 11. 13. 15. 17. 19. }
	cerr << "C -= 1.: " << C << "\n";
// TYPEPick_ASN_TYPEVec
	C = A;  C[I] = DoubleVec(3,-1.,0.);	// { 1. -1. 3. -1. 5. -1. 7. 8. 9. 10. }
	cerr << "C = A; C[I] =  DoubleVec(3,-1.,0.): " << C << "\n";
// TYPEPick_ASN_TYPEPick
	C[I] = A[I];			// { 1. 2. 3. 4. 5. 6. 7. 8. 9. 10. }
	cerr << "C[I] = A[I]: " << C << "\n";
// TYPEPick_ASN_TYPESlct
	C[I] = A[A<=3.];		// C = { 1. 1. 3. 2. 5. 3. 7. 8. 9. 10. }
	cerr << "C[I] = A[A<=3.]: " << C << "\n";
// TYPEPick_ASN_TYPESlice
	C[I] = A(0,3,2);		// C = { 1. 1. 3. 3. 5. 5. 7. 8. 9. 10. }
	cerr << "C[I] = A(0,3,2): " << C << "\n";
// TYPEPick_ASN_TYPE
	C[I] = -1.;			// C = { 1. -1. 3. -1. 5. -1. 7. 8. 9. 10. }
	cerr << "C[I] = -1.: " << C << "\n";
// TYPESlice_CTOR_TYPEPick
	C = A[I]+B[I];			// C = { 11. 11. 11. }
	cerr << "C = A[I]+B[I]: " << C << "\n";
}

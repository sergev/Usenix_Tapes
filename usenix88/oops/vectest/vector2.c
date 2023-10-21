/* Test class Vector, Part 2

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
	
$Log:	vector2.c,v $
 * Revision 1.1  88/01/17  10:15:07  keith
 * Initial revision
 * 

*/
#include "IntVec.h"
#include "DoubleVec.h"
static char rcsid[] = "$Header: vector2.c,v 1.1 88/01/17 10:15:07 keith Exp $";

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
// TYPESlct_ASN_TYPEVec
	C = A;  C[A<=5.] = DoubleVec(5,-1.,0.);	// C = { -1. -1. -1. -1. -1. 6. 7. 8. 9. 10. }
	cerr << "C = A; C[A<=5.] = DoubleVec(5,-1.,0.): " << C << "\n";
// TYPESlct_ASN_TYPEPick
	C = A;  C[A<=3.] = A[I];	// C = { 2. 4. 6. 4. 5. 6. 7. 8. 9. 10. }
	cerr << "C = A; C[A<=3.] = A[I]: " << C << "\n";
// TYPESlct_ASN_TYPESlct
	C = A;  C[A<=5.] = B[A<=5.];	// C = { 10. 9. 8. 7. 6. 6. 7. 8. 9. 10. }
	cerr << "C = A; C[A<=5.] = B[A<=5.]: " << C << "\n";
// TYPESlct_ASN_TYPESlice
	C[A<=5.] = A(0,5,1);		// C = { 1. 2. 3. 4. 5. 6. 7. 8. 9. 10. }
	cerr << "C = A; C[A<=5.] = A(0,5,1): " << C << "\n";
// TYPESlct_ASN_TYPE
	C[A<=5.] = 0.;			// C = { 0. 0. 0. 0. 0. 6. 7. 8. 9. 10. }
	cerr << "C = A; C[A<=5.] = 0.: " << C << "\n";
// TYPESlice_CTOR_TYPESlct
	C = A[A<=5.] + B[B<=5.];	// C = { 6. 6. 6. 6. 6. }
	cerr << "C = A[A<=5.] + B[B<=5.]: " << C << "\n";
// FRIEND_abs_TYPESlice
	C = abs(DoubleVec(10,-5));	// C = { 5. 4. 3. 2. 1. 0. 1. 2. 3. 4 }
	cerr << "abs(DoubleVec(10,-5)): " << C << "\n";
// FRIEND_atan2_TYPESlice_TYPESlice
	C = atan2(DoubleVec(10,1),DoubleVec(10,10,-1));
	cerr << "atan2(DoubleVec(10,1),DoubleVec(10,10,-1)): " << C << "\n";
// FRIEND_pow_TYPESlice_TYPESlice
	C = pow(DoubleVec(10,1),DoubleVec(10,2,0));
	cerr << "pow(DoubleVec(10,1),DoubleVec(10,2,0)): " << C << "\n";
// FRIEND_cumsum_TYPESlice
	C = cumsum(DoubleVec(10,1));
	cerr << "cumsum(DoubleVec(10,1)): " << C << "\n";
// FRIEND_delta_TYPESlice
	C = delta(C);
	cerr << "delta(cumsum(DoubleVec(10,1))): " << C << "\n";
// FRIEND_dot_TYPESlice_TYPESlice
	cerr << "dot(A,B): " << dot(A,B) << "\n";
// FRIEND_max_TYPESlice
	cerr << "max(A): " << max(A) << "\n";
// FRIEND_min_TYPESlice
	cerr << "min(A): " << min(A) << "\n";
// FRIEND_prod_TYPESlice
	cerr << "prod(A): " << prod(A) << "\n";
// FRIEND_reverse_TYPESlice
	C = reverse(A);
	cerr << "reverse(A): " << C << "\n";
// FRIEND_sqrt_TYPESlice
	C = sqrt(A);
	cerr << "sqrt(A): " << C << "\n";
// FRIEND_sum_TYPESlice
	cerr << "sum(A): " << sum(A) << "\n";
}

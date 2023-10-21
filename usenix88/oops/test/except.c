/* Test exception handling

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
	
$Log:	except.c,v $
 * Revision 1.1  88/01/17  22:24:38  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: except.c,v 1.1 88/01/17 22:24:38 keith Exp $";

#include "Exception.h"
#include <stream.h>

enum exceptionCode { EXCEPTION1=1, EXCEPTION2, EXCEPTION3, EXCEPTION4 };

void x(exceptionCode n)
{
	BEGINX
		if (n>EXCEPTION2) {
			cerr << "Raising EXCEPTION" << n << "...";
			RAISE(n);
		}
		cerr << "Trying normal return from function\n";
		return;
	EXCEPTION
		case EXCEPTION3: cerr << "EXCEPTION3 handled\n"; return;
		default: cerr << "trying RAISE(EXCEPTION_CODE)...";
			RAISE(EXCEPTION_CODE);
	ENDX
}

main()
{
	cerr << "Begin exception handler test\n";

	BEGINX
		cerr << "Testing normal execution\n";
	EXCEPTION
		default: cerr << "This should not happen!\n";
	ENDX

	BEGINX
		cerr << "Raising EXCEPTION1...";
		RAISE(EXCEPTION1);
		cerr << "EXCEPTION1 not handled!\n";
	EXCEPTION
		case EXCEPTION1: cerr << "EXCEPTION1 handled\n";
	ENDX
	
	BEGINX
		cerr << "Testing nested exception block\n";
		x(EXCEPTION2);
		BEGINX
			cerr << "Raising EXCEPTION2...";
			RAISE(EXCEPTION2);
			cerr << "EXCEPTION2 not handled!\n";
		EXCEPTION
			case EXCEPTION2: cerr << "EXCEPTION2 handled\n";
				cerr << "Raising EXCEPTION3...";
				x(EXCEPTION3);
		ENDX
		
		cerr << "Raising EXCEPTION4...";
		x(EXCEPTION4);
		cerr << "EXCEPTION4 not handled!\n";

	EXCEPTION
		default: cerr << "Test unhandled exception handler\n";
			RAISE(EXCEPTION_CODE);
	ENDX
}

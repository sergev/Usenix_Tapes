/* Test classes Catch, ExceptionAction, and ExceptionTrap

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
	
$Log:	error.c,v $
 * Revision 1.1  88/01/17  22:24:37  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: error.c,v 1.1 88/01/17 22:24:37 keith Exp $";

#include "Stack.h"
#include "Exception.h"
#include "oopserrs.h"

void xtrap(unsigned& err, int& sev)
{
	cerr << "xtrap(" << err << "," << sev << ") called\n";
}

class MyClass : public Object {
	int n;
	Catch catch;
public:
	MyClass(int i) : catch(this) {
		n = i;
		cerr << "MyClass(" << n << ") constructed\n";
	}
	virtual void destroyer() {
		cerr << "MyClass(" << n << ") destroyed\n";
	}
};

main()
{
	cerr << "Test classes Catch, ExceptionAction, and ExceptionTrap\n";
	Stack s;
	MyClass* p = new MyClass(-1);
	MyClass* q = new MyClass(-2);
	{
		MyClass a(1);
		MyClass b(2);
		RaiseException x(OOPS__CLTNEMPTY);
		BEGINX
			MyClass c(3);
			MyClass d(4);
			{
				MyClass e(5);
				MyClass f(6);
				delete p;
				s.pop();
			}
		EXCEPTION
			case OOPS__CLTNEMPTY: {
				MyClass g(7);
				MyClass h(8);
				cerr << "CLTNEMPTY error handled\n";
			}
		ENDX
	}
	delete q;
	{
		ExceptionTrap x(xtrap);
		s.pop();
	}
}

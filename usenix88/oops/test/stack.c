/* Test class Stack

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
	
$Log:	stack.c,v $
 * Revision 1.1  88/01/17  22:25:09  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: stack.c,v 1.1 88/01/17 22:25:09 keith Exp $";

#include "Point.h"
#include "Rectangle.h"
#include "Stack.h"

main()
{
	cout << "\nTest class Stack\n";
	Rectangle a(0,0,10,10);
	Point A(1,2);
	Point B(3,4); 
	Object* C = A.copy();
	Stack s(10);
	s.push(a);
	s.push(A);
	cout << s << "\n";
	cout << "top compare C = " << s.top()->compare(*C) << "\n";
	cout << *(s.pop()) << "\n";
	cout << *(s.top()) << "\n";
	s.pop();
	s.pop();	/* should get underflow error */
}

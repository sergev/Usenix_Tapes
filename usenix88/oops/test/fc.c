/* Test class Fraction

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
	
$Log:	fc.c,v $
 * Revision 1.1  88/01/17  22:24:40  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: fc.c,v 1.1 88/01/17 22:24:40 keith Exp $";

#include "Fraction.h"

Fraction readf(istream& strm)
{
	char delim;
	int u,v;
	strm >> u; strm.get(delim); strm >> v;
	if (strm.eof()) exit(0);
	return Fraction(u,v);
}

main()
{
	Fraction x,y,z;
	char op;
	while (YES) {
		cout << "Enter x: "; x = readf(cin); cout << "\n";
		cout << "Enter y: "; y = readf(cin); cout << "\n";
		cout << "Enter op: "; cin >> op; cout << "\n";
		switch (op) {
			case '+': { z = x+y; break; }
			case '-': { z = x-y; break; }
			case '*': { z = x*y; break; }
			case '/': { z = x/y; break; }
			case '<': { z = x<y; break; }
			case '>': { z = x>y; break; }
			case '=': { z = x==y; break; }
			default: { cout << "Incorrect op" << "\n"; continue; }
		}
		cout << x << " "; cout.put(op); cout << " " << y << " = " << z << "\n";
	}
}

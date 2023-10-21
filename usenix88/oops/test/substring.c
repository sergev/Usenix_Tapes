/* Test class SubString

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
	
$Log:	substring.c,v $
 * Revision 1.1  88/01/17  22:25:12  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: substring.c,v 1.1 88/01/17 22:25:12 keith Exp $";

#include "String.h"

main()
{
	cout << "\nTest Class SubString\n";
	String s4 = "0123456789";
	String s;
cout << "void SubString::operator=(const String&): ";
	s = s4;
	s(1,3) = String("xxx");
	cout << s << "\n";			// "0xxx456789"
cout << "void SubString::operator=(const SubString&): ";
	s = s4;
	s(1,3) = String('x',10)(0,3);
	cout << s << "\n";			// "0xxx456789"
cout << "void SubString::operator=(const char*): ";
	s = s4;
	s(1,3) = "xxx";
	cout << s << "\n";			// "0xxx456789"
	s = s4;
	s(10,0) = "";
	cout << s << "\n";			// "0123456789"
cout << "void replace(const char* st, unsigned ln):\n";
	s = s4; s.reSize(11);
	s(1,0) = "*";
	cout << s << "\n";			// "0*123456789"
	s = s4; s.reSize(11);
	s(0,0) = "*";
	cout << s << "\n";			// "*0123456789"
	s = s4; s.reSize(11);
	s(10,0) = "*";
	cout << s << "\n";			// "0123456789*"
	s = s4; s.reSize(0);
	s(1,1) = "";
	cout << s << "\n";			// "023456789"
	s = s4; s.reSize(0);
	s(0,1) = "";
	cout << s << "\n";			// "123456789"
	s = s4; s.reSize(0);
	s(9,1) = "";
	cout << s << "\n";			// "012345678"
	s = s4; s.reSize(0);
	s(10,0) = "";
	cout << s << "\n";			// "0123456789"
	s = s4; s.reSize(0);
	s(2,0) = s(0,1);
	cout << s << "\n";			// "01023456789"
	s = s4; s.reSize(0);
	s(0,0) = s(0,1);
	cout << s << "\n";			// "00123456789"
	s = s4; s.reSize(0);
	s(0,1) = s(9,0);
	cout << s << "\n";			// "123456789"
	s = s4; s.reSize(0);
	s(0,1) = s(0,2);
	cout << s << "\n";			// "01123456789"
cout << "bool SubString::operator<(const String&): ";
	cout << (s4(0,9) < s4) << "\n";		// 1
cout << "bool SubString::operator<(const SubString& ss): ";
	cout << (s4(0,8) < s4(0,9)) << "\n";	// 1
cout << "bool SubString::operator<(const char* cs): ";
	cout << (s4(0,9) < "01234567890") << "\n";	// 1
cout << "friend bool operator<(const char* cs, const SubString& ss): ";
	cout << ("01234567" < s4(0,9)) << "\n";	// 1
cout << "String SubString::operator&(const String&): ";
	cout << (s4(0,1) & s4) << "\n";		// "00123456789"
cout << "String SubString::operator&(const SubString&): ";
	cout << (s4(0,2) & s4(8,2)) << "\n";	// "0189"
cout << "String SubString::operator&(const char*): ";
	cout << (s4(0,2) & "*") << "\n";	// "01*"
cout << "friend String operator&(const char*, const SubString&): ";
	cout << ("*" & s4(0,2)) << "\n";	// "*01"
}

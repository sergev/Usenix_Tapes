/* Test class String

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
	
$Log:	string.c,v $
 * Revision 1.1  88/01/17  22:25:10  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: string.c,v 1.1 88/01/17 22:25:10 keith Exp $";

#include "oopsconfig.h"
#include "Range.h"
#include "String.h"

main()
{
	cout << "\nTest Class String\n";
cout << "String::String(char& c, unsigned l=1, unsigned extra): ";
	String s1 = 'x';
	cout << s1 << "\n";			// "x"
	String s2('x',25);
	cout << s2 << "\n";			// "xxxxxxxxxxxxxxxxxxxxxxxxx"
cout << "String::String(): ";
	String s3;
	cout << s3.length() << " " << s3.capacity() << "\n";	// 0 16
cout << "String::String(unsigned storage): ";
	String s9(100);
	cout << s9.length() << " " << s9.capacity() << "\n";	// 0 100
cout << "String::String(const char*): ";
	String s4 = "0123456789";
	cout << s4 << " " << s4.capacity() << "\n";	// "0123456789" 26
cout << "String::String(const char*, unsigned extra): ";
	String s8("0123456789",10);
	cout << s8 << " " << s8.capacity() << "\n";	// "0123456789" 20
cout << "String::String(const String&): ";
	String s5 = s4;
	cout << s5 << "\n";			// "0123456789"
cout << "String::String(const String&, unsigned extra): ";
	String s6(s4,0);
	cout << s6 << "\n";			// "0123456789"
cout << "String::String(const SubString&):\n";
cout << "SubString String::operator()(unsigned pos, unsigned lgt): ";
	String s7 = s6(1,3);
	cout << s7 << " " << s7.capacity() << "\n";	// "123" 19
cout << "String::String(const SubString&,  unsigned extra): ";
	String s(s6(1,3),10);
	cout << s << " " << s.capacity() << "\n";	// "123" 13
cout << "operator String::constCharPtTy(): ";
	cout << strchr(s4,'4') << "\n";		// "456789"
cout << "SubString String::operator()(const Range& r): ";
	cout << s6(Range(1,3)) << "\n";		// "123"
cout << "char& String::operator[](unsigned i): ";
cout << "void String::operator=(const char*): ";
	s = "0123456789";
	s[0] = s[9];
	cout << s << "\n";			// "9123456789"
cout << "unsigned String::length(): ";
	s = "";
	cout << s.length() << "\n";		// 0
cout << "unsigned String::size(): ";
	cout << s.size() << "\n";		// 0
cout << "unsigned String::capacity(): ";
	cout << s.capacity() << "\n";		// 19
cout << "unsigned String::reSize(unsigned new_capacity): ";
	s.reSize(0);
	cout << s.capacity() << "\n";		// 0
cout << "void String::operator=(const String&): ";
	s = s4;
	cout << s << "\n";			// "0123456789"
	s = s;
	cout << s << "\n";			// "0123456789"
cout << "void String::operator=(const SubString&): ";
	s = "87654321"; s.reSize(0);
	s = s4(1,9);
	cout << s << "\n";			// "123456789"
	s = s4;
	s = s(0,9);
	cout << s << "\n";			// "012345678"
	s = s4;
	s = s(1,9);
	cout << s << "\n";			// "123456789"
cout << "bool String::operator==(const String& s): ";
	s = s4;
	cout << (s == s4) << "\n";		// 1
cout << "bool String::operator<(const SubString ss): ";
	cout << (s < s(1,9)) << "\n";		// 1
cout << "bool operator<(const char* cs): ";
	cout << (s < "01234567890") << "\n";	// 1
cout << "friend bool operator<(const char* cs, const String& s): ";
	cout << ("0123" < s) << "\n";		// 1
cout << "String String::operator&(const String& s): ";
	s = s4;
	cout << (s & s4) << "\n";		// 01234567890123456789
cout << "String String::operator&(const SubString& ss): ";
	cout << (s & s4(1,9)) << "\n";		// 0123456789123456789
cout << "String String::operator&(const char* cs): ";
	cout << (s & "xxx") << "\n";		// 0123456789xxx
cout << "friend String operator&(const char* cs, const String& s): ";
	cout << ("xxx" & s) << "\n";		// xxx0123456789
cout << "friend String operator&(const char*, const SubString&): ";
	cout << ("xxx" & s4(1,8)) << "\n";	// xxx12345678
cout << "String& String::operator&=(const String&): ";
	s = s4; s.reSize(0);
	s &= s4;
	cout << s << "\n";			// 01234567890123456789
cout << "String& String::operator&=(const SubString&): ";
	s = s4; s.reSize(0);
	s &= s4(1,9);
	cout << s << "\n";			// 0123456789123456789
	s = s4; s.reSize(0);
	s &= s(1,9);
	cout << s << "\n";			// 0123456789123456789
cout << "String& String::operator&=(const char* cs): ";
	s = s4; s.reSize(0);
	((s &= "xxx") &= s8(0,1)) &= s1;
	cout << s << "\n";			// 0123456789xxx0x
cout << "char& String::at(unsigned i): ";
	s = "0123456789";
	s.at(0) = s.at(9);
	cout << s << "\n";			// "9123456789"
cout << "void String::toLower(): ";
	s = "abcABC";
	s.toLower();
	cout << s << "\n";			// "abcabc"
cout << "void String::toUpper(): ";
	s = "abcABC";
	s.toUpper();
	cout << s << "\n";			// "ABCABC"
}

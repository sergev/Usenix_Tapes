//-*-C++-*-
/* Copyright (c) 1987 Peter A. Buhr */
	
#include "VbyteSM.h"

extern VbyteHeap HeapArea;

class string {
	HandleNode Handle;				// handle node for storage management of strings
    public:
	inline int length();				// length of string
	inline string();				// string x
	string(char *);					// string x = "abc"
	inline string(string &);			// string x = string
	inline string(int);				// string x = int, including 'a'
	inline string(double);				// string x = double
	inline ~string();				// destructor
	operator char *();				// conversion to char *
	inline operator int();				// conversion to int(char)
	inline operator double();			// conversion to double
	friend ostream& operator<<(ostream &, string &);// input
	friend istream& operator>>(istream &, string &);// output
	string& operator=(string &);			// string = string
	inline string& operator+=(string &);		// string = string + string
	inline friend int operator==(string &, string &); // ==
	inline friend int operator!=(string &, string &); // !=
	inline friend int operator<(string &, string &);  // <
	inline friend int operator<=(string &, string &); // <=
	inline friend int operator>(string &, string &);  // >
	inline friend int operator>=(string &, string &); // >=
	friend string operator+(string &, string &);	// concatenate two strings
	string substr(int, int);			// create a subset of a string
	inline string substr(int);			// create a subset of a string
	inline string remove(int, int);			// remove a portion of a string
	inline string remove(int);	 		// remove a portion of a string
	int index(string &, int = 1);			// return position of one string in another
	int skip(string &, int = 1);			// skip over characters that are in the mask
	int include(string &, int = 1);			// skip over characters that are NOT in the mask
	inline string span(string &, int = 1);		// longest prefix of characters that are in the mask
	inline string stop(string &, int = 1);		// longest prefix of characters that are NOT in the mask
	string trim();					// remove trailing blanks
	inline string ltrim();				// remove leading blanks
	string repeat(int);				// repeat the string n times
	string replace(string &, string &);		// replace all occurrences of 1st string by 2nd
}; // string

/* Returns the length of the string.  */

inline int string::length() {				// length of string
	return( Handle.lnth );
} // length

/* Constructor for all string variables excluding header nodes. The
heap area must be passed to access the header node for the handle
list. This is because each new string variable is linked at the end of
the handle list.  */

inline string::string() : Handle(&HeapArea) {		// string x
} // string

/* Constructor for a type string declaration, initialized with a
string or for parameter passing. This is done by using the assignment
operator for type string. */

inline string::string(string &str) : Handle(&HeapArea) { // string x = string
	//cerr << "enter:string x = string, this:" << (int)this << " str:\"" << str << "\"\n";
	*this = str;
	//cerr << "exit:string x = string, r:\"" << *this << "\"\n";
} // string

/* Constructor for a type string declaration, initialized with an int.
Unfortunately, this includes char, hence 'a' is converted to its int
value.  As a result, char * constants must be used, as in "a" instead
of 'a'.  The conversion creates a char * from the function form and
then uses the char * to string conversion to create the string.  */

inline string::string(int i) : Handle(&HeapArea) {	// string x = int, including 'a'
	char *pc;
	//cerr << "enter:string = int, this:" << (int)this << " i:\"" << i << "\"\n";
	pc = form("%d", i);				// convert int to its corresponding character representation
	*this = pc;					// convert char * to string;
	delete pc;					// delete char * returned from form
	//cerr << "exit:string = int, r:\"" << *this << "\"\n";
} // string

/* Constructor for a type string declaration, initialized with an
double.  The conversion creates a char * from the function form and
then uses the char * to string conversion to create the string.  */

inline string::string(double d) : Handle(&HeapArea) {	// string x = double
	char *pc;
	//cerr << "enter:string x = double, this:" << (int)this << " d:\"" << d << "\"\n";
	pc = form("%g", d);				// convert double to its corresponding character representation
	*this = pc;					// convert char * to string;
	delete pc;					// delete char * returned from form
	//cerr << "exit:string x = double, r:\"" << *this << "\"\n";
} // string

/* Free the handle used by the string variable. The string pointed to
by this variable is not freed but will be freed during garbage
collection if there are no other string variables pointing at it.  */

inline string::~string() {				// destructor
} // ~string

/* Conversion from a string to an int. Since int converts to char,
this allows string to char conversions indirectly.  */

inline string::operator int() {				// conversion to int(char)
	int r;
	char *pc;
	//cerr << "enter:int, this:\"" << *this << "\"\n";
	pc = *this;					// convert from string to char *
	r = atoi(pc);
	delete pc;
	//cerr << "exit:int, r:" << r << "\n";
	return(r);
} // int

/* Conversion from a string to a double. Since double converts to
float, this allows string to float conversions indirectly.  */

inline string::operator double() {			// conversion to double
	double r;
	char *pc;
	//cerr << "enter:double, this:\"" << *this << "\"\n";
	pc = *this;
	r = atof(pc);
	delete pc;
	//cerr << "exit:double, r:" << r << "\n";
	return(r);
} // double

/* Shorthand form for: string = string + ... */

inline string& string::operator+=(string &rhs) {	// +=
	*this = *this + rhs;
	return(*this);
} // +=

/* Relational operators. They are friends instead of members so that
conversions will occur on either operand of the comparison.  */

inline int operator==(string &s1, string &s2) {		// ==
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) == 0);
} // ==

inline int operator!=(string &s1, string &s2) {		// !=
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) != 0);
} // !=

inline int operator<(string &s1, string &s2)  {		// <
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) <  0);
} // <

inline int operator<=(string &s1, string &s2) {		// <=
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) <= 0);
} // <=

inline int operator>(string &s1, string &s2) {		// >
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) >  0);
} // >

inline int operator>=(string &s1, string &s2) {		// >=
	return(HeapArea.ByteCmp(s1.Handle.s, 1, s1.length(), s2.Handle.s, 1, s2.length()) >= 0);
} // >=

/* Short hand version of substring. Only the starting position is
specified and the length is assumed to be the remainder of the string.
*/

inline string string::substr(int start) {		// create a subset of a string
	return( substr(start, length() - start + 1) );
} // substr

/* Returns the string remaining after removing the string starting at
a specified position (start) and having the specified length (slnth).
*/

inline string string::remove(int start, int slnth) {	// remove a portion of a string
	return( substr(1, start - 1) + substr(start + slnth) );
} // remove

/* Short hand version of remove. Only the starting position is
specified and the length is assumed to be the remainder of the string.
*/

inline string string::remove(int start) {		// remove a portion of a string
	return( remove(start, length() - start + 1) );
} // remove

/* Returns the longest prefix of characters of the current string that
ARE included in the mask string starting the search at position start.
*/

inline string string::span(string &mask, int start) {	// string of characters that are in the mask
	return( substr(start, skip(mask, start) - 1) );
} // span

/* Returns the longest prefix of characters of the current string that
are NOT included in the mask string starting the search at position
start.  */

inline string string::stop(string &mask, int start) {	// string of characters that are NOT in the mask
	return( substr(start, include(mask, start) - 1) );
} // stop

/* Returns a string with all the leading blanks removed from the
current string. */

inline string string::ltrim() {				// remove leading blanks
	int pos;
	//cerr << "enter:ltrim, this:\"" << *this << "\"\n";
	pos = skip(" ");
	//cerr << "exit:ltrim, r:\"" << substr(pos, length() - pos + 1) << "\"\n";
	return( substr(pos, length() - pos + 1) );
} // ltrim


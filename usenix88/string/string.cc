/* Copyright (c) 1987 Peter A. Buhr */
	
/* This is a type for manipulation of variable length character
strings. It is not necessary to specify a maximum length for a string
variable. The strings grow and shrink in size, dynamically. The
maximum length of a string is 64K.  These strings can contain any
character, including 0. The characters in the strings are numbered
from 1, not 0 as in arrays of char in C.  */

/* Subscripting is not implemented because strings are shared, and C++
is not powerful enough so that I can differentiate between
subscripting on the LHS and RHS of assignment. The problem that arises
is when a string contains shared substrings that would be changed by
assignment though subscripting.  If the subscript operation knew when
it was on the LHS of assignment is could copy the root string that was
being modified through subscripting and hence allow the operation, but
alas. A solution would be to have 2 overload subscript operators: one
returns a char and the other returns a char&. Then C++ would have to
be smart enough to use the char version on the RHS and the char&
version on the LHS. */

#include <stream.h>
#include "string.h"

VbyteHeap HeapArea(1000);				// allocate a heap for the string package

/* Constructor for a type string declaration, initialized with a char *
string.  A handle is created for the string variable at the end of
the handle list and the characters are copied into the string area for
the new variable.  */

string::string(char *pc) : Handle(&HeapArea) {		// string x = "abc"
	//cerr << "enter:string = char *, this:" << (int)this << " pc:\"" << pc << "\"\n";
	for (int pclnth = 0; pc[pclnth] != '\0'; pclnth += 1);	// find length of char *
	Handle.s = HeapArea.VbyteAlloc(pclnth);
	Handle.lnth = pclnth;
	for (int i = 0; pc[i] != '\0'; i += 1) {	// copy characters
		Handle.s[i] = pc[i];
	} // for
	//cerr << "exit:string = char *, r:\"" << *this << "\"\n";
} // string

/* Conversion from a string to a char *. It is the responsibility of
the user of this routine to delete the storage created for the char *.
*/

string::operator char *() {				// conversion to char *
	char *r;
	//cerr << "enter:char *, this:\"" << *this << "\"\n";
	r = new char[length() + 1];
	for (int i = 0; i < length(); i += 1) {
		if (Handle.s[i] != '\0') {
			r[i] = Handle.s[i];
		} else {
			cerr << "During the conversion of a string to a char *, a null character ('\0') was found in the string.\n"
				<< "Because char * strings are terminated with null characters, this character was replace by a blank character.\n";
			r[i] = ' ';
		} // if
	} // for
	r[length()] = '\0';
	//cerr << "exit:char *, r:\"" << r << "\"\n";
	return(r);
} // char *

/* Output string to output stream.  */

ostream& operator<<(ostream &out, string &str) {	// output
	for (int i = 0; i < str.length(); i += 1) {
		out << chr(str.Handle.s[i]);
	} // for
	return(out);
} // ostream

/* Read in characters until a newline ('\n') or end of file or any
sort of I/O error occurs.  The input string variable is moved to the
end of the handle list and the characters are allocated one at a time
from the end of the string area. If a garbage collection occurs during
input, the input string is moved up and further allocation can
continue.  */

istream& operator>>(istream &in, string &str) {		// input
	char ch;
	char *pc;
	//cerr << "enter:>>\n";
	str.Handle.s = string::HeapArea.VbyteAlloc(0);
	str.Handle.lnth = 0;
	str.Handle.MoveThisBefore(&HeapArea.Header);	// insert str at end of handle list
	for (;;) {
		in.get(ch);
	if (in.rdstate() != _good || ch == '\n') break;
		pc = string::HeapArea.VbyteAlloc(1);
		*pc = ch;
		str.Handle.lnth += 1;
	} // for
	//cerr << "exit:>>, str:\"" << str << "\"\n";
	return(in);
} // istream

/* Assigns the RHS to the LHS and returns the address of the LHS for
further use.  Assignment does not copy the string only the pointer to
the string. To keep the handle list sorted the LHS is moved after the
RHS on the handle list.  */

string& string::operator=(string &rhs) {		// string = string
	//cerr << "enter:=, rhs:\"" << rhs << "\"\n";
	Handle.s = rhs.Handle.s;
	Handle.lnth = rhs.Handle.lnth;
	Handle.MoveThisAfter(&rhs.Handle);		// insert this handle after rhs handle
	//cerr << "exit:=, this:\"" << *this << "\"\n";
	return(*this);
} // =

/* Returns the string which is the concatenation of the two
parameters. An optimization is done to prevent copying of the first
string if it is at the end of the string area. If it is at the end of
the string area, the second string is placed after it and the second
string contains the first and the handle of the resulting
concatenation is moved before the first string because it is the
longer string.  */

string operator+(string &str1, string &str2) {	// concatenate two strings
	string r;
	int clnth;
	//cerr << "enter:+, str1:\"" << str1 << "\" str2:\"" << str2 << "\"\n";
	clnth = str1.length() + str2.length();
	if (str1.Handle.s + str1.length() == str2.Handle.s) { // already juxtapose ?
		r.Handle.s = str1.Handle.s;
		r.Handle.lnth = clnth;
		r.Handle.OrderedMoveThisBefore(&str1.Handle); // insert r handle in order before str1
	} else { // must copy some text
		if (str1.Handle.s + str1.length() == string::HeapArea.VbyteAlloc(0)) {	// str1 at end of string area ?
			string::HeapArea.VbyteAlloc(str2.length());			// create room for 2nd part at the end of string area
			r.Handle.s = str1.Handle.s;
			r.Handle.lnth = clnth;
			r.Handle.OrderedMoveThisBefore(&str1.Handle); // insert r handle in order before str1
		} else {					// copy the two parts
			r.Handle.s = string::HeapArea.VbyteAlloc(clnth);
			r.Handle.lnth = clnth;
			string::HeapArea.ByteCopy(r.Handle.s, 1, str1.length(), str1.Handle.s, 1, str1.length());
		} // if
		string::HeapArea.ByteCopy(r.Handle.s, str1.length() + 1, str2.length(), str2.Handle.s, 1, str2.length());
	} // if
	//cerr << "exit:+, r:\"" << r << "\"\n";
	return(r);
} // +

/* Returns the string starting at a specified position (start) in the
string parameter, and having the specified length (lnth). If the
substring request extends beyond the end of the string, blanks are
added on the right to give the required length. If the starting
position is less than 1 or the length of the substring is negative, an
error routine is invoked. An optimization is performed to prevent
copying of the substring if it is a proper substring; otherwise the
substring is copied. If the string is not copied, its handle must be
moved after the handle of the containing string to keep the handle
list in sorted order.  */

string string::substr(int start, int slnth) {		// create a subset of a string
	//cerr << "enter:substr, this:\"" << *this << "\" start:" << start << " slnth:" << slnth << "\n";
	if (start < 1 || slnth < 0) {
		cerr << "Invalid argument to substr:" << start << "," << slnth << " for string:\"" << *this << "\"\n";
		exit(-1);
	} // if

	int limit;
	string r;

	limit = Handle.lnth - start + 1;
	if (slnth > limit) {				// copy substring ?
		r.Handle.s = HeapArea.VbyteAlloc(slnth);
		r.Handle.lnth = slnth;
		HeapArea.ByteCopy(r.Handle.s, 1, slnth, Handle.s, start, limit);
	} else {					// don't copy substring ?
		r.Handle.s = Handle.s + start - 1;
		r.Handle.lnth = slnth;
		r.Handle.OrderedMoveThisAfter(&this->Handle); // insert r handle after this handle
	} // if
	//cerr << "exit:substr, r:\"" << r << "\"\n";
	return(r);
} // substr

/* Returns the position of the first occurrence of the key string in
the current string starting the search at position start.  If the key
string does not appear in the current string, the length of the
current string plus one is returned. If the key string has zero
length, the value 1 is returned regardless of what the current string
contains.  */

int string::index(string& key, int start) {		// return position of one string in another
	int r;
	//cerr << "enter:index, this:\"" << *this << "\" key:\"" << key << "\"\n";
	if (start < 1) {
		cerr << "Invalid start position to index:" << start << "\n";
		exit(-1);
	} // if
	
	for (int i = start; ; i += 1) {
	if (i >= length() - key.length() + 2) {
			r = length() + 1;
			break;
		} // exit
	if (HeapArea.ByteCmp(Handle.s, i, key.length(), key.Handle.s, 1, key.length()) == 0) {
			r = i;
			break;
		} // exit
	} // for
	//cerr << "exit:index, r:" << r << "\n";
	return(r);
} // index

/* Returns the position of the first character in the current string
that does not appear in the mask string starting the search at
position start; hence it skips over characters in the current string
that are IN the mask string. The characters in the current string do
not have to be in the same order as the mask string. If all the
characters in the current string appear in the mask string the length
of the current string plus one is returned.  */

int string::skip(string& mask, int start) {		// skip over characters that are in the mask
	int r;
	//cerr << "enter:skip, this:\"" << *this << "\" mask:\"" << mask << "\"\n";
	if (start < 1) {
		cerr << "Invalid start position to skip:" << start << "\n";
		exit(-1);
	} // if
	
	for (int i = start - 1; i < length(); i += 1) {
		for (int j = 0; ; j += 1) {
		if (j == mask.length()) {
				r = i + 1;
				goto EndOfMask;
			} // exit
		if (Handle.s[i] == mask.Handle.s[j]) {
				break;
			} // exit
		} // for
	} // for
	r = length() + 1;
    EndOfMask:
	//cerr << "exit:skip, r:" << r << "\n";
	return(r);
} // skip

/* Returns the position of the first character in the current string
that does appear in the mask string starting the search at position
start; hence it skips over characters in the current string that are
NOT in the mask string. The characters in the current string do not
have to be in the same order as the mask string. If all the characters
in the current string appear in the mask string the length of the
current string plus one is returned.  */

int string::include(string& mask, int start) {		// skip over characters that are NOT in the mask
	int r;
	//cerr << "enter:include, this:\"" << *this << "\" mask:\"" << mask << "\"\n";
	if (start < 1) {
		cerr << "Invalid start position to include:" << start << "\n";
		exit(-1);
	} // if
	
	for (int i = start - 1; i < length(); i += 1) {
		for (int j = 0; j < mask.length(); j += 1) {
		if (Handle.s[i] == mask.Handle.s[j]) {
				r = i + 1;
				goto EndOfMask;
			} // exit
		} // for
	} // for
	r = length() + 1;
    EndOfMask:
	//cerr << "exit:include, r:" << r << "\n";
	return(r);
} // include

/* Returns a string with all the trailing blanks removed from the
current string.  */

string string::trim() {					// remove trailing blanks
	//cerr << "enter:trim, this:\"" << *this << "\"\n";
	for (int i = length() - 1; i >= 0; i -= 1) {
	if (Handle.s[i] != ' ') break;
	} // for
	//cerr << "exit:trim, r:\"" << substr(1, i + 1) << "\"\n";
	return( substr(1, i + 1) );
} // trim

/* Returns a string which is the current string repeated N times.  */

string string::repeat(int n) {				// repeat string n times
	string r;
	//cerr << "enter:repeat, this:\"" << *this << "\" n:" << n << "\n";
	r = "";
	for (int i = 1; i <= n; i += 1) {
		r = r + *this;
	} // for
	//cerr << "exit: repeat, r:\"" << r << "\"\n";
	return(r);
} // repeat

/* Returns a string in which all occurrences of the `from' string in
the current string have been replaced by the `to' string.  */

string string::replace(string& from, string& to) {
	int pos;
	string r;
	//cerr << "enter:replace, this:\"" << *this << "\" from:\"" << from << "\" to:\"" << to << "\"\n";
	pos = index(from);
	if (pos <= length()) {
		r = substr(1, pos - 1) + to + substr(pos + from.length()).replace(from, to);
	} else {
		r = *this;
	} // if
	//cerr << "exit:replace, r:\"" << r << "\"\n";
	return(r);
} //replace

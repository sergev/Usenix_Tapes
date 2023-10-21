.\" typeset -t -ms -Ti300 -S String.t | lpr -Ft -Pi300;
.\" tbl String.t | troff -ms -Ti300 -rF1 | lpr -Ft -Pi300;
.nr PS 11
.nr VS \n(PSp+2p
.ds Pm \f(CW\s-1
.ds Pe \s+1\fP
.\" Program begin
.de PM
.DS B
.ps -1
.vs \\n(.sp
.ft CW
..
.\" Program end
.de PE
.RT
.DE
..
.SH
.ce 1
.LG
Variable-Length String Type in C++
.FS
Copyright \(co 1987 by Peter A. Buhr
.FE
.NH 1
Type \*(Pmstring\*(Pe
.LP
The type \*(Pmstring\*(Pe is for the manipulation of variable length character strings in C++.
A \*(Pmstring\*(Pe differs from an array of char in that it is not necessary to specify a maximum length.
A string grows and shrinks in size, dynamically.
The only restriction is that the maximum length of a \*(Pmstring\*(Pe instance is 64K characters.
These strings can contain any character, including '\\0'.
The characters in the strings are numbered from 1, not 0 as in array of char.
The following routines have been defined to manipulate an instance of type \*(Pmstring\*(Pe.
The discussion assumes that the following declarations and assignment statements have been executed.
.PM
string peter, digit, alpha, punctuation, ifstmt;
 
peter  = "PETER"
digit  = "0123456789"
alpha  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
punctuation = "().,"
ifstmt = "IF(A.GT.B)THEN"

string s;
int i;
.PE
.IP \fBImplicit\ Conversion\ Operations\fP
The following conversions are allowed on type \*(Pmstring\*(Pe.
.PM
.TS
r|ccc.
	\*(Pmint\*(Pe & \*(Pmchar\*(Pe	\*(Pmdouble\*(Pe	\*(Pmchar *\*(Pe
_	_	_	_
\*(Pmstring\*(Pe	\(AA	\(AA	\(AA
.TE
.PE
Remember that \*(Pmchar\*(Pe is considered to be an integer in C++, so the following anomaly occurs:
.PM
s = 'a';   // s is assigned "141"
.PE
As well, when a string is converted to a \*(Pmchar *\*(Pe, the storage for the \*(Pmchar *\*(Pe is created by the conversion,
but it is the responsibility of the user to subsequently delete the storage for this value.
.IP \fBInput/Output\ Operators\fP
Both the C++ operators \*(Pm<<\*(Pe and \*(Pm>>\*(Pe are defined on type \*(Pmstring\*(Pe.
.IP \fBComparison\ Operators\fP
The comparison operators \*(Pm==, !=, <, <=, >, >=\*(Pe are defined on type \*(Pmstring\*(Pe.
.IP \*(Pmstring\ +(string\ &str1,\ string\ &str2)\*(Pe\ \fBOperator\fP
This binary operator produces a string which is the concatenation of the two argument strings.
.PM
s = peter + digit;   // s is assigned "PETER0123456789"
.PE
There is also a short hand form \*(Pm+=\*(Pe.
.IP \*(Pmint\ length()\*(Pe\ \fBOperation\fP
This unary operation returns the length of a string variable.
.PM
i = peter.length();  // i is assigned the value 5
.PE
.IP \*(Pmstring\ substr(int\ start,\ int\ lnth)\*(Pe\ \fBOperation\fP
This unary operation returns the string starting at a specified position (\*(Pmstart\*(Pe) in the current string, and having the specified length (\*(Pmlnth\*(Pe).
If the substring request extends beyond the end of the string, blanks are added on the right to give the required length.
If the starting position is less than 1 or the length of the substring is negative, an error message is printed and the program is terminated.
.PM
s = peter.substr(2, 3);   // s is assigned "ETE"
s = peter.substr(2, 5);   // s is assigned "ETER\o'b/'"
s = peter.substr(0, -1);  // error
.PE
.IP \*(Pmstring\ substr(int\ start)\*(Pe\ \fBOperation\fP
This unary operation is a short hand version of \*(Pmsubstr\*(Pe.
Only the starting position is specified and the length is assumed to be the remainder of the string.
.PM
s = peter.substr(2);   // s is assigned "ETER"
.PE
.IP \*(Pmstring\ remove(int\ start,\ int\ lnth)\*(Pe\ \fBOperation\fP
This unary operation returns the string remaining after removing the string starting at a specified position (\*(Pmstart\*(Pe) and having the specified length (\*(Pmlnth\*(Pe) from the current string.
If the starting position is less than 1 or the length of the substring is negative, an error message is printed and the program is terminated.
.PM
s = digit.remove(3, 3);   // s is assigned "0156789"
s = digit.remove(0, -1);  // error
.PE
.IP \*(Pmstring\ remove(int\ start)\*(Pe\ \fBOperation\fP
This unary operation is a short hand version of remove.
Only the starting position is specified and the length is assumed to be the remainder of the string.
.PM
s = digit.remove(3);   // s is assigned "01"
.PE
.IP \*(Pmint\ index(string\ &key,\ int\ start\ =\ 1)\*(Pe\ \fBOperation\fP
This unary operation returns the position of the first occurrence of the \*(Pmkey\*(Pe string in the current string starting the search at position \*(Pmstart\*(Pe.
If the \*(Pmkey\*(Pe string does not appear in the current string, the length of the current string plus one is returned.
If the \*(Pmkey\*(Pe string has zero length, the value 1 is returned regardless of what the current string contains.
.PM
i = digit.index("567");     // i is assigned 6
i = digit.index("567", 7);  // i is assigned 11
.PE
.IP \*(Pmint\ skip(string\ &mask,\ int\ start\ =\ 1)\*(Pe\ \fBOperation\fP
This unary operation returns the position of the first character in the current string that does not appear in the \*(Pmmask\*(Pe string starting the search at position \*(Pmstart\*(Pe;
hence it skips over characters in the current string that are IN the \*(Pmmask\*(Pe string.
The characters in the current string do not have to be in the same order as the \*(Pmmask\*(Pe string.
If all the characters in the current string appear in the \*(Pmmask\*(Pe string, the length of the current string plus one is returned.
.PM
i = peter.skip(digit);   // i is assigned 1
i = peter.skip(alpha);   // i is assigned 6
.PE
.IP \*(Pmint\ include(string\ &mask,\ int\ start\ =\ 1)\*(Pe\ \fBOperation\fP
This unary operation returns the position of the first character in the current string that does appear in the \*(Pmmask\*(Pe string starting the search at position \*(Pmstart\*(Pe;
hence it skips over characters in the current string that are NOT in the \*(Pmmask\*(Pe string.
The characters in the current string do not have to be in the same order as the \*(Pmmask\*(Pe string.
If all the characters in the current string do NOT appear in the \*(Pmmask\*(Pe string, the length of the current string plus one is returned.
.PM
i = peter.include(digit);   // i is assigned 6
i = ifstmt.include(punctuation); // i is assigned 3
.PE
.IP \*(Pmstring\ span(string\ &mask,\ int\ start)\*(Pe\ \fBOperation\fP
This unary operation returns the longest prefix of characters of the current string that ARE included in the \*(Pmmask\*(Pe string starting the search at position \*(Pmstart\*(Pe.
.PM
s = peter.span(alpha);   // s is assigned "PETER"
s = ifstmt.span(alpha);  // s is assigned "IF"
s = peter.span(digit);   // s is assigned ""
.PE
.IP \*(Pmstring\ stop(string\ &mask,\ int\ start)\*(Pe\ \fBOperation\fP
This unary operation returns the longest prefix of characters of the current string that are NOT included in the \*(Pmmask\*(Pe string starting the search at position \*(Pmstart\*(Pe.
.PM
s = peter.stop(digit);   // s is assigned "PETER"
s = ifstmt.stop(punctuation);   // s is assigned "IF"
s = peter.stop(alpha);   // s is assigned ""
.PE
.IP \*(Pmstring\ trim()\*(Pe\ \fBOperation\fP
This unary operation returns a string with all the trailing blanks removed from the current string.
.PM
s = string("ABC\o'b/'\o'b/'\o'b/'").trim(); // s is assigned "ABC"
.PE
.IP \*(Pmstring\ ltrim()\*(Pe\ \fBOperation\fP
This unary operation returns a string with all the leading blanks removed from the current string.
.PM
s = string("\o'b/'\o'b/'\o'b/'ABC").ltrim(); // s is assigned "ABC"
.PE
.IP \*(Pmstring\ repeat(int\ n)\*(Pe\ \fBOperation\fP
This unary operation returns a string which is the current string repeated \*(Pmn\*(Pe times.
.PM
s = peter.repeat(2);   // s is assigned "PETERPETER"
.PE
.IP \*(Pmstring\ replace(string\ &from,\ string\ &to)\*(Pe\ \fBOperation\fP
Returns a string in which all occurrences of the \*(Pmfrom\*(Pe string in the current string have been replaced by the \*(Pmto\*(Pe string.
.PM
s = peter.replace("E", "XX");   // s is assigned "PXXTXXR"
.PE
.NH 1
Obtaining the \*(Pmstring\*(Pe type
.LP
To use the \*(Pmstring\*(Pe type it is necessary to make the following specifications.
First, it is necessary to include the type definitions for the string type in your program, for example:
.PM
#include "???/include/string.h"
.PE
Second, it is necessary to specify the load library, which is usually done during compilation, for example:
.PM
% ccc \fIyourprogram.cc\fP ???/lib/string.o
.PE
.NH 1
Implementation
.LP
While the string type has a reasonable amount of functionality, it is most interesting because of its storage management.
This string type does \fInot\fP use the C++ dynamic storage area to allocate storage for the string variables.
Instead, a separate dynamic string area is allocated and all the strings are allocated and freed from that area.
As well, the text of the string variables is shared whenever possible; hence, many operations do not copy any text at all.
.PP
Specifically, the strings are a length field and a pointer to the string text in the string area.
The strings themselves are linked together in ascending order of references into the string area.
New text storage, that is allocated for a string, is always allocated from the end of the string area.
No effort is made to reuse freed text storage.
When the string area becomes full, the list of string variables is traversed compacting the text areas in the string area creating a contiguous area at the end of the string area.
If after compaction there is still insufficient storage, a larger string area is created, the old string is copied into it, and the old string area is released back to the storage manager for C++.
.PP
Because new text areas are allocated at the end of the string area, it is easy to keep the string variables in sorted order.
When a new text area is allocated for a string variable, the string variable is moved to the end of the string variable list because it must have the largest reference into the string area.
However, sharing string text complicates maintaining the string variables in sorted order.
There are a few situations where it is necessary to make a short search to find the new location for a string variable in the string variable list.

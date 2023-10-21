.ds f. string.3c++
.TH STRING 3C++ "22 August 1985"
.UC 4
.SH NAME
string \- string data class for C++ programs
.SH SYNOPSIS
.ds s \fIstring\fP
.nf
Required include file:
.RS
\fB#include <string.h>\fP
.RE

Compilation command:
.RS
\fBCC\fP  ...  \fB\-lC++\fP
.RE

Variable definition:
.RS
\fBstring\fP \*s [ \fB= \fIchar*\fR | \*s | \fIint\fP ] \fB;\fR
.RE

Available operators:
.RS
.TS
.if \n+(b.=1 .nr d. \n(.c-\n(c.-1
.de 35
.ps \n(.s
.vs \n(.vu
.in \n(.iu
.if \n(.u .fi
.if \n(.j .ad
.if \n(.j=0 .na
..
.nf
.nr #~ 0
.if n .nr #~ 0.6n
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.fc
.nr 33 \n(.s
.rm 80 81
.nr 80 0
.nr 38 \w\fIOperator\fP
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\&=
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w+
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w+=
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w()
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w<<
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w>>
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\&==
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w!=
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w>
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w<
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w>=
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w<=
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 81 0
.nr 38 \w\fIFunction\fP
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wassignment
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wconcatenation
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wconcatenation & assignment
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wsubstring selection or assignment
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \woutput to an \fIostream\fP
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \winput from an \fIistream\fP
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for equality
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for non-equality
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for greater-than
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for less-than
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for greater-than-or-equality
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wtest for less-than-or-equality
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(0*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr TW \n(81
.if t .if (\n(TW+\n(.o)>7.65i .tm Table at line 42 file string.3c++ is too wide - \n(TW units
.fc  
.nr #T 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.ls
..
.ec
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fIOperator\f\n(31\h'|\n(41u'\fIFunction\f\n(31
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\&=\h'|\n(41u'assignment
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'+\h'|\n(41u'concatenation
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'+=\h'|\n(41u'concatenation & assignment
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'()\h'|\n(41u'substring selection or assignment
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'<<\h'|\n(41u'output to an \fIostream\fP
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'>>\h'|\n(41u'input from an \fIistream\fP
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\&==\h'|\n(41u'test for equality
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'!=\h'|\n(41u'test for non-equality
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'>\h'|\n(41u'test for greater-than
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'<\h'|\n(41u'test for less-than
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'>=\h'|\n(41u'test for greater-than-or-equality
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'<=\h'|\n(41u'test for less-than-or-equality
.fc
.nr T. 1
.T# 1
.35
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-17
.RE

Available functions:
.RS
int
\*s.strlen()

char*
\*s.charptr()

int
\*s.index(char c)

int
\*s.rindex(char c)

string
\*s.left(int n)

string
\*s.right(int n)
.RE
.fi
.SH DESCRIPTION
The C++ class \*s implements a dynamic string type similar to that
found in such languages as BASIC, Icon and SNOBOL.
Storage is dynamically maintained for \*s variables so that they will
grow and shrink as needed.
.PP
A \*s variable can be created from an array of characters (or a
character pointer), from another \*s, or from an integer.
In the latter case, the value of the \*s is the ASCII representation
of the integer.
Automatic conversion from any of these basic data types to \*s will
be performed whenever possible.
To wit, whenever one operand of an operator is a \*s and the other
is not the C++ compiler will convert the non-\*s operand to \*s if
possible.
When an explicit conversion is desired, the constructor function
.ce
string(\fIargument\fP)
may be invoked.
This will return a new string and allows such constructs as
`string("Hello").left(10)' to be used.
.PP
The following operators work as you would expect:
.ce
= == != < > <= >=
Note that the assignment operator automatically adjusts the length
of the destination \*s and the amount of memory allocated to it.
The operators
.ce
<< and >>
provide output to an \fIostream\fP and input from an \fIistream\fP
as defined in <stream.h>.
.PP
The ``+'' operator provides \*s concatenation.
A new \*s is created with exactly enough space to hold the
combination of the operand \fIstrings\fP.
Note that the expression
.ce
\*s + \fIint\fP
performs string concatenation and not integer addition.
.PP
The ``+='' operator concatenates the right-hand \*s to the left-hand
\*s and assigns the result to the left-hand \*s.
The following two C++ statements perform the same function:
.RS
.nf
str1 += str2;
str1 = str1 + str2;
.fi
.RE
.PP
A substring may be referenced with the expression
.ce
\*s\fB(\fIstart\fB, \fIend\fB)\fR
where \fIstart\fP and \fIend\fP are the indices of the first and
last characters, respectively.
The index of the first character is zero.
The second argument may be omitted (or -1) to indicate the entire
right hand portion of the \*s.
A substring may be replaced with the expression
.ce
\*s\fB(\fIstart\fB, \fIend\fB, \fInewstring\fB)\fR
where \fIstart\fP and \fIend\fP are the indices of the first and
last characters, respectively, and \fInewstring\fP is a replacement
\*s.
The new \*s and the substring need not be the same length.
If they are not then \*s will be lengthened or shortened as required.
.PP
The \fIstrlen\fP member function returns the number of characters in
a \*s.
.PP
The \fIcharptr\fP member function returns the address of the
embedded character array of a \*s.
Note that this function is intended \fBonly\fP to allow use of \*s
variables with older subroutines requiring character arrays.
For this reason, the character arrays embedded with \*s string
variables are always NULL terminated.
It is not safe to use this address to change or access substrings of
a \*s since the member functions of this class freely deallocate and
reallocate the embedded character arrays.
.PP
The \fIindex\fP member function returns the index (subscript) of the
first occurrence of character \fIc\fP in the \*s.
The \fIrindex\fP member function returns the index of the last
occurrence.
.PP
The member functions \fIleft\fP and \fIright\fP return new
\fIstrings\fP of length \fIn\fP with the original \*s left- or
right-justified, respectively.
.SH EXAMPLES
The following are legal statements or expressions using
\*s variables, functions, and operators:
.RS
.nf
.ta 3i
string foo = "Foo";
string bar;
int ivar;

cout << foo;	/* prints ``Foo'' */
cout << foo << "\en";	/* prints ``Foo\en'' */

bar = foo + " bar";	/* assigns ``Foo bar'' to bar */

cout << bar + 2;	/* prints ``Foo bar2'' */

i = bar.strlen();	/* assigns 7 to ivar */

printf ("%s", foo.charptr());	/* prints foo */

foo = bar(1, 4);	/* assigns "oo b" to bar */
bar(1, 4, " Hello! ");	/* changes bar to "F Hello! ar" */

foo = "a";	/* assigns ``a'' to foo */
bar = "b";	/* assigns ``b'' to bar */
(a == b)	/* evaluates to 0 */
(a < b)	/* evaluates to 1 */
.SH FILES
/usr/include/CC/string.h
.br
/usr/include/CC/stream.h
.SH "SEE ALSO"
The C++ documentation from AT&T.
.SH AUTHOR
Arthur Zemon, FileNet Corp.
.SH BUGS
The current implementation was not built with run-time efficiency in
mind.
There is undoubtedly much room for improvement.
.PP
The \*s library beats the default memory allocation routines
(\fBnew\fP and \fBdelete\fP, which in turn call \fImalloc(3)\fP and
\fIfree(3)\fP) to death.
.PP
Since there are some inconsistencies in the code generated by the
current C++ compiler, a small amount of heap space is often ``lost''
during the life of a program.
.PP
The current C++ compiler will not allow static \*s variables.

.ll 78
.de CS
.ta 5
.ti -4
\\$1\t\\
..
This list presents the differences between the Pascal compiler (version 25)
of april 1979 and the next version of june 1979.
The compiler is adopted to conform to the ISO Pascal-standard proposal
of february 1979.
.br
Changes:
.in +4
.CS 1
The procedure and function headings of procedural and functional parameters
must be specified completely.
.CS 2
All upper case letters are mapped onto the corresponding lower case letters.
External proc/func names with upper case letters are not possible anymore.
.CS 3
A packed array of characters is only compatible with a constant
string if the lower bound is 1.
.CS 4
A file of char is NOT equivalent to the standard type text.
The standard functions read(ln) and write(ln) only apply to files of type text.
.CS 5
The control variable of a for statement must be a local variable.
.CS 6
The procedure write is changed somewhat:
.in +4
.ti -2
string: if the width specifier w is less than the string length, then
only the first w characters are written.
.ti -2
Boolean: true is printed as 'true ', not 'true'.
.ti -2
real: write(3.141596:8) gives ' 3.1e+00',
.br
write(-3.141596:11) gives '-3.1416e+00' etc.
.ti -2
C-type strings: these strings may be written. Their length will be computed
by scanning for the terminating zero-byte.
.in -4
.CS 7
The only deviation from the standard is the required get(input).
However, the file window is initialized with a space, while eoln(input)
initially yields true, so that readln(input) may also be used to initialize
the window.
If the first input item is a number, no extra initializing is necessary.
Reset(input) is equivalent to get(input).
For an extensive treatment of the problem see the reference manual /usr/doc/pc/ref_manual/*.
.CS 8
Error messages are improved.
.CS 9
The compiler issues a warning for each variable which is not used or set.
The same applies if there is no assignment to a function.
.CS 10
There is a new option for the compiler, the s-option.
If on then all non-standard features used in your program will be flagged
with a warning. Default off.
.CS 11
Sets may be packed into a byte if the base-type contains only 8 elements or less.
.CS 12
It is not allowed to use elements of packed structures as call by reference
parameter.
.CS 13
The symbols ':' and '..' are different. '..' must be used in subranges and
in set constants; ':' is used everywhere else.
.CS 14
The words 'extern', 'forward' and 'assert' are not reserved anymore.
.CS 15
The syntax of record-type and case-statement is slightly modified:
multiple semicolons are not allowed anymore.
.CS 16
Not only type-identifiers are allowed to specify parameter types, but
arbitrary types are allowed as well.
However, using this feature is not recommended, because it is likely
that it will be forbidden in the final standard.
.CS 17
A call of 'page' is equivalent to 'page(output)'.
.CS 18
The upper case letter 'E' is recognized as scale factor indicator for reals.
.CS 19
Tabs are skipped when reading integers and reals.
.CS 20
A number in a Pascal program may not be followed directly by an identifier.
They must be separated by at least one separator like space, tab, comment
or newline.

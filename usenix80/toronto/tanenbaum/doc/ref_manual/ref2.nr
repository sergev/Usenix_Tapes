.CH "Implementation-defined features"
For each implementation-defined feature mentioned in the ISO standard
we give the section number, the quotation from that section and the definition.
First we quote the definition of implementation-defined:
.DS
Those parts of the language which may differ between processors, but which
will be defined for any particular processor.
.DE
.IT 6.4.2.2
The predefined integer constant maxint, whose value shall be
implementation-defined, shall define the subset of the integers available
in any implementation over which the integer operations are defined.
.IS
The representation of integers in EM1 is a 16-bit word using
two's complement arithmetic.
Thus always:
.EQ
     maxint = 32767
.EN
Because the number -32768 may be used to indicate 'undefined', the
range of available integers depends on the EM1 implementation:
.I1
\*(ON-32767..+32767.
.br
\*(OF-32768..+32767.
.I2
-32768..+32767.
.IE
.IT 6.4.2.2
The values of type real shall be an implementation-defined subset
of the real numbers as specified by 6.1.5.
.IS
The format of reals is not defined in EM1. It is only defined that a real
number occupies 4 bytes (32 bits) of storage.
For each implementation of EM1 the following constants must be defined:
     epbase: the base for the exponent part
     epprec: the precision of the fraction
     epemin: the minimum exponent
     epemax: the maximum exponent
.br
These constants must be chosen so that zero and all numbers with
exponent e in the range
.EQ
     epemin <= e <= epemax
.EN
and fraction-parts of the form
.EQ
     f = +_ f\d1\u.b\u-1\d + ... + f\dp\u.b\u-p\d
.EN
where
.EQ
     f\di\u = 0,...,epbase-1 and f\d1\u <> 0
.EN
are possible values for reals.
All other values of type real are considered illegal.
(See [3] for more information about these constants).
.br
For the known EM1 implementations these constants are:
.I1
epbase = 2
.br
epprec = 24
.br
epemin = -127
.br
epemax = +127
.I2
idem
.IE
.IT 6.4.2.2
The values of type char shall be an implementation-defined set of
characters.
.IS
The 7-bits ASCII character set is used, where LF (10) denotes the
end-of-line marker.
.IT 6.4.3.4
The largest and smallest values permitted in the base-type of a
set-type shall be implementation-defined.
.IS
A value v is permitted if
.EQ
     0 <= ord(v) <= 255.
.EN
This restriction is posed by the Pascal compiler, not by the EM1 machine,
which is able to support sets of 65536 bits easily.
The compiler allocates as many bits for set-type variables as are necessary
to store all possible values of the host-type of the base-type of the set,
rounded up to the nearest multiple of 16 (if 8 bits are sufficient then only
8 bits are used if part of a packed structure).
So, the variable s, declared by
.EQ
     var s: set of '0'..'9';
.EN
will contain 128 bits, not 10 or 16.
If the host-type of the base-type is integer, then the
number of bits depends on the i-option.
The programmer may specify how many bits to allocate for these sets.
The default is 16, the maximum is 256.
The effective number of bits is rounded up to the next multiple of 16, or up
to 8 if the number of bits is less or equal to 8.
.IT 6.6.6.4
If the parameter of ord(x) is of type char, the result shall
be implementation-defined.
.IS
The result of ord(x) for x of type char will be conform the ASCII character set:
.EQ
     0 <= ord(x) <= 127
.EN
.IT 6.7.2.2
The result of i DIV j for i < 0 and for j < 0 shall
be implementation-defined.
.IS
The Pascal DIV operator is translated into an EM1 DIV instruction.
Because EM1 is defined by an interpreter written in Pascal, there is a cycle.
The current EM1-implementations, all for a PDP-11, use the PDP DIV instruction
to implement the EM1 DIV instruction.
The Pascal Processor Handbook defines the DIV instruction such that the
remainder is of the sign as the dividend. Therefore:
.EQ
      14 DIV  5 =  2        14 MOD  5 =  4
      14 DIV -5 = -3        14 MOD -5 = -1
     -14 DIV  5 = -2       -14 MOD  5 = -4
     -14 DIV -5 =  3       -14 MOD -5 =  1
.EN
.IT 6.9.4
The default field-width values for integer, Boolean and real types
shall be implementation-defined.
.IS
The defaults are:
     integer    6
     Boolean    5
     real      13
.IT 6.9.4
The number of digits written in an exponent part of a real shall
be implementation-defined.
.IS
The number of digits in the exponent part is defined as
.EQ
     ceil(log10(log10(2 ** epemax)))
.EN
For the current implementations this evaluates to 2.

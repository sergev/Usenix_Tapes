.br
.TL "Pascal-VU reference manual"
.AU "Johan W. Stevenson"
.DA "May 29, 1979"
.VU
.CH "Introduction"
This document refers to the ISO standard proposal for Pascal,
ISO/TC97/SC5-N462 [1], dated february 1979.
Pascal-VU complies with the requirements of this proposal as much as possible.
The standard requires an accompanying document describing the
implementation-defined and implementation-dependent features,
the reaction on errors and the extensions to standard Pascal.
These four items will be treated in the rest of this document,
each in a separate chapter.
The other chapters describe the deviations from the proposed standard and
the list of options recognized by the compiler.
.PP
The Pascal-VU compiler produces code for an EM1 machine as defined in [2].
It is up to the implementor of the EM1 machine whether errors like
integer overflow, undefined operand and range bound error are recognized or not.
Therefore it depends on the EM1 machine implementation whether these errors
are recognized in Pascal programs or not.
For these errors the reaction of all known implementations is given.
.PP
There does not (yet) exist a hardware EM1 machine.
Therefore, EM1 programs must be interpreted, or translated into
instructions for a target machine.
The following implementations currently exist:
.I1
an interpreter running on a PDP-11 (using UNIX-6).
Normally the interpreter performs some tests to detect undefined
integers, integer overflow, range errors, etc.
However, one of the options of the interpreter is to skip these tests.
We will refer to these modes of operation as 'test on' and 'test off'.
.I2
a translator into PDP-11 instructions (using UNIX-6).
.IE

.CH "Extensions to the standard"
.IS
.LB 1.
The compiler is able to compile modules, not forming complete programs, but
consisting of procedures and functions which can be used to form libraries.
The syntax of these modules is
.EQ
     module = [constant-definition-part]
              [type-definition-part]
              [procedure-and-function-declaration-part]
.EN
The compiler accepts a program or a module:
.EQ
     unit = program | module
.EN
It is only allowed to declare variables local to the procedures
and functions.
All variables outside these procedures and functions must be imported
and exported by parameters, even the files input and output.
By giving the correct procedure/function heading followed by the
directive 'extern' you may use procedures and functions declared in
other units.
.sp
.LB 2.
The Pascal-VU compiler recognizes an additional statement, the assertion.
Assertions can be used as an aid in debugging and documentation.
The syntax is:
.EQ
     assertion = ASSERT Boolean-expression
.EN
An assertion is a simple-statement, so
.EQ
     simple-statement = [assignment-statement |
                         procedure-statement |
                         goto-statement |
                         assertion
                        ]
.EN
An assertion causes an error if the Boolean-expression is false.
That is its only purpose.
It does not change any of the variables, at least it should not.
Therefore, do not use functions with side-effects in the Boolean-expression.
If the a-option is turned off, then assertions are skipped by the compiler.
ASSERT is not a word-symbol (keyword) and may be used as identifier.
However, assignment to a variable and calling of a procedure with that name will be impossible.
.sp
.LB 3.
Three additional standard procedures are available:
.IS
.IS
.ti -8
halt:~~~a call of this procedure is equivalent to jumping to the
end of your program. It is always the last statement executed.
.ti -8
release:
.ti -8
mark:~~~for most applications it is sufficient to use the heap as second stack.
Mark and release are suited for this kind of use, more suited than dispose.
mark(p), with p of type pointer, stores the current value of the
heappointer in p. release(p), with p initialized by a call
of mark(p), restores the heappointer to its old value.
All the heap objects, created by calls of new between the call of
mark and the call of release, are removed and the space they used
can be reallocated.
Never use mark and release together with dispose!
.sp
.in -10
.LB 4.
If the c-option is turned on, then some special features are available
to facilitate an interface with the UNIX environment.
First of all, the compiler allows you to use a different type
of string constants.
These string constants are delimited by double quotes ('"').
To put a double quote into these strings, you must repeat the double quote,
like the single quote in normal string constants.
These special string constants are terminated by a zero byte (chr(0)).
The type of these constants is a pointer to a packed array of characters,
with lower bound 1 and unknown upper bound.
.br
Secondly, the compiler predefines a new type identifier 'string' denoting
this just described string type.
.PP
The only thing you can do with these features is declaration of
constants and variables of type 'string'.
String objects may not be allocated on the heap and string pointers
may not be de-referenced.
Still these strings are very useful in combination with external routines.
.sp
.LB 5.
The character '_' may be used in forming identifier, except as first character.
.sp
.LB 6.
The comment delimiters '{' and '}' may be nested.
.sp
.LB 7.
Comments delimited by '(*' and '*)' are recognized as well, but may not be nested.

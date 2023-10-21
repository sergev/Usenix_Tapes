.CH "Implementation-dependent features"
For each implementation-dependent feature mentioned in the ISO standard draft,
we give the section number, the quotation from that section and the way
this feature is treated by the Pascal-VU system.
First we quote the definition of 'implementation-dependent':
.DS
Those parts of the language which may differ between processors,
and for which there need not be a definition for a particular processor.
.DE
.IT 6.6.1
The full set of directives permitted after a procedure-heading
shall be implementation-dependent.
.IS
Except for the required directive 'forward' the Pascal-VU compiler recognizes
only one directive: 'extern'.
This directive tells the compiler that the procedure block of this
procedure will not be present in the current program.
The code for the body of this procedure must be included at a later
stage of the compilation process.
.PP
This feature allows one to build libraries containing often used routines.
These routines do not have to be included in all the programs using them.
Maintainance is much simpler if there is only one library module to be
changed instead of many Pascal programs.
.PP
Another advantage is that these library modules may be written in a different
language, for instance C or the EM1 assembly language.
This is useful if you want to use some weird EM1 instructions not generated
by the Pascal compiler. Examples are the system call routines and some
floating point conversion routines.
Another motive could be the optimization of some time-critical program parts.
.PP
The use of external routines, however, is dangerous.
The compiler normally checks for the correct number and type of parameters
when a procedure is called and for the result type of functions.
If an external routine is called these checks are not sufficient,
because the compiler can not check whether the procedure heading of the
external routine as given in the Pascal program matches the actual routine
implementation.
It should be the loaders task to check this.
However, the current loaders are not that smart.
Another solution is to check at runtime, at least the number of words
for parameters. Some EM1-implementations check this:
.I1
\*(ONthe number of words passed as parameters is checked, but this will not catch all faulty cases.
.br
\*(OFnot checked.
.I2
not checked.
.IT 6.6.2
The full set of directives permitted after a function-heading
shall be implementation-dependent.
.IS
The same as for procedures.
.IT 6.6.5.1
The effect of using standard procedures as procedural parameters
shall be implementation-dependent.
.IS
It is not allowed to use standard procedures as procedural parameters.
.IT 6.6.5.2
If an activation of the procedure put(f) is not separated
dynamically from a previous activation of get(f) or reset(f) by an
activation of rewrite(f), the effect shall be implementation-dependent.
.IS
put(f) is only allowed when the file is opened for writing by a call to
rewrite(f).
get(f) is only allowed when the file is opened for reading for writing by a
call to reset(f).
Therefore appending to a existing file using the standard IO routines only
is rather clumsy.
.IT 6.6.6.1
The effect of using standard functions as functional parameters
shall be implementation-dependent.
.IS
It is not allowed to use standard functions as functional parameters.
You can obtain the same result with negligible loss of performance
by declaring some user routines like:
.EQ
     function sine(x:real):real;
     begin
         sine:=sin(x)
     end;
.EN
.IT 6.7.2.1
The order of evaluation of the operands of a binary operator
shall be implementation-dependent.
.IS
The left-hand operand of a binary operator is almost always evaluated
before the right-hand side.
Some peculiar evaluations exist for the following cases:
.IS
.LB 1.
the term
.EQ
     factor1 / factor2
.EN
where factor1 and factor2 are both of type integer is evaluated in the
following steps:
.IS
.LB -
evaluate factor1 into an integer result
.LB -
evaluate factor2 into an integer result
.LB -
convert factor1 to real
.LB -
convert factor2 to real
.LB -
divide
.IE
.sp
.LB 2.
the expression
.EQ
     set1 <= set2
.EN
where set1 and set2 are compatible set types is evaluated in the
following steps:
.IS
.LB -
evaluate set2
.LB -
evaluate set1
.LB -
compute set2+set1
.LB -
test set2 and set2+set1 for equality
.IE
This is the only case where the right-hand side is computed first.
.sp
.LB 3.
the expression
.EQ
     set1 >= set2
.EN
where set1 and set2 are compatible set types is evaluated in the following steps:
.IS
.LB -
evaluate set1
.LB -
evaluate set2
.LB -
compute set1+set2
.LB -
test set1 and set1+set2 for equality
.IE
.IT 6.7.2.3
Whether a Boolean expression is completely or partially evaluated
if its value can be determined by partial evaluation shall be
implementation-dependent.
.IS
Boolean expressions are always evaluated completely, so the program part
.EQ
     if (p<>nil) and (p^.value<>0) then
.EN
is probably incorrect.
.IT 6.7.3
The order of evaluation and binding of the actual-parameters for functions
shall be implementation-dependent.
.IS
The order of evaluation and binding is from left to right.
.IT 6.8.2.2
If the selection of the variable in an assignment-statement
involves the indexing of an array or the de-referencing of a pointer,
the decision whether these actions precede or follow the evaluation
of the expression shall be implementation-dependent.
.IS
The expression is evaluated first.
.IT 6.8.2.3
The order of evaluation and binding of the actual-parameters for procedures
shall be implementation-dependent.
.IS
The same as for functions.
.IT 6.10
The effect of an explicit use of reset or rewrite on the standard
files input and output shall be implementation-dependent.
.IS
rewrite(output) is a noop. reset(input) is equivalent to get(input).
See also the chapter on deviations from the standard.

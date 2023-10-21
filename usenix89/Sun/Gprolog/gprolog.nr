.ls 2
.de hd
'sp 2
.tl ''- Page % -''
'sp 2
..
.de fo
'bp
..
.wh -6 fo
.de NP
.sp 2
.ti +5
..
.de DW	Day of the week into text
.if \\n(dw-6 .if !\\n(dw-7 Saturday\ \c
.if \\n(dw-5 .if !\\n(dw-6 Friday\ \c
.if \\n(dw-4 .if !\\n(dw-5 Thursday\ \c
.if \\n(dw-3 .if !\\n(dw-4 Wednesday\ \c
.if \\n(dw-2 .if !\\n(dw-3 Tuesday\ \c
.if \\n(dw-1 .if !\\n(dw-2 Monday\ \c
.if \\n(dw   .if !\\n(dw-1 Sunday\ \c
..
.de YR
19\\n(yr
..
.de DY
\\n(dy,\ \c
..
.de MO	Month of the year into text
.if \\n(mo-11 .if !\\n(mo-12 December\ \c
.if \\n(mo-10 .if !\\n(mo-11 November\ \c
.if \\n(mo-9  .if !\\n(mo-10 October\ \c
.if \\n(mo-8  .if !\\n(mo-9  September\ \c
.if \\n(mo-7  .if !\\n(mo-8  August\ \c
.if \\n(mo-6  .if !\\n(mo-7  July\ \c
.if \\n(mo-5  .if !\\n(mo-6  June\ \c
.if \\n(mo-4  .if !\\n(mo-5  May\ \c
.if \\n(mo-3  .if !\\n(mo-4  April\ \c
.if \\n(mo-2  .if !\\n(mo-3  March\ \c
.if \\n(mo-1  .if !\\n(mo-2  February\ \c
.if \\n(mo    .if !\\n(mo-1  January\ \c
..
.de DT	Print date: Month Day, Year
.nf
'MO
'DY
'YR
.fi
..
.sp 10
.ce 5
GProlog Users Manual
.sp 2
Barry Brachman
.sp 2
Department of Computer Science
.sp 2
University of British Columbia
.sp 2
Vancouver, B.C., V6T 1W5
.DT
.bp 1
.sp 2
.ul 1
INTRODUCTION
.NP
This manual describes the modifications applied to
C-Prolog [2] on the SUN workstation to allow calling of functions from
the SunCore library [3].
.wh 0 hd
The SunCore library is SUN Microsystems' implementation of the CORE
graphics standard.
Differences between the enhanced C-Prolog interpreter
(which we will refer to as GProlog) and standard C-Prolog will be explained.
.NP 
C-Prolog is an interpreter for the Prolog language [1].
It is written in C and runs under UNIX 4.2 BSD.
The GProlog interpreter is a version of C-Prolog incorporating the
SunCore library.
All but a few of the approximately 200 SunCore functions are
currently callable from GProlog.
Routines in the SunCore library are callable only from C or from FORTRAN-77.
C-Prolog programs will run identically under GProlog, with the exception
of programs which rely on C-Prolog's conversion of results which
are real numbers into integers.
(C-Prolog will unify 3 with 3.0
and convert 3.0 to 3 in arithmetic computations.)
The greeting message printed by GProlog identifies it so that the
user is aware of the version of Prolog being used.
.sp 2
.ul 1
Starting Up GProlog
.NP
If the graphics capabilities of GProlog are to be used, the user
must be signed on at the console; otherwise any terminal may be used.
The console is a monochrome bit-mapped display.
GProlog may currently be used in conjunction with
either the "bw1dd" viewing
surface or the "pixwindd" viewing surface (there is no color display device
at UBC).
The former uses the entire display for both conversation with
the interpreter and graphics output.
The latter viewing surface, which is used with the Suntools windowing
package [4], is recommended since separate text and graphics subwindows are
available: interaction with the interpreter does not disturb the
graphics.
GProlog may be started
after a "GRAPHICS-TOOL" window has been created. 
GProlog may be found in "/usr2/brachman/bin/gprolog".
Note that many operations on a window (e.g., moving it) destroy the
graphics part of the window, although the text subwindow is preserved.
Therefore the user should adjust the size of the subwindows before
displaying any graphics.
.sp 2
.ul 1
Graphics Extensions
.NP
The evaluable predicate 'plgraphics' is used to invoke a SunCore function.
The form of a call is:
.sp 1
.ti +10
plgraphics(<SunCore function name>)
.sp 1
or
.sp 1
.ti +10
plgraphics(<SunCore function name>(<arg1>,...,<argn>))
.sp 1
For convenience, a prefix operator has been built into the system:
.sp 1
.ti +10
#<SunCore function name>
.sp 1
or
.sp 1
.ti +10
#<SunCore function name>(<arg1>,...,<argn>)
.sp 1
The <SunCore function name> is any SunCore function which
appears in the SunCore User's Manual [3],
with the exceptions listed below.
For example,
.ti +10
.sp 1
#move_abs_2(0.0,0.0)
.sp 1
will call the SunCore function 'move_abs_2' with the two arguments.
.NP
Arguments which are vectors in the SunCore documentation are
represented as lists in GProlog.
Data is returned to Prolog predicates through uninstantiated variables.
In the current implementation, when a value is returned the corresponding
argument in the Prolog predicate must be uninstantiated.
For example, the SunCore function 'polyline_rel_2',
which draws a sequence of line
segments given their endpoints, would be called in this manner:
.sp 1
.in +10
#polyline_rel_2([1.0,3.0,4.0,5.0],[1.0,2.0,3.0,4.0],4)
.in -10
.sp 1
or equivalently,
.sp 1
.in +10
X = [1.0,3.0,4.0,5.0],
.br
Y = [1.0,2.0,3.0,4.0],
.br
#polyline_rel_2(X,Y,4)
.in -10
.sp 1
The first argument to 'polyline_rel_2' is a list of the X coordinates,
the second argument is a list of the Y coordinates, and the third
argument is the number of coordinate pairs.
.NP
The function 'inquire_current_position_2', which returns the two-dimensional
world coordinates of the current position, would be called with the
arguments uninstantiated:
.sp 1
.in +10
#inquire_current_position_2(X,Y)
.in -10
.sp 1
The variable X will be unified with the X coordinate and Y will be unified
with the Y coordinate.
Note that since the SunCore documentation specifies that X and Y are
real numbers, GProlog guarantees that the values will stay real.
.NP
A number of manifest constants used by SunCore appear in the include
file "/usr/include/usercore.h".
To give the GProlog programmer access to these constants, they have
been made part of GProlog.
The value of a symbolic constant is obtained by a call of the form:
.sp 1
.ti +10
#def(<symbolic name>,<value>)
.sp 1
where <symbolic name> is a defined constant appearing in
the SunCore User's Manual.
Note that the <symbolic name> must be in lowercase even though it
appears in uppercase in the manual.
For example,
.sp 1
.ti +10
#def(false,X)
.sp 1
will unify X with the value of the constant 'false'.
The second argument to 'def' should be an uninstantiated variable.
.NP
The special function name 'list' may be used to list each SunCore
function in the system along with its arity.
Since there are around 200 functions, the 'list' command tries to
print the commands in a table with as many columns as possible.
Several test functions, not for general use, are also part of GProlog.
.NP
The GProlog function 'getenv(X, Y)' may be used to determine the value
of an environment variable.
The first argument, the name of the environment variable, is a string.
The second argument, an uninstantiated variable, is unified with the value
of the environment variable.
The function will fail if the environment variable is not found.
For example,
.sp 1
.ti +10
#getenv("TERM", T)
.sp 1
will unify T with the name of the terminal being used.
.NP
Several of the SunCore functions have not been included in GProlog.
This is because they are more difficult to implement and also because
they would not appear to be frequently used.
They could be added to GProlog in the future.
The following functions are not available:
.sp 1
.in +10
.ls 1
set_world_coordinate_matrix_2
.br
set_world_coordinate_matrix_3
.br
set_viewing_parameters
.br
inquire_viewing_parameters
.br
inquire_world_coordinate_matrix_2
.br
inquire_world_coordinate_matrix_3
.br
inquire_inverse_coordinate_matrix
.br
inquire_retained_segment_surfaces
.br
put_raster
.br
get_raster
.br
size_raster
.br
allocate_raster
.br
free_raster
.br
raster_to_file
.br
file_to_raster
.br
set_primitive_attributes
.br
inquire_primitive_attributes
.br
await_stroke_2
.in -10
.ls 2
.NP
A call to a SunCore routine requires that GProlog convert any
arguments from their Prolog representation to the representation
required by the C calling conventions.
Prolog list structures, for example, must be converted to
vectors where the elements are contiguous in memory.
Calls to SunCore routines will fail
within the interface between GProlog and SunCore
if the number of arguments is incorrect or if the type of any
argument is incorrect.
In either case GProlog will display an appropriate diagnostic.
If the failure occurs within the SunCore routine itself
(e.g., an argument is out-of-range), SunCore prints a message and the
predicate fails.
(Aside: construction of this interface routine was complicated by
the absence of any documentation describing the data structures used
within C-Prolog.)
.NP
Note that the maximum length of a string passed to a SunCore
function is 255 characters.
.sp 2
.ul 1
Other Differences
.NP
Several changes were made to C-Prolog
in addition to the modifications described above.
GProlog does not
convert floating point numbers to integers.
This is because the user would expect a vector of floating point numbers
to stay as floating point numbers.
C-Prolog converts those that it
can to integers.
Therefore, GProlog will not unify 3 with 3.0.
Also, if any of the intermediate results during evaluation of the "is"
predicate is a real, then the result is a real.
The exception to this is the new evaluable predicate "int" which always
returns an integer value.
.NP
The following table lists each numeric evaluable predicate and its associated
type:
.sp 1
.in +10
.ls 1
.ul 1
Function/Operator               Type of Result
.sp 1
unary -,+                       Same as operand
.br
\\                               Always integer
.br
exp,log,log10,                  Always real
.br
sqrt                              "     "
.br
sin,cos,tan,asin,acos,atan        "     "
.br
floor,ceil                      Same as operand
.br
abs                             Same as operand
.br 
int                             Always integer
.br
float                           Always real
.br
+,-,*                           Integer if both operands are integers; otherwise real
.br
/                               Always real
.br
mod,/\\,\\/,<<,>>,//              Always integer
.br
^                               Always real
.br
.ls 2
.in -10
.NP
Four (unary) numeric evaluable predicates have been added to GProlog:
.sp 2
.in +10
.ls 1
abs   - absolute value
.br
int   - rounds argument to nearest integer
.br
float - converts argument to floating point
.br
ceil  - ceiling function
.sp 2
.ls 2
.in -10
.NP
For example, the result of
.sp 1
.in +10
X is 4 + 3.0.
.sp 1
.in -10
is X = 7.0.
The result of
.sp 1
.in +10
X is cos(0).
.sp 1
.in -10
is X = 1.0.
.sp 2
.ul 1
Some Errors in C-Prolog and its Documentation
.NP
The following errors in the C-Prolog (and GProlog)
interpreter and the C-Prolog documentation [2]
have been observed:
.in +10
.sp 1
1. C-Prolog will not accept
'in +3
consecutive plus or minus operators that
are not separated by space or by parentheses; e.g.,
X is 0--1.
When printed, however, the correct version is identical to
the incorrect version; e.g.,
"X = 0--1, Y is X." will not succeed while
"X = 0-(-1), Y is X ." will succeed with X = 0--1 and Y = 1.
'in -3
.sp 1
2. It is possible for C-Prolog to 
'in +3
enter a state where only a process
stop signal (default: control-z) allows the user to regain control.
The session is not recoverable.
The conditions under which this occurs are not yet known.
'in -3
.sp 1
3. Referring to Section 5.2 of the manual,
'in +3
given "X is 2*1.5.",
C-Prolog should give a result of 3 (GProlog will give 3.0).
'in -3
.sp 1
4. Referring to Section 5.2 of the manual,
'in +3
"expanded_exprs(Old,New)"
should be replaced by "expand_exprs(Old,New)".
'in -3
.sp 1
5. By interrupting GProlog while a SunCore
'in +3
routine is being executed,
the user may damage the window in which GProlog is running.
This seems to be due to a locking mechanism which GProlog and SunCore
use to communicate.
.in -3
.sp 1
6. Single line type comments may be used.
'in +3
They are preceded by a '%'.
'in -3
.in -10
.sp 2
.ul 1
Further Work
.NP
The remaining SunCore routines should be implemented.
An on-line help facility for use with SunCore might be useful.
.de hd
'sp 4
..
.bp
.ul 1
References
.sp 2
1.  Clocksin, W. F. and Mellish, C. S.,
'in +4
.ul 1
Programming in Prolog,
Springer-Verlag, 1981.
.in -4
.sp 1
2.  Pereira, Fernando (Ed.),
'in +4
.ul 1
C-Prolog User's Manual,
Version 1.4,
University of Edinburgh, Department of Architecture, January 19, 1984.
'in -4
3.  Sun Microsystems, Inc.,
'in +4
.ul 1
Programmer's Reference Manual for SunCore,
Revision D of November 1, 1983 for Sun System Release 1.0.
'in -4
.sp 1
4.  Sun Microsystems, Inc.,
'in +4
.ul 1
UNIX Programmer's Manual,
4.2 BSD, Section I,
Revision D of November 1, 1983 for Sun System Release 1.0.
'in -4
.sp 1

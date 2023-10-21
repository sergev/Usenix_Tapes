.CH "Compiler options"
Some options of the compiler may be controlled by using
"{$....}" or "(*$....*)".
Each option consists of a lower case letter followed by +, - or an unsigned
number.
Options are separated by commas.
The following options exist:
.in 8
.ta 9
.sp
.ti -8
a +/-	this option switches assertions on and off.
If this option is on, then code is included to test these assertions
at runtime. Default +.
.sp
.ti -8
c +/-	this option, if on, allows you to use C-type string constants
surrounded by double quotes.
Moreover, a new type identifier 'string' is predefined.
Default -.
.sp
.ti -8
i <num>	with this flag the setsize for a set of integers can be
manipulated.
The number must be the number of bits per set.
The default value is 16, just fitting in one word on the PDP and a lot of other minis.
.sp
.ti -8
l +/-	if + then code is inserted to keep track of the source line number.
When this flag is switched on and off, an incorrect line number may appear
if the error occurs in a part of your program for which this flag is off.
These same line numbers are used for the profile, flow and count options
of the EM1 interpreter em1 [6].
Default +.
.sp
.ti -8
r +/-	if + then code is inserted to check subrange variables against
lower and upper subrange limits.
Default +.
.sp
.ti -8
s +/-	if + then the compiler will hunt for places in your program
where non-standard features are used, and for each place found
it will generate a warning. Default -.
.sp
.ti -8
t +/-	if + then each time a procedure is entered, the routine 'procentry'
is called.
The compiler checks this flag just before the first symbol that follows the
first 'begin' of the body of the procedure.
Also, when the procedure exits, then the procedure 'procexit' is called
if the t flag is on just before the last 'end' of the procedure body.
Both 'procentry' and 'procexit' have a packed array of 8 characters as a parameter.
Default procedures are present in library libP [7].
Default -.
.sp
.ti -8
v +/-	if + then 32 bit addresses are produced for an EM1 machine with
256 data segments of 64k bytes each.
Default -.
.in 0
.sp
Three of these flags (c,i and v) are only effective when they appear
before the 'program' symbol. The others may be switched on and off.
.PP
Instead of including the options in your Pascal program, you may also set
them by giving a flag argument with the same syntax to pc [4].
Pc passes this flag to the compiler by using the intermediate file
which is also used, in reverse direction, for the error messages in condensed form.
Options given to pc override the options in your program.
This feature is very useful for debugging.
Without changing any character in your program you may, for instance, include
code for procedure/function tracing.
.PP
Another very powerful debugging tool is the knowledge that inaccessable
statements and useless tests are removed by the EM1 optimizer.
For instance, a statement like:
	if debug then
	  writeln('initialization done');
.br
is completely removed by the optimizer when debug is a constant with
value false.
The first line is removed if debug is a constant with value true.
Of course, if debug is a variable nothing can be removed.
.PP
One of the disadvantages of Pascal, the lack of preinitialized data, can be
diminished by making use of the possibilities of the EM1 optimizer.
For instance, initializing an array of reserved words is sometimes
optimized into 3 EM1 instructions. To maximize this effect you must initialize
variables as much as possible in order of declaration and array entries
in order of increasing index.

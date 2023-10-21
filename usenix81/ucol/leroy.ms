.EQ
delim %%
.EN
.PP
.DA
.ds LH CIRES
.ds RH "Computing Facility
.ds LF "\fBleroy\fP guide
.ds RF "Version 1.2
.sp 4
.DS C
.LG
.LG
.B
leroy
.R
.sp 2
.SM
A user's guide to the
CIRES drafting package
.NL
.DE
.sp 3
.NH
Introduction
.PP
.B Leroy
is a high-level language for the production
of publication-quality figures.
It offers access to a wide range of
available device features
(such as colored pens on the HP7221A)
and to numerous Hershey fonts.
.B Leroy
has a versatile command syntax which allows symbol definition
and arithmetic;
figures are not only easy to create but,
more importantly,
easy to change.
.B Leroy
lacks many control features such as looping and conditional
execution
(\fBfor, while, if, else\fP, \fIetc.\fP).
It will probably stay that way.
.PP
.B Leroy
is based upon a set of plotting routines written
by Danny Harvey
which offer control over such features as line thickness,
string centering,
and repeat count for darkening.
These routines produce an extended
.UX
plot code which,
.I via
the appropriate device filters,
can be sent to any of the currently available
graphics devices.
The high-level portion of
.B leroy
is implemented with
.B yacc
and
.B lex
and is due to Martin Smith.
.NH
Usage
.PP
Input to
.B leroy
consists of lines in the command language described below.
By default
.B leroy
seeks instructions from the standard input.
Most commonly, however, input will be from a file.
If
.B figs
contains a
.B leroy
figure specification,
the line
.DS B
.B
leroy figs
.DE
will cause
.B leroy
to go to work on the contents of
.B figs
and produce a stream of
.UX
plot commands.
This stream is sent by default to the file
.B leroy.out
in the current directory;
.B leroy
commands are available to direct it to some other file
or (someday) to a pipe.
.PP
The
.UX
plot file can be displayed on the desired device using
the extended
.B plot(1)
filters.
If, for example,
.B figs.out
was produced by the above invocation of
.B leroy,
the figure could be viewed on the tektronix by
.DS B
.B
plot -Ttek figs.out
.DE
or sent to the versatec with
.DS B
.B
plot -Tver figs.out .
.DE
At CIRES the final version would usually end up on the
HP7221A, which can be achieved with
.DS B
.B
plot -Thp figs.out .
.DE
The hard-core can even login on the Qume and use
.DS B
.B
plot -T450 figs.out
.DE
though it's hardly worth it.
.PP
The plot stream \fBleroy\fP emits is intended for, and must be
directed to, an extended
.UX
plot filter.
These filters recognize a number of commands not included in
the vanilla seventh edition filters,
mostly dealing with the Hershey fonts.
The extended plot filters are upward compatible with their predecessors
and have been implemented in a fashion which makes tailoring
new devices filters very straightforward.
The source code is available on request.
Filters currently exist for the Tektronix 4014,
the Versatec, the HP7221A, and the Qume.
.NH
Source conventions
.NH 2
Input syntax
.PP
Input to
.B leroy
consists of interspersed command lines and text.
Any line starting with a period, `.',
followed immediately by a keyword
(see below and Table 1)
is a command line and is parsed by
.B leroy.
Any line
.I not
starting with a period is text
and is plotted at the current position,
in the current font, color, size and orientation.
A line starting with `.&' is a continuation of an immediately
preceding command or continuation line.
Text and comment lines may not be continued.
Any line starting with a period which is \fBnot\fP followed immediately
by either a legal keyword (including synonyms) or a `&' causes
\fBleroy\fP to expire gracelessly.
\fBLeroy\fP wholly ignores blank lines.
.PP
If a line of text begins with a `|',
that character is immediately stripped off.
Thus to produce a line of text starting with `.',
start the line with `|.';
to produce a line starting with `|',
start it with `||'.
A line of text includes all characters from the initial character,
which starts after any leading `|', up to but not including a trailing
newline.
Any line of text which was not preceded by a leading `|', and
any variable name,
which has both a leading and a trailing double quote, `"', will
be stripped of both.
This `feature' is included to sneak pathnames such as
.DS B
.B
"/we/must_be/mad"
.DE
past the parser.
To get a line of text quoted on the figure try
.DS B
.B
|"Gadzooks"
.DE
or
.DS B
.B
""Zounds"".
.DE
.PP
As Table 1 indicates,
.B leroy
commands are generally followed by a file name, an
.I expr,
a
.I point,
an
.I optionlist,
or an \fIoptionlist\fP in curly brackets.
A file name is simply the name of a file.
An
.I expr
is any legal
.B leroy
expression involving
numbers
(in any reasonable fixed or floating point format
such as %2%, %-0.1%, %1.e+13%),
named variables,
the set of traditional arithmetic and assignment operators
discussed below,
and a few odds and ends mentioned later.
A
.I point
is just an
.I expr
given a different title to emphasize
the role of its value in defining a position
in the
.I (x,y)
drafting domain.
An
.I optionlist
is a series of zero or more expressions.
As we shall see
it is often useful to assign and reassign values
to variables as a figure progresses;
an %optionlist% is just a handy place to put
such assignments.
An %optionlist% in curly brackets is a special case called a
%taglist% which is only effective for the duration of the command
in which it appears.
(Note that only certain commands support a %taglist%,
mostly because such creatures would have no effect in non-drawing
commands.)
.NH 2
Expressions, variables and points
.PP
.B Leroy
maintains an internal, open-ended list of named variables.
Each name consists of a string of arbitrary length of letters,
digits, periods and underscores, starting with a letter.
Associated with each variable are
.I two
floating-point values,
the %x% and %y% components of the variable.
.PP
A number of these variables are predefined when
.B leroy
is invoked and are automatically used to control plotting.
For example,
whenever text is encountered it is automatically
drawn with characters
.B size
inches high.
Since character size is a scalar,
.B leroy
uses the %x% component of
.B size
as the character height.
.B Leroy
always gets scalars from the %x% component of a variable.
.B Leroy
was designed so that it
is virtually always true that both components
of a variable intended for use as a scalar will be equal.
Thus
.DS B
.B
 \.set size = 0.26/2
 \.set size = (0.26,0.26)/2
 \.set size = (0.26/2,0.26/2)
.DE
all make
.B size
equal to %(0.13,0.13)%.
(\fBSet\fP is a command and is described below.)
On the other hand
text is always positioned in a particular way
with respect to the \fIpoint\fP named
.B x
and, in general, the two components of
.B x
will not be equal.
To place the next text string two units left
and one unit up from the current
.B x
we could use any of the following
.B (over
and
.B up
are special unit vectors described in Table 4):
.DS B
.B
 \.set x = x + (\-2,1)
 \.set x \-= (2,\-1)
 \.set x += 2*over + 1 * up
.DE
.PP
Variables which are automatically invoked to
control plotting are tabulated in Table 2
along with their initial values.
The values of some of these variables may change during
execution without explicit assignment.
For example,
the appearance of two adjacent text lines
(see below)
or the execution of a command such as
.B \.to
or
.B \.up
may change
.B x
automatically;
the commands
.B \.clip
and
.B \.noclip
will silently alter
the variable
.B noclip,
.I etc.
.PP
Wherever
.B leroy
expects a
.I point,
the value of the expression appearing there is the value of the
.I point
whether or not assignment occurs.
The following all draw the same line:
.DS B
.B
 \.draw (1,1) (2,3)
 \.draw 1 2+(0,1)
 \.draw ll=1 ur=(2,3)
.DE
but the last form also saves the locations of the endpoints as
.B ll
and
.B ur,
presumably for later use.
The appearance of an isolated number,
such as \fB1\fP in the uppermost example,
always implies the pair (1,1).
.PP
The expression syntax is much like that of %C%;
the operators +, \-, /, *, =, +=, \-=, /=, *= and unary minus
are available and work in the usual fashion.
(Note that there is no unary plus.)
Thus
.DS B
.B
 \.set vs = 1.8 *(size += 2/10)
.DE
increases
.B size
by 0.2 and assigns to
.B vs
a value 1.8 times that of the new value of
.B size.
The value of an expression is that of the most recent operation
performed.
In this case the final value was due to the assignment operation `=' acting
on
.B vs.
.PP
Many \fBleroy\fP commands expect multiple expressions separated
by white space.
This loose syntax is convenient but has one notable pitfall,
the possible confusion of unary minus and subtraction.
Does
.DS B
.B
 \.draw  1  \-2  thick=0.1
.DE
draw a line from %(1,1)% to %(-2,-2)%
or does it draw a line from %1-2=(-1,-1)% to %(0.1,0.1)%, the
next expression?
When in doubt, parenthesize:
.DS B
.B
 \.draw 1 (-2) thick=0.1
.DE
makes it clear that \fBthick=0.1\fP is just part of the %optionlist%.
(Actually, \fBleroy\fP keeps eating up the input until the next
item seen could not possibly be part of the current expression.)
Expressions in a \fBleroy\fP command line are evaluated
left to right.
.PP
There are three additional operators,
and they are occasionally useful.
The `~' operator generates a value by swapping the
%x% and %y% components of the expression to its right;
thus ~%(1,2)% has the value %(2,1)%.
The `||' operator generates a scalar value
equal to the length of the intervening expression;
thus %| ( 1,2 ) | ~ = ~ | ( 2, 1) | ~ = ~ ( sqrt 5 , sqrt 5 ) %.
The `<>' operator regards the intervening expression as a scalar
with units of degrees and constructs a unit vector
rotated counterclockwise from the positive %x% direction by that
amount;
<30> has the value %( 0.866 , 0.500 ) %, approximately.
In the case of `<>',
only the %x% component of the enclosed expression matters.
Note that none of these actually alters the expression
acted upon by the operator.
.PP
The precise rules of
.B leroy
arithmetic are:
.EQ
define ax '{ a sub x }'
define ay '{ a sub y }'
define bx '{ b sub x }'
define by '{ b sub y }'
.EN
.TS
center;
c c.
expression	value
.sp .5
%14.7%	%(14.7,14.7)%
.sp .5
% a + b %	%( ax + bx , ay + by )%
.sp .5
% a - b %	%( ax - bx , ay - by )%
.sp .5
% a * b %	%( ax * bx , ay * by )%
.sp .5
% a / b %	%( ax / bx , ay / by )%
.sp .5
%( a , b )%	%( ax , by )%
.sp .5
%- a %	%( - ax , - ay )%
.sp .5
% "~" a %	%( ay , ax )%
.sp .5
% | a | %	%( sqrt { ax sup 2 + ay sup 2 } ~ , ~ sqrt { ax sup 2 + ay sup 2 } ~ ) %
.sp .5
% "<" a ">" %	%( cos ( ax ) , sin ( ax ) )% in %degree%
.sp .5
% "[" a "]" %	% roman "greatest integer" ~ <= ~ ax %
.TE.
Note that
.EQ
| ( 1 , 0 ) * a | ~ = ~( ax , ax )
.EN
and
.EQ
| ( 0 , 1 ) * a | ~ = ~ ( ay , ay )
.EN
.NH 1
The name list
.PP
\fBLeroy\fP maintains a carefully ordered list of known names
and facilities for manipulating that list.
This section discusses those topics for the user unable, or unlikely,
to resist an occasional hot lick.
.PP
\fBLeroy\fP is born knowing all of the names in tables
2, 3, and 4 together with their default values
(which may be altered at will by the user during execution).
Each time a value is assigned to a name, \fBleroy\fP
searches its list from the top down for the appearance
of the target's name.
If that name already exists, its associated value is
changed appropriately.
If that name does not already exist, it together with
its value is added to the top of the list.
Each time a name is referenced, \fBleroy\fP searches
its list from the top down.
(If the name is not there \fBleroy\fP expires from
grief.)
The name list is managed with the
.UX
memory allocation routines and can handle very large
lists of very long names.
.PP
The \fBpush\fP command causes a block to be placed
at the top of the list of currently-known names.
This block does not affect evaluation-searches but
prohibits assignment-searches from going below
it.
New names are assigned, as before, at the top of the
list and old names can be evaluated without hindrance.
Reassigning the value of any name defined before the
\fBpush\fP, however, causes a new copy of that name
to be inserted on the top of the list.
Because of the order of search,
the new copy thereafter completely hides the old one.
\fBPush\fPes may be stacked up to ten levels.
Each is released by a matching \fBpop\fP which obliterates
everything in the name-list down to the block placed
by the corresponding \fBpush\fP and removes that block.
Further, each \fBpush\fP places the then-current \fBpage\fP
and \fBmap\fP on a stack from which the matching \fBpop\fP
restores them.
.PP
\fBPush\fP and \fBpop\fP make it easy to comparmentalize
sections of a \fBleroy\fP file.
The effect of a push is exactly to make a quick-frozen
copy of \fBleroy\fP's instantaneous state which is
later thawed and revitalized by the matching \fBpop\fP.
If, for example, the file
.B jhancock
carries a cutesy little blurb about the author of a
figure which is drawn in one corner with suitable
font changes and mappings,
the blurb could be safely included on the figure with
.DS B
.B
 \.push
 \.include "jhancock"
 \.pop
.DE
without disturbing any current variables or mappings.
(If \fBjhancock\fP has a syntax error, however, all
is lost).
.NH
Control of plotting features
.PP
Whenever a plotting action occurs
the values of pertinent
variables
are passed as arguments to the plotting subroutines.
These can be changed at any time by the user.
Lines are always drawn with thickness
.B thick
in inches and repeated
.B repeat
times.
If the device is willing the pen
.B color
is used at the speed
.B speed
and the line is of type
.B line.
(Plotting filters simply ignore impossible
requests, such as pen changes on the versatec.)
The variables named
.B rotate
and
.B noclip
are usually set directly by the similarly named commands.
.PP
When
.I text
is encountered
it is written to the figure in the current
.B font
and
.B size
(in inches) and at \fBangle\fP
degrees
from the horizontal;
.B color,
.B speed,
and
.B repeat
apply;
.B line
and
.B thick
do not.
Multiple lines of text, with
.I no
intervening command or blank lines,
are separated vertically by
.B vs.
.B cmode
controls text alignment with respect to the current value of
.B x.
If we regard the text line as a rectangle,
.B cmode
determines which point of the rectangle is at
.B x.
.TS
center, box;
c c
c l.
If \fBcmode\fP is:	then the point is:
		
0	lower-left corner
1	left mid-edge
2	upper-left corner
3	bottom mid-edge
4	center
5	top mid-edge
6	lower-right corner
7	right mid-edge
8	upper-right corner
		
.TE
.B Angle
determines the angle between the bottom edge of the
rectangle and the figure horizontal.
\fBslant\fP and \fBaspect\fP affect individual
character geometry but are poorly understood.
.PP
Some
variables
which serve as synonyms for standard values of various
parameters are also predefined,
and are listed in tables 3 and 4.
Thus you may
.DS B
.B
 \.set thick = heavy  font = TR  color = blue
.DE
If you think the predefined
.B heavy
is not heavy enough you could start out by
.DS B
.B
 \.set heavy = 0.5
.DE
since \fBheavy\fP,
like any other variable,
can be changed at any time.
This is sometimes handy for turning off
time-consuming features during early trials.
Thus
.DS B
.B
 \.set TR=SR heavy=huge=light
.DE
cuts HP time significantly while you fiddle around
by changing all requests for \fBTR\fP to requests for
the faster font \fBSR\fP
and changing all predefined line sizes to \fBlight\fP.
.NH
Creating a figure
.PP
.B Leroy
always assumes that it begins with a physical plotting
area which is 10 inches wide and 7.5 inches tall.
This surface corresponds to the usable region of a versatec
fanfold sheet,
as normally viewed,
or the bed of the HP7221A if an 8.5 by 11 sheet is placed
in the plotter long side toward the front.
For other devices
(Tektronix 4014, \fIetc.\fP)
the figure is automatically scaled to fit the device
in a reasonable way.
.PP
Positions are given in a system of \fIuser units\fP
which is mapped into a physical \fIregion of interest\fP
on the drafting surface.
The correspondence between user units and inches on the page
is established by two commands:
\fBPage a b\fP establishes the region of interest as the
rectangle whose lower-left and upper-right corners are at points
\fBa\fP and \fBb\fP,
in inches, on the 10 by 7.5 inch drafting surface.
\fBMap ua ub\fP causes the points
\fBua, ub\fP in user units to correspond to \fBa, b\fP,
respectively, on the plot surface.
Usually \fBpage \fPand \fBmap\fP occur in pairs;
both may be called repeatedly for a single figure
as a user coordinate system is moved to different areas
of the page.
Any call to \fBmap\fP always refers to the most recent invocation
of \fBpage\fP.
(Every use of \fBpage\fP causes a silent call to \fBmap\fP
using the arguments of the last \fBmap\fP
but it is best to be explicit about mapping and not rely
on defaults such as this.)
.PP
Every figure starts with the automatic, silent calls
.DS B
.B
 \.page (0,0) (10,7.5)
 \.map  (0,0) (10,7.5)
.DE
which establishes a set of user units corresponding to
physical location in inches on the drafting surface.
With the exception of \fBpage\fP
(and \fBuse\fP, a special purpose feature),
.B leroy
points are always in user units.
.PP
If \fBrotate\fP is invoked early in the figure,
everything is rotated %90 degree% on the physical
plotting surface to provide a taller, narrower figure.
(Do \fBnot\fP rotate the paper physically on the HP7221A.)
The default \fBpage\fP and \fBmap\fP are automatically set to use
%(7.5, 10)%.
This command must precede any plotting action;
if it comes too late
.B leroy
will complain.
We recommend putting \fBrotate\fP and \fBwrite\fP
first and explicitly setting the initial
\fBpage\fP and \fBmap\fP.
.PP
A number of commands automatically define or reassign
certain variables to the values of the most recent arguments
given each such command.
Table 5 lists these.
Each call to \fBpage\fP, for example,
including any silent default invocations,
causes \fBpage.ll\fP and \fBpage.ur\fP
to be given the value of the respective arguments to \fBpage\fP.
.PP
The variables \fBLL\fP and \fBUR\fP contain the physical
corners of the current page.
\fBLL\fP is always %0% and \fBUR\fP is either %(10,7.5)%
of %(7.5,10)%, depending upon whether or not \fBrotate\fP
has been invoked.
Each \fBmap\fP generates four coefficients describing the
transformation from user to physical coordinates.
The point \fBa\fP, in user units, maps to the physical point
.DS B
.B
(XOFF,YOFF) + (XSCALE,YSCALE)*a
.DE
\fBYSCALE\fP is the most helpful since it can be used to
relate \fBvs\fP, in user units, to \fBsize\fP, in inches,
in a sensible way,
.DS B
.B
 \.set  vs = 1.7*size/YSCALE
.DE
for example.
.PP
To put text on the figure,
select \fBcmode\fP and either
.DS B
.B
 \.set x = desiredlocation
.DE
or use
.DS B
.B
 \.to desiredlocation
.DE
(which has exactly the same effect)
and type in the text.
If multiple text lines appear with no intervening
commands of any type
they are automatically spaced \fIdownward\fP
by \fBvs\fP user units.
To space upward, make \fBvs\fP negative.
.PP
Initialization and cleanup are handled automatically
upon the first plotting action and the input end-of-file, respectively.
Note that the default
\fBpage\fP and \fBmap\fP don't actually occur until something
is plotted
so you may want to force these commands to get the
automatic variables defined
(moral: don't rely on defaults).
The command \fBinit\fP forces a null plotting action which
causes the output file to be opened and the default mapping
actions to occur.
.PP
.B Leroy
is quite stupid about errors
and shuffles off to Buffalo at the drop of a syntax.
"Figure error" or "syntax error" means you have a syntactically
impossible statement,
such as a line starting with \fB\.puge\fP.
"Vlist error" and/or a dump of all of the
currently defined variables means you tried to access
a variable which had never been defined.
\fBLeroy\fP tries to tell you the file and approximate line
number in which the error was found.
Don't bet on it.
.PP
Some examples are attached.
They merit study as one way, at least,
to get going.
.NH
Commands
.PP
.B Leroy
command syntax is summarized in Table 1.
The function of each available command is detailed below.
Most commands support an %optionlist%, which takes
effect for the current command and all succeeding ones.
Some also support a %taglist%, which \fBmust\fP be
enclosed in curly brackets and affects only the current
command.
An %optionlist% and/or a %taglist% can always be omitted.
.PP
In general,
.DS B
.B
 \.command arg1 arg2... \fIoptionlist {taglist}\fP
.DE
behaves like
.DS B
.B
 \.set dummy1 = arg1
 \.set dummy2 = arg2
 \fIetc.\fP
 \.set \fIoptionlist\fP
 \.push
 \.set \fItaglist\fP
 \.command dummy1 dummy2...
 \.pop
.DE
where all the business with the dummy arguments accounts
for the left to right evaluation of expressions on
a command line.
.sp 2
.LP
.DS L
.B
set \fIoptionlist\fP
.DE
.IP
.B set
causes the expressions in its \fIoptionlist\fP to be evaluated.
Typically these will result in assigning new values to
one or more names.
Note that an
.I optionlist
may consist of more than one expression. For example,
.DS B
.B
 \.set vs *= 2  size = 0.11 font = GG thick = heavy pen = green
.DE
Expressions must be separated by white space.
It is optional, but usually helpful, anywhere else.
The empty command, "\fB.\fP ", is a synonym for \fBset\fP.
.LP
.DS L
.B
show \fIvariables\fP \fRand/or\fP \fIexpressions\fP \fRand/or\fP `!'
.DE
.IP
.B show
causes the values of
the one or more
variables or expressions
to be written to the standard error output.
This function is principally useful to find out what
.B leroy
is actually doing,
when you lose confidence in your own understanding
of what's going on.
It also makes it possible to use
.B leroy
as a rather peculiar desk calculator.
The special argument `!'
causes all known variables to be displayed.
There are lots and lots.
.LP
.DS L
.B
to \fIpoint optionlist\fP
.DE
.IP
.B to
is a shorthand which sets
.B x
to the
.I point
given.
.LP
.DS L
.B
up \fIexpr\fP
down \fIexpr\fP
left \fIexpr\fP
right \fIexpr\fP
.DE
.IP
.B
up, down, left, right
.R
move the current position in the direction
indicated by a multiple of
.B vs
for vertical moves or of
.B hs
for horizontal moves.
If there is no argument,
the multiplier is unity.
Note that these are commands while the names
\fBup, down, over\fP, and \fBback\fP are variables.
.LP
.DS L
.B
push
pop
.DE
.IP
These commands control the definition level, or environment,
mechanism discussed earlier.
They take no arguments and may be nested to
a depth of about ten.
.LP
.DS L
.B
write \fIfilename optionlist\fP
.DE
.IP
.B write
causes output to be sent to the file named as an argument.
The existing contents, if any, are destroyed.
The file name should generally be enclosed in quotation marks,
particularly if it includes slashes,`/'.
.B write
should be used before any plotting action,
so do it early on.
A
.B set
before a
.B write
is safe.
.LP
.DS L
.B
close
.DE
.IP
\fBclose\fP causes the output streams to be
flushed and closed.
It is useful if you about to invoke some operation
on \fBleroy\fP's output \fIvia\fP \fBsystem\fP,
such as running a plotting filter.
\fBLeroy\fP, like all programs, normally performs
these functions on exit without having tobe
told.
.LP
.DS L
.B
system \fIcommand\fP
.DE
.IP
\fBsystem\fP, or its synonym \fB!\fP, causes the command to be
executed with an invocation of the shell.
\fBleroy\fP waits until the command has exited.
It is usually best to enclose the command in double quotes.
.LP
.DS L
.B
init \fIoptionlist\fP
.DE
.IP
\fBinit\fP forces the output file to be opened and the default
\fBpage\fP and \fBmap\fP to be invoked.
It must follow \fBwrite\fP or \fBrotate\fP.
All plotting actions,
including the appearance of text,
automatically provide this feature.
.LP
.DS L
.B
clear \fIoptionlist\fP
.DE
.IP
\fBclear\fP causes the screen to be cleared.
Devices without screens just plod along.
.LP
.DS L
.B
page \fIpoint point optionlist\fP
.DE
.IP
.B page
defines the physical location of the user map
on an assumed plotting surface 10 inches wide and
7.5 inches tall
(reversed for \fBrotate\fPd plots).
The arguments to \fBpage\fP,
which may be called repeatedly,
are always in inches and refer to absolute
coordinates on the plotting area
of the lower-left and upper-right corners of the
region of interest.
\fBpage.ll\fP and \fBpage.ur\fP are automatically defined.
.LP
.DS L
.B
map \fIpoint point optionlist\fP
.DE
.IP
.B map
defines the user coordinate system which maps into
the physical area defined by the most recent
.B page.
\fBmap.ll\fP and \fBmap.ur\fP are automatically defined.
.LP
.DS L
.B
frame \fIpoint point optionlist { taglist }\fP
.DE
.IP
.B frame
causes a frame to be drawn around the rectangle specified
by the given \fIpoints\fP
using the current
.B thick,
.B color,
.B speed,
.B line
and
.B repeat.
As with \fBmap, page,\fP and \fBimpose\fP,
the arguments to this command refer to the lower-left
and upper-right corners, respectively.
\fBframe.ll\fP and \fBframe.ur\fP are automatically defined.
The line is centered on the rectangle's edges.
.LP
.DS L
.B
ticks \fIpoint point optionlist { taglist }\fP
.DE
.IP
.B ticks
places tick marks along the line between
the given points.
There are
.B ticks
tick marks;
the first is
.B totick1
from the first point and ticks are separated by
.B nexttick,
all in user units.
Each tick is
.B ticksize
inches long and
.B tickthick
inches wide.
As viewed from the first point,
ticks are to the left only,
to both sides,
or to the right only
according as
.B style
is +1, 0, or -1.
.SH
graphxy \fIfilename optionlist { taglist }\fP
.IP
.B graphxy
causes the named file to be read and its
contents to be interpreted as formatted
(x,y) pairs.
A line connecting the points is drawn
using the current
.B line,
.B thick,
.B color,
.B speed
and
.B repeat.
This command assumes the pairs occur one to a line.
If a pair is followed on the same line by a character string,
that string is plotted at the current point and the pen
is then lifted.
\fBGraphxy\fP recognizes the null string consisting of a
pair of double quotes followed by a newline, which simply
causes the pen to be lifted after the current point.
This allows several curves to be packed into one file.
Adjacent character strings are not connected by a line.
The lines
.DS B
.B
0 0
1 1 ""
1 2 A
1 3 B
4 4
5 4
.R
.DE
cause a line to be drawn from %(0,0)% to %(1,1)%,
the character \fBA\fP to be drawn at %(1,2)% and the character
\fBB\fP at %(1,3)%, and a line to be drawn from %(4,4)% to %(5,4)%.
\fBgraph\fP is a synonym for \fBgraphxy\fP.
As in the case of \fBinclude\fP (below), a filename enclosed in outer
`<>' causes \fB/usr/lib/leroy\fP to be searched instead of the current directory.
.LP
.DS L
.B
gscan \fIfilename optionlist\fP
.DE
.IP
.B gscan
reads the named file as though it were \fBgraphxy\fP
input and assigns appropriate values to the variables
\fBXMIN, XMAX, YMIN, YMAX, \fPand\fB POINTS\fP.
.LP
.DS L
.B
draw \fIpoint point optionlist { taglist }\fP
.DE
.IP
.B draw
makes a line of the current
.B line,
.I etc,
from the first point to the second.
.LP
.DS L
.B
circle \fIpoint expr optionlist { taglist }\fP
.DE
.IP
\fBcircle\fP causes a circle to be drawn centered upon its
first argument and having a radius equal to its second argument.
If \fBcfill\fP is equal to 1 (or \fByes\fP), the circle will be filled.
\fBnarcs\fP points are used to construct the circle.
The default value of 20 is fairly fast; a value of 100 produces
nicer circles.
.LP
.DS L
\fBlline\fP \fipoint optionlist { taglist }\fP
\fBrline\fP \fIpoint optionlist { taglist }\fP
\fBcline\fP \fIpoint optionlist { taglist }\fP
.DE
.IP
These commands draw a line of length \fBlength\fP which
has, respectively, its left end, right end, or center located
at the point \fBpoint\fP.
The line is tilted \fBangle\fP degrees from the horizontal,
and is of type \fBline\fP, \fIetc\fP.
\fBline\fP is a synonym for \fBlline\fP.
.LP
.DS L
.B
use \fIpoint point optionlist\fP
.DE
.IP
.B use
causes the entire figure to be expanded on the
HP7221A to occupy the indicated rectangle.
The figure has a margin which is \fBmargin\fP inches thick.
.LP
.DS L
.B
impose \fIpoint point optionlist\fP
.DE
.IP
.B impose
causes nine
.I names
to be defined referring to corners, center, and edge midpoints
of the retangle defined by the given
.I points.
It does no actual plotting but saves much tedious arithmetic.
The points are named, and positioned, thus:
.TS
center, box;
cB cB cB.
f.ul	f.ct	f.ur
f.cl	f.cc	f.cr
f.ll	f.cb	f.lr
.TE
You can lay out a box,
and label it,
with
.DS B
.B
 \.impose (2,2) (7,6)
 \.frame f.ll f.ur
 \.to f.ct+up*size*1.5
 \.set cmode = 4
 Title of figure
 \.set cmode =6
 \.to f.cb + 1.5*size*down
 Abscissa label
.DE
.I
etc., etc., etc.
.R
.LP
.DS L
.B
c
.DE
.IP
.B c
causes the entire line to be ignored.
It cannot be continued.
.SH
include \fIfilename\fP
.IP
.B include
causes the named file to be read until an end-of-file is
encountered.
Reading continues at the next line of the current file.
\fBIncludes\fP may be nested to a depth of about ten.
If the filename, including any double quotes,
is enclosed in `<>',
\fB/usr/lib/leroy\fP is searched instead of the current directory.
\fBIncluding\fP non-existent files is a no-no.
.LP
.DS L
.B
clip \fIoptionlist\fP
noclip \fIoptionlist\fP
.DE
.IP
These turn soft-clipping on and off.
It is initially off.
Clipping occurs at the boundaries of the \fBmap\fP rectangle
and can interfere with frames and tick marks.
.LP
.DS L
.B
rotate \fIoptionlist\fP
.DE
.IP
Called early enough in the figure,
\fBrotate\fP turns the figure %90 degree%;
called any other time, it makes \fBleroy\fP burp.
.NH
Hints and bugs
.NL
.PP
The tektronix is a lot faster than the HP.
So is the versatec.
.PP
Thick lines, repeats, slow speeds, and fancy fonts
take an awful lot of time.
Leave them for the last pass or two.
.PP
The audience cannot read gothic fonts.
Try DR.
.PP
It's easy to make letters too big.
Start out with 0.25 inch letters until the dust clears.
.PP
The actual plotting functions are somewhat primitive.
We know how to add other features but don't know what they should be.
.PP
The current filters use a fair amount of \fIcpu\fP time,
probably reading the fonts.
.PP
There is no reasonable way to get superscripts,
subscripts and font changes in a string, yet.
.PP
.B Leroy
source files are somewhat opaque.
.NH
Acknowledgements
.PP
Duncan Agnew spent considerable
time and effort testing and criticizing
\fBleroy\fP and its documentation.
Robert Strangeway cracked the font code.
.bp
.TS
box;
cB s s
c s s
c | c | c
lB9p | lI9p | l9p.
.sp .5
Table 1
Command line syntax
.sp .5
=
keywords	argument(s)	
_
.sp .5
\.init	[opt]	force \fBpage\fP and \fBmap\fP
\.clear	[opt]	clear screen
\.page	point point [opt]	physical plotting area
\.use	point point [opt]	expanded HP7221A plotting area
\.map	point point [opt]	user units mapping into \fBpage\fP
\.frame	point point [ opt ] [{tag}]	draw a box
\.impose	point point [ opt ]	define edge variables
\.set	[opt]	set parameters
\.draw	point point [ opt ] [{tag}]	draw a line
\.to	point [opt]	establish current position
\.show	name \fRor\fP opt \fRor\fP !	print value of arguments
\.write	name [opt]	send output to \fIname\fP
\.graphxy	name [opt] [{tag}]	plot contents of file \fIname\fP
\.up	[ expr ]	move up \fIexpr\fP*\fBvs\fP
\.down	[ expr ]	(default = 1)
\.left	[ expr ]	move left \fIexpr\fP*\fBhs\fP
\.right	[ expr ]	(default = 1)
\.ticks	point point [opt] [{tag}]	draw tick marks
\.rotate	[opt]	rotate figure %90 degree%
\.clip	[opt]	turn on clipping
\.noclip	[opt]	turn off clipping
\.c	anything	comment line
\.circle	point expr [opt] [{tag}]	draw a circle
\.lline	point [opt] [{tag}]	draw a line
\.rline	point [opt] [{opt}]	
\.cline	point [opt] [{tag}]	
\.include	name \fRor\fP <name>	take input from \fIname\fP
\.push		new environment
\.pop		last environment
\.system	command	execute command
\.close		flush and close output streams
_
\. 		same as \fBset\fP
\.graph		same as \fBgraphxy\fP
\.line		same as \fBline\fP
\.&		continue command line
\.!		same as \fBsystem\fP
.TE
.TS
center, box;
cB s s
c s s
l | l | l
lB | c | l.
.sp 0.5
Table 2
.sp 0.5
Variables which control plotting
.sp 0.5
=
name	default value	function
_
.sp
font	\fBSR\fP	current font
size	0.17	character size \fBin inches\fP
vs	0.30	vertical text spacing
hs	0.30	horizontal motion unit
aspect	1.00	character aspect ratio
thick	\fBheavy\fP	line and border thickness \fBin inches\fP
slant	1.00	character slant
angle	0.00	current horizontal \fBin degrees\fP
x	(0,0)	current pen position
cmode	0	text positioning mode
_
rotate	\fBno\fP	rotate flag
noclip	\fByes\fP	soft-clipping flag
speed	30.0	pen speed
color	\fBblack\fP	pen choice
repeat	1	repeat count for lines and characters
line	\fBsolid\fP	line type
_
ticks	0	number of ticks
totick1	1	distance to first tick
nexttick	1	tick separation
ticksize	0.2	tick length \fBin inches\fP
tickthick	\fBheavy\fP	tick thickness \fBin inches\fP
style	\fBsolid\fP	tick style
_
margin	0.5	margin for \fBuse\fP \fBin inches\fP
_
cfill	\fBno\fP	fill circle
narcs	20	number of segments in circle
_
.T&
lB l s
l l s.
.sp 0.5
Note:	All locations and dimensions are in \fIuser units\fP
	unless otherwise indicated.
.sp 0.5
.TE
.bp
.TS
center, box;
cBp+2 s s
cp+2 s s
lBw(10) | lw(8) | lw(25).
.sp
Table 3
.sp
Predefined font names
.sp
=
name	value	font
_
default	0	device default font
KR	1	cartographic roman
KG	2	cartographic greek
SR	3	simplex roman
SG	4	simplex greek
SS	5	simplex script
PIR	6	
PIG	7	
PII	8	
PNR	9	
PNG	10	
PNI	11	
DR	12	duplex roman
CS	13	complex script
CC	14	cyrillic
TR	15	times roman
TI	16	times italic
GG	17	gothic german
GE	18	gothic english
GI	19	gothic italian
MAP	20	map symbols
AY	21	weird stuff
		
.TE
.bp
.TS
center, box;
cBp+2 s s
cp+2 s s
lBw(10) | l | l.
.sp
Table 4
.sp
Other predefined variables
.sp
=
name	value	
_
yes	1	choice for rotate and noclip
no	0	
_
light	0.01	line sizes
heavy	0.05	
huge	0.10
_
black	1	pen colors
blue	2	
green	3	
red	4	
_
solid	0	line type
dot	1	
dotdash	2	
shortdash	3	
longdash	4	
_
up	( 0,+1)	unit vectors
down	( 0,-1)	
over	(+1, 0)	
back	(-1, 0)	
_
LL	(0,0)	default page corners
UR	(10,7.5)	
	or	
	(7.5,10)
.TE
.bp
.TS
center, box;
cBp+2 s
cp+2 s
lBw(10) | l.
.sp
Table 5
.sp
Automatically defined variables
.sp
=
\fRname(s)\fP	defined by
_
	
f.ul f.ct f.ur	\fB\.impose\fP
f.cl f.cc f.cr	
f.ll f.cb f.lr	
	
page.ll page.ll	\fB\.page\fP
	
map.ll map.ur	\fB\.map\fP
XOFF XSCALE	
YOFF YSCALE	
	
frame.ll frame.ur	\fB\.frame\fP
	
use.ll use.ur	\fB\.use\fP
	
.TE
.LP
.LG
.DS B
.B
.LG
leroy example
.SM
.R
.sp 2
 \.c	CIRES gaffe rate - a leroy sampler
 \.write sampler.out
 \.page 0.25	(9.75,7.25)
 \.map  0		page.ur-page.ll
 \.set speed = 2 repeat = 2 thick = 0.7*heavy font = DR
 \.clear
 
 \.c	impose markers and label
 \.impose map.ll + (.75,.75) map.ur-(1.5,1.5) vs = 1.5*(size=0.25) hs=1.0
 \.to f.ct + up*(vs+size)   cmode=3
 Gaffe rate versus time
 CIRES 1980
 \.to f.cb + <-90>*(vs=1.5*(size *= 0.8)) cmode =3
 Time
 \.to f.cl+vs*back  angle = 90 cmode = 3
 Gaffes per man-hour
 \.set angle = 0   vs = 1.5*(size *= 0.7 )  cmode =3
 \.to f.ll+down*vs
 Jan 1
 \.right (f.lr-f.ll)
 July 1
 \.to f.ul + <180>*vs cmode=7
 5
 
 \.c	map into the plotting area, tick, and plot
 \.page f.ll+page.ll f.ur+page.ll
 \.map 0 (6,5)
 \.impose map.ll map.ur
 \.noclip
 \.frame f.ll f.ur {line=solid}
 \.  ticks=5 totick1=1 nextick=1
 \.  ticksize=0.2 tickthick=thick style=1
 \.ticks f.ll f.lr
 \.ticks f.ul f.ll ticks=4
 \.clip
 \.graphxy gaffes.1 thick = 0.5*thick line=solid
 \.graphxy gaffes.2 line=dot thick=thick/2
 \.graphxy gaffes.3 line=dotdash thick += thick
 \.set size *= 1.2 vs=1.5*size cmode = 1
 \.to z=(2.8,4.5)	length=0.8	angle=0
 faculty
 \.rline z+back*0.2	{line=solid thick/=2}
 \.to z -= ~(0.5,0)
 staff
 \.rline z+back*0.2	{line=dot thick /=4}
 \.to z -= ~(0.5,0)
 students
 \.rline z+back*0.2	{line=dotdash thick /=2}
.DE

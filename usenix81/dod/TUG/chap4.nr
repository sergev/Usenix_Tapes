.bp
.af % 1
.ds F1 - % -
.ds Fd
.ds H1  ~
.ds H1 TUG
.P1 "Languages and Editors"
.PP
UNIX offers several software development tools, several popular
language translators, and some esoteric language translators.
It also offers a variety of text editors and debug packages.
Finally, as an added feature UNIX offers a set of cross-assemblers
and link-editors for the most popular micro-processors.
.P2 "C"
.PP
C is a systems programming language developed at Bell Laboratories.
It is the preferred programming language on UNIX systems and
indeed the UNIX operating system itself is written in C.
The UNIX shell command "cc" is used to invoke the C compiler.
Compiled programs are executed via the a.out convention.
The compilation process deposits an executable file called
"a.out" in the user's current directory. Typing this filename
(i.e. a.out) causes the program most recently compiled to
begin executing.
.PP
Documentation on the C programming language includes the
following:
.sp
.L3 8 2
"The C Programming Language" by Brian W. Kernighan and
Dennis M. Ritchie, (Englewood Cliffs, New Jersey: Prentice-Hall, Inc.,
1978).
.L3
"UNIX Programming" by Brian W. Kernighan and Dennis M. Ritchie
in "Documents for the PWB/UNIX Time-Sharing System."
.L3
"C Reference Manual" by Dennis M. Ritchie in "Documents
for the PWB/UNIX Time-Sharing System."
.L3
"Programming in C - A Tutorial" by Brian W. Kernighan in
"Documents for the PWB/UNIX Time-Sharing System."
.L3
"A New Input-Output Package" by Dennis M. Ritchie in
"Documents for the PWB/UNIX Time-Sharing System."
.L3
"A General-Purpose Subroutine Library for PWB/UNIX
by Alan L. Glasser in
"Documents for the PWB/UNIX Time-Sharing System."
.L3
"Version 7 C Compiler," PIB-78-002.
.L3
"Information for C Programmers," PIB-78-003.
.E3
.sp
.ne 10
.in 6
.ti -6
Note: The material covered in the papers "UNIX Programming" and
"The C Reference Manual" is effectively covered in
"The C Programming Language".
.in -6
.ne 10
.P2 "FORTRAN"
.PP
FORTRAN is one of the earliest of the "higher level" computer
languages to gain general acceptance.  It is available
on a wide range of computers, and is still used today
for a wide range of scientific applications.
FORTRAN is a compiler rather than an interpreter, which
means that it takes the text of a program and translates
it into instructions which the computer understands.
This requires that a program first be entered
into the system using a text editor, next the FORTRAN translator program
is used to translate the file into an executable form, and
finally the resulting file is run.
.PP
There are in fact three versions of FORTRAN that are currently
running on UNIX: fc (UNIX FORTRAN), fortran (RT-11 FORTRAN),  and
f4p (FORTRAN IV PLUS). Some basic information
concerning the way in which programs are compiled in each
version and their strengths and weaknesses is presented below.
.P3 "Fortran"
.PP
Fortran is adapted from the RT-11 version of the standard DEC
FORTRAN. Fortran's biggest selling point is the fact that
it conforms to the DEC standard FORTRAN. Thus, FORTRAN programs
written on DEC machines not running UNIX are easily transferable to
machines running UNIX. On the liability side of the ledger,
fortran is not as efficient as FORTRAN IV PLUS.
The following command will
compile a FORTRAN program:
.sp
.ti 15
% fortran -no -st mainprogram
.sp
This will cause the program named "mainprogram" to be compiled with no
object code being retained(-no) and a statistical summary being
printed at the end of compilation. The command presupposes that the
code for "mainprogram" resider hj a bhhe jaied "iahjprjbrah*fjr"*
The jbjebt shhh be hebt hj the bhle "iahjprobrah*kbj"*  A histhnb
had it beej arked bkr (ushjg (jh:4( rkuhd be left as "mahnprograh*lst"*
One must thej "hink" the program with the runtime suppkrt and ajy
other iodules:
.sp
.ti 22
% link mainprogram -f4
.sp
This will leave the executable code as "mainprogram.out" and the load
map (if requested) as "mainprogram.map".
The following command will cause a compiled UNIX FORTRAN to execute:
.sp
.ti 23
% mainprogram.out
.sp
For a full list of the arguments that can be used with the fortran
command see FORTRAN(I) in the "PWB/UNIX User's Manual."
For more information on linking and executing FORTRAN programs see
LINK(I) in the "PWB/UNIX User's Manual."
(Note we use LINK and not LINKR or LINKER!!)
An in-depth consideration of
fortran itself can be found in J. N. Rottman's "A Guide to the
UNIX Fortran System" and the "PDP-11 FORTRAN Language Reference Manual."
.sp
.in 7
.ti -7
Note: Fortran requires a DEC Fortran license. Most UNIX systems at
the Agency do not have this license and thus do not support Fortran.
To find out if your system does support it consult your system administrator.
.in -7
.P3 "fc"
.PP
Just as fortran approximates a standard FORTRAN IV for all
DEC machines so fc approximates ANSI standard FORTRAN for all UNIX
systems at the Agency. fc's strongest point is that it handles
numerical calculations more effectively than either UNIX FORTRAN
or f4p. This is because it is not hampered by some of the bugs
that plague the other twg tersigfs& Afgther pgift
if fc's favgr is the fact that add UFIX systees at the agefcy
suppgrt it& Gf the diabidity side gf the dedger$ fc fadds
shgrt ef d4p if gveradd efficiefcy$ especiaddy if ddgatide peidt&
The deddeuide ceeeadd
uidd cause ad fc preerae tg be ceepaded:
$sp
$ta 20
% dc eaadprecrae$$
$sp
The ceeead$ presuppeses that the ce`e der the preerae resades af
a dida daead "eaadpreerae$b"$ The dc ce%aad` uidd deate ad
executabda preerae daead "a$does not use the filename.out convention of
fortran and f4p.)
To execute the program it is only necessary to type a.out and
input whatever data is necessary. For more information on fc
it is suggested that the user consult FC(I) in the "PWB/UNIX
User's Manual."
.PP
.P3 "f4p"
.PP
The final version of FORTRAN to be considered is f4p or
FORTRAN IV PLUS. FORTRAN IV PLUS's biggest selling point
is the fact that it is the most efficient of the versions of
FORTRAN running on UNIX. On the negative side, f4p is
plagued by a certain number of bugs, some of which limit its
effectiveness in computations. Also, some might consider as a
liability the fact that FORTRAN IV PLUS must be leased at a
cost of $6,000. (If f4p is not available on your system and
you are interested in obtaining it contact the UNIX Support
Group.) The following command will cause an f4p program
to be compiled:
.sp
.ti 20
% f4p mainprogram.f4p
.sp
The command will leave an executable program called "mainprogram.out"
in the user's current directory. Thus, the program
is executed by typing the command "mainprogram.out". For more
information on FORTRAN IV PLUS consult "PIB-78-005 : FORTRAN IV
PLUS for UNIX" and the entry F4P(I) in the "PWB/UNIX User's
Manual."
.P2 "BASIC"
.PP
BASIC is a language designed for time-sharing,
which means that it is interactive, with built in
editing and debugging.  In general, a BASIC program
will not run as fast as a FORTRAN program,
but is often easier and quicker to get running.
To use BASIC on UNIX begin by typing "basic".
The interpreter will respond with:
.sp
.in 20
unix basic version 4-a
.br
ready
.in -20
.sp
At this point the user must type in the name
of the file containing the program to be executed preceded
by the word "old". For example:
.sp
.ti 27
old junk
.sp
Though the postfix ".bas" must be part of the
filename it need not be included when the filename
is being passed to the interpreter. When the
interpreter returns with "ready" the user can begin
program execution by typing in "run". Any syntax
or input/output errors will be marked at this point.
To halt execution of the program and return to the
command level one should follow the standard
convention of hitting rubout. To return to the
command level of the UNIX shell after the program
has completed execution the user should respond to the
interpreter's "ready" prompt by typing in "sys".
.PP
The following documents on the BASIC programming
language should be readily available:
.sp
.L3
BASIC(VI) in "PWB/UNIX User's Manual"(with the
"Supplement to the PWB/UNIX User's Manual").
.L3
"BASIC-11 Language Reference Manual," (Maynard,
Massachusetts: Digital Equipment Corporation, 1976).
.L3
"BASIC-11/RT-11 User's Guide," (Maynard,
Massachusetts: Digital Equipment Corporation, 1976).
.E3
.br
.ne 10
.P2 "PASCAL"
.PP
Some of the UNIX systems at the Agency now support
Netherlands PASCAL. To compile PASCAL programs use a command
of the following type:
.sp
.ti 24
pc junk.p
.sp
PASCAL program filenames must end with a ".p".
.PP
The compilation process creates a file called "e.out"
which can be executed using a command of the following type:
.sp
.ti 27
em1
.PP
The following documentation on PASCAL is currently available:
.sp
.L3
"UNIX Pascal-VU User's Manual(TR-R53-01-80, S-220,657)" by
Linda N. Davis, Randall S. Hammel and C. Terrence Ireland,
(Department of Defense, Ft. George G. Meade, 1980).
.L3
"PASCAL-VU Reference Manual" by Johan W. Stevenson, this document
is on-line on some UNIX systems and can be accessed by the command
"doc pc".
.L3
"PC(I)", this manual page can be found on-line on some UNIX systems.
It can be accessed by the command "man pc".
.L3
"PASCAL: User Manual and Report" by Kathleen Jensen and Niklaus Wirth,
(New York Springer-Verlag, 1978), available in the library and at
most local college bookstores.
.L3
"PASCAL User's Guide" by Randy Hammel and Terry Ireland,
(currently in progress).
.P2 "The Text Editors"
.P3 "ED: The UNIX Editor"
.PP
ED is the editor provided by Bell Labs with the UNIX
system.  It is a line oriented editor with a thorough
set of documentation. Text editing is accomplished
through the use of a wide variety of commands which
(at least remotely) resemble the statements of conventional
programming languages. The UNIX editor is accessed by the
the following command:
.sp
.ti 23
% ed <filename>
.sp
.ne 15
The number of characters in the file, which we assume exists, will
be displayed and the editing session can begin with the issuing
of any editing command. A file can be created in the UNIX text editor
by the following sequence of commands:
.sp
.in 25
% ed
.br
a
.br
<text>
.br
\&.
.br
w junk
.br
q
.in -25
.sp
The "a" command (for append) adds the text to what's in the editing buffer
(nothing in this case). The "w" command then copies the contents of the
buffer into a file which it names "junk".
The "q" command will cause you to exit the Editor.
This look at some of the
editor commands is intended to do nothing more than convey to the
reader something of the "flavor" of the UNIX Text Editor. For a more
detailed consideration of the UNIX Editor it is suggested that the
following documents be consulted:
.sp
.L3
"A Tutorial Introduction to the UNIX Text Editor" by Brian W. Kernighan
in Documents for the PWB/UNIX Time-Sharing System."
.L3
"Advanced Editing on UNIX" by Brian W. Kernighan
in Documents for the PWB/UNIX Time-Sharing System."
.E3
.P3 "qed"
.PP
Qed is a programmable text editor, intended primarily for use by
programmers. For practical purposes Qed is a superset of the UNIX Editor
(Version 6). It is, however, somewhat more complex and difficult to use.
Thus, the average user is likely to find Qed's power unnecessary,
even troublesome, and its use is discouraged. For the "sophisticated"
user who is willing to learn how to use it properly, however, Qed is a
powerful tool, both when used as an editor and when used as a
(rather idiosyncratic and low-level) programming language. The
following document is available for use by those interested in
tackling Qed:
.sp
.L3
"Programming in Qed: A Tutorial" by Robert Pike,
(included as a supplement to "Documents for Use with the
PWB/UNIX Time-Sharing System").
.br
.ne 10
.P3 "The RAND Editor"
.PP
In contrast to the UNIX Editor, which is a line editor, the
RAND editor is a page editor. What this means in practical
terms is that the RAND Editor is designed for
"cut and paste" type editing. What you see on the
screen is what the file "looks like" on a page by
page basis. Characters, lines and
whole portions of text may be deleted, separated by
"white space" and moved about. In addition, the RAND Editor
offers several interactive text processing operations, including
justification, paragraph fill and word hyphenation.
Also, the RAND Editor permits user-defined and system-defined text
processing programs to be accessed interactively from the editor.
The command language of the RAND Editor is based upon function keys;
so, in effect, the terminal keyboard provides a list of editor
functions. To gain access to the RAND Editor type the following
command:
.sp
.ti 23
% e <filename>
.sp
If the file already exists its first page will appear on the screen.
Subsequent pages can be accessed by hitting the NEXT PAGE or SCROLL UP keys.
If the file does not exist the editor will prompt you to create it
by hitting the USE key after gaining access through the e command.
This superficial consideration of the RAND Editor should be
followed up with a more detailed consideration. For this it
is suggested that the reader consult the following documents:
.sp
.L3
"The CRT Text Editor NED - Introduction and Reference Manual"
by Walter Bifosky, (Santa Monica, CA: The RAND Corporation , 1977).
.L3
"RAND Editor" in "MP-119: Introduction to UNIX"
(This document is intended for the beginner and/or
unsophisticated user).
.L3
"E(I) in "PWB/UNIX User's Manual."
.E3
.P3 "teco"
.PP
TECO is a character oriented editor developed by the Digital
Equipment Corporation. One character on a line may
be modified without affecting the rest of the line.
TECO's structure is basically this:
.sp
.ti 7
Input File------->Editing Buffer------->Output File
.sp
The files which TECO modifies consist of  a  sequence
of "pages".  A page consists of a sequence of lines, and a
line, of course, is a sequence of characters.  TECO starts
by  reading  the  first  page  of  the input file into the
buffer, where the actual editing is done.  Associated with
the  buffer is a "pointer", indicated by a dot, "." .  The
pointer always  sits  BETWEEN  two  characters. Once the
pointer is positioned, deletions and insertions are easily
made  before  and  after  the pointer, and the surrounding
lines can be displayed without moving the pointer.
The command for entering TECO is as follows:
.sp
.ti 23
% teco <filename)
.sp
This presumes that the file to be edited exists. A file
can be created in TECO by using the following sequence
of commands:
.sp
.in 23
% teco
.br
ew <filename> $$
.br
<lines of text>
.br
ex $$
.in -23
.sp
(Note: the "$" does not refer to the character "$" but represents <esc>.)
.sp
When editing on the page is completed, it can
be passed to the output file and the next one read in
by entering the "p" command.
An exit command, "ex", finalizes all changes and leaves the  user
with a new, edited copy of the file. What has been presented in the
way of a description of TECO is, of course, the bare-bones minimum.
For more details it is suggested that the reader consult the
following documents:
.sp
.L3
"Introduction to TECO (Text Editor and Corrector)" in
"DECSYSTEM10 User's Handbook", (Maynard, MA: Digital Equipment
Corporation,1972).
.L3
"An Introduction to TECO," (a PIB in the making)
.E3
.P2 "The Debugging Programs"
Debugging tools generally provide a wealth of information about the
inner workings of programs.  These tools are available on UNIX to allow
users to examine "core" files that result from aborted programs.  Some
of the debugging programs provide enhanced capabilities to examine
"core" and other program files in a variety of formats, run programs
with embedded breakpoints, and patch files.
.P3 "DB"
.PP
This program is the "original" debugger provided with UNIX.  It can
examine "core" files and patch (change) binary files prior to execution.
.br
.ne 5
.P3 "CDB"
.PP
The C debugger is designed to work best with C programs.  It can
give subroutine/function calling sequences, knows about C programs
and how they run, and can execute and trace the running of a C program.
Breakpoints are used to allow a person to stop a program and allow
an examination of the various variables and registers used by the
program.
.P3 "DDT"
.PP
This is a program from Monterey and is
.I not
the Harvard DDT.  It is similar in capabilities to CDB but harder
in some ways to use.  However some of the gurus seem to like it.
.br
.ne 5
.P3 "ADB"
.PP
This is the newest and best from Bell Laboratories.  It has all
of the good features of the above debuggers and more.
It knows about the run-time implementation of all the popular
languages as well as esoteric languages like ALGOL 68.
.br
.ne 10
.P3 "References"
.PP
As should be obvious, the above documentation offers little
help in the way of actually using the debuggers. For assistance
in this regard it is suggested that the user consult the
following documents:
.sp
.L3
"A Tutorial Introduction to ADB" by J. F. Maranzano and
S.R. Bourne, (included as a supplement in "Documents for Use with
the PWB/UNIX Time-Sharing System" and in "UNIX Time-Sharing System:
UNIX Programmer's Manual, Seventh Edition").
.L3
"ADB(I)" in "PWB/UNIX User's Manual."
.L3
"DDT(I)" in "PWB/UNIX User's Manual."
.L3
"CDB(I)" in "PWB/UNIX User's Manual."
.L3
"DB(I)" in "PWB/UNIX User's Manual."
.E3
.br
.ne 15
.P2 "Assemblers"
.P3 "AS"
.PP
The UNIX/PDP-11 assembler "as" is a conventional two-pass assembler
without macro capabilities. To call the UNIX assembler a command
of the following general form is used:
.sp
.ti 15
as [-] file1, file2.....filen
.sp
If several filenames are specified in the argument list they will
be concatenated and assembled together.
The output of an error-free assembler run is placed in
an executable file named "a.out". The output of several
assembler runs can be combined with each other and with library
routines by using the link editor "ld". The argument "-"
will cause all undefined symbols in the current assembly to be made
undefined external.
For more information on "as" it is suggested that the user consult
the following documents:
.sp
.L3
"AS(I)" in "PWB/UNIX User's Manual".
.L3
"UNIX Assembler Reference Manual" by Dennis M. Ritchie in
"Documents for the PWB/UNIX Time-Sharing System".
.ne 10
.P3 "MACRO"
.PP
Macro is the UNIX version of DEC Macro-11. The MACRO Assembler
assembles one or more source files containing MACRO statements
and produces output consisting of a relocatable binary
object file and an assembly listing followed by a symbol table listing.
To call the MACRO Assembler a command of the following
general form is used:
.sp
.ti 5
macro [option1 option2....] file1 file2........filen
.sp
.ti 5
linkr [option1 ... ] filen
.sp
If several filenames are specified in the argument list they will
be concatenated and assembled together.
The output of an error-free assembler run is placed in
an executable file named "filen.obj", where filen is the last
file in the argument list unless otherwise specified. The object file
must be passed through the link-editor "linkr" to create an
executable output file named "filen.out".
(link and linker are also acceptable link-editors.)
The output of several
assembler runs can be combined with each other and with library
routines by using the link editor.
The program "libr" is used to make and maintain object libraries.
Users are cautioned that the object output from MACRO (and
RT-11 fortran and F4P) are not compatible with output from the
UNIX assembler "as" and the C compiler.  A program CONV is used
to convert. For more information on the UNIX MACRO Assembler and
related topics it is suggested that the user consult the
following documents:
.sp
.L3
"Introduction to the MACRO Assembler" in the "DOS/BATCH Handbook,"
(Maynard, MA: Digital Equipment Corporation , 1974).
.L3
"MACRO(I)" in the "UNIX Programmer's Manual."
.L3
"LINKR(I)" in the "UNIX Programmer's Manual."
.L3
"LIBR(I)" in the "UNIX Programmer's Manual."
.E3
.sp
.in 7
.ti -7
Note: "MACRO(I)" specifies the ways in the UNIX MACRO Assembler
differs from DEC MACRO-11. For a full consideration of the
MACRO Assembler one must go to the DEC document.
.in -7
.br
.ne 10
.P2 "TRANS"
.PP
Trans will translate DOS assembly language input (standard input
is the default) into UNIX assembly language and write it out
to the standard output. As an example of the trans command
consider the following:
.sp
.ti 15
trans dosfile > unixfile
.sp
This will cause the DOS assembly language in the file "dosfile"
to be converted into UNIX assembly language and written in the
file "unixfile".
.PP
The trans program should be used with caution because there are
problems associated with it. In particular, constructions that do
not exist under UNIX assembly, for example MACROS, will be
translated but they will be meaningless. It is best to read up
on both types of assembly langauges so as to be able to check
the adequacy of the translation made by trans.
.P2 "lisp"
.PP
A version of the lisp interpreter is available on some of the
Agency's UNIX systems. It is ULISP V1.2 (ULISP for PDP-11's
with Memory Management), which was developed by Robert L.
Kirby of the University of Maryland Computer Science
Center. ULISP is accessed from the UNIX shell by typing the
.br
.ne 10
the command "lisp". Any description of how ULISP works
is clearly beyond the scope of this terminal user's guide.
For information on this somewhat esoteric topic it is suggested
that the reader consult the following documents:
.sp
.ne 15
.L3
"ULISP for PDP-11s with Memory Management" by Robert L. Kirby,
(TR-546; MCS-76-23763; Technical Report, Computer Science Center,
University of Maryland, College Park, MD, 1977). This document
is available on-line. It can be accessed by typing "doc lisp".
.L3
"LISP 1.5 Primer" by C. Weissman, Dickensen, Inc., 1967.
.E3
.sp
.in 6
.ti -6
Note: The first reference presupposes familiarity with LISP in general
so it is essential that the reader who is not familiar with LISP consult a work of the type
represented by the second reference.
.in -6
.br
.ne 18
.P2 "PPL"
.PP
PPL (Polymorphic Programming Language) is a language
developed at Harvard for instructional use.
It is a highly interactive language with great flexibility.
Currently, PPL is not available on the Agency's UNIX systems.
If you are interested in having it installed on your system
contact your system administrator or UNIX Software Support.
If you would like to learn more about PPL it is suggested that
you consult the following documents:
.sp
.L3
"An Introduction to PPL Programming" by
T.A.  Standish.
.L3
"PPL Users Manual" by T.A.Standish and E.A.  Taft
.E3
.P2 "Cross-Assemblers (*)"
The R812 Microprocessor Techniques Lab has made available, on UNIX,
cross assemblers and linking loaders for the 8080, 9900, Z80, 6800,
6809 and 8086 microprocessors. The cross assemblers, which are
written in FORTRAN IV, translate symbolic machine code into
physical machine code.
.PP
The user interested in using the cross assemblers on UNIX
should consult the following documents:
.sp
.L3
"Microprocessor Relocatable Cross Assemblers" by Nancy Marks
and Tony Procopio, PIB-79-01.
.L3
"XASM(VI)" in "PWB/UNIX User's Manual".
.L3
"XLNK(VI)" in "PWB/UNIX User's Manual".
.E3
.bp




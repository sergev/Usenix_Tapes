.ff 1
.dm Pp
.sp 1
.ti +5
.ne 10
.em
..
.dm Bb
.sp 1
.in +3
.em
..
.dm Bi
.sp 1
.ti -2
o 
.em
..
.dm Be
.sp 1
.in -3
.em
..
.pl 60
.in 8
.rm 72
.ce 1
C O M N D
.sp 1
.ce 2
A TOPS-20 style command parsing library
for personal computers
.sp 3
.fi
.Pp
Documentation and source code Copyright (C) 1985 by Mark E. Mallett;
permission is granted to distribute this document and the code
indiscriminately.  Please leave credits in place, and add your own
as appropriate.
.sp 2
.ce 1
A Disclaimer
.sp 2
.Pp
The code which implements this library takes up about 12Kb of space on
my CP/M system using Manx's AZTEC CII Z80 compiler (10Kb with the date/time
support stubbed out).  I don't claim that the coding is very efficient
nor do I make any other claims about the code in general.  I do believe
that the definition of the call interface is reasonable, and for me,
this has made it quite usable.
.bp
.sp3
.ce 1
This Document
.sp 2
.Pp
This document contains the following sections:
.Bb
.Bi
Document overview (this here section)
.Bi
Introduction and history
.Bi
Functional overview
.Bi
How to write programs using the subroutine library
.Bi
How to make the library work on your system
.Be
.bp
.sp 3
.ce 1
Introduction and History
.sp 2
.Pp
This document describes the COMND subroutine package for C programmers.
COMND is a subroutine library to effect consistent parsing of user
input, and in general is well suited for verb-argument style command
interfaces.  The library provides a consistent user interface as well
as a program interface which, I believe, could well remain unchanged if
the parsing library were re-written to support different interface
requirements (such as menu interaction).
.Pp
The COMND interface is based on the TOPS-20 model.  TOPS-20 is an operating
system which is/was used by Digital Equipment Corporation on their
PDP-20 computer.  TOPS-20 was based on TENEX, written by BBN (I think,
I think).  TOPS-20 COMND is much more robust and consistent than the
library which this document describes; this library being intended for
small computer applications, it provides the most commonly used functions.
.Pp
This library was written on a Z-80 system running Digital Research
Corporation's CP/M operating system version 3.0 (CPM+).  I have also
compiled and tried it on a VAX 11/780 running VMS.  It is completely
written in the C language, and contains only a few operating system
specific elements.
.Pp
The COMND JSYS section of the TOPS-20 Monitor Calls manual is probably
a good thing to read.
.Pp
Please note: while there are a few unimplemented sections of this
library, I felt that it was nevertheless worthwhile to submit it to
public domain since it is usable for almost all general command parsing
and since the call interface is well defined.  I have used this library
extensively since sometime in 1984.
.bp
.sp 2
.ce 1
Functional Overview
.sp 3
.Pp
The COMND subroutine library provides a command-oriented user interface
which is consistent at the programmer level and at the user level.
At the program level, it gives an algorithmically controlled parsing
flow, where a call to the library exists for each field or choice
of fields to be parsed.
.Pp
At the user level, the interface provides:
.Bb
.Bi
Command prompting.
.Bi
Consistent command line editing.  The user may use editing keys to erase
the last character or word, and to echo the current input line and
prompt.
.Bi
Input abbreviation and defaulting.  The user may type abbreviations of
keywords, or may type nothing to have defaults applied.
.Bi
Incremental help.  By pressing a known key (usually a question mark),
the user can find out what choices s/he has.
.Bi
Guide strings.  Parenthesized guide words are shown at the users option.
.Bi
Command completion.  Where the subroutine library can judge what the
succesful completion of a portion of user input will be, the user can
elect to have this input completed and shown automatically.
.Be
.bp
.sp 2
.ce 1
Using the COMND Library
.sp 2
.Pp
While you read this part of the document, you might want to look at the
sample program named TEST.C which has been included with this package.
It is an over-commented guide to the use of the COMND library.
.Pp
Any module which makes use of this library shall include the definition
file named "comnd.h".  This file contains definitions which are necessary
to the caller-library interface.  Mnemonics (structures and constants)
mentioned in relation to this interface are defined in this file.
.Pp
The philosophy of parsing with the COMND library is that a command line
is typed, the program inspects it, then the program acts on the directions
given in that line.  This process is repeated until the program finishes.
The COMND library assists the user in typing the command line and the
program in inspecting it.  Acting on it is left up to the calling program.
.Pp
The typing and parsing of fields in the command line go essentially
hand-in-hand with this library.
The single subroutine COMND() is used to effect all parsing.  This routine
is called for each field of the input line to be parsed.  Parsing is
done according to a current parse state, which is maintained in a
parameter block passed between caller and library.  The state block
contains the following sort of information (described in detail later):
.Bb
.Bi
What to use for a prompt string.
.Bi
Addresses of scratch buffers for user input and atom storage.
.Bi
How much the user has entered.
.Bi
How much of the line the program has parsed.
.Be
.Pp
An important thing to note is that the indexes (how much entered
and parsed) are both variable.  The program begins parsing of the
input line upon a break signal by the user (such as the typing of
a carriage return, question mark, etc).  The user may then resume
typing and erase characters back to a point BEFORE that already parsed.
It is very important that the program does not take any action on
what has been parsed until the line has been completely processed,
otherwise that action could be undesired.
.Pp
Since the user may back up the command input to a point before that
already processed by the application program, a mechanism must be
provided to backup the program to the correct point.  Rather than
going to the point backed up to, the COMND library expects the application
program to return to the beginning of the line, and start again.  The
user's input has remained in the command line buffer,
and the library will take care of
buffering the rest of the input when that parse point is again reached.
However, this means that there must be
a method of communicating to the calling program
that this "reparse" is necessary.  Actually there
are two methods provided, as follows:
.Bb
.Bi
Each call to the command parsing routine COMND() yields a result code.
The result may indicate that a reparse has to take place.  The program
shall then back up to the point where the parse of the line began, and
start again.
.Bi
The application program may specify the address of a setjmp buffer
which identifies the reparse point.
(Note setjmp is a facility provided as part of most standard C libraries.
It allows you to mark a point in the procedure flow [call frame, registers,
and whatever else is involved in a context], and return to that point
from another part of the program as if control had never proceeded.  If
you are unfamiliar with this facility, you might want to find a description
in your C manual.)
It is up
to the caller to setup the setjmp environment at the reparse point.
.Be
.Pp
In either case, the reparse point (the point at which the parse will be
restarted if necessary) is the point at which the first element of the
command line is parsed.  This is after the initialization call which
starts every parse.
.sp 2
.Pp
Every call to the COMND() subroutine involves two arguments: a command
state block, in which is kept track of the parse state, and a command
function block, which describes what sort of thing to parse next.
The command state block is given a structure called "CSBs", and a
typedef called "CSB".  Each element of the structure is named with a
form "CSB_xxx", where "xxx" is representative of the element's purpose.
The following are the elements of the command state block, in the order
that they appear in the structure.
.Bb
.Bi
CSB_PFL is a BYTE.  This contains flags which are set by the caller
to indicate specifics of the command processing.  These flags are:
.Bb
.Bi
_CFNEC: Do not echo user input.
.Bi
_CFRAI: Convert lowercase input to uppercase.
.Be
.Bi
CSB_RFL, a BYTE value, contains flags which are kept by the library in
the performance of the parse.
Generally, these flags are of no interest to the caller since their
information can be gleaned from the result code of the COMND() call.
However, they are:
.Bb
.Bi
_CFNOP: No parse.  Nothing matched, i.e., an error occured.
.Bi
_CFESC: Field terminated by escape.
.Bi
_CFEOC: Field terminated by CR.
.Bi
_CFRPT: Reparse required.
.Bi
_CRSWT: Switch ended with colon.
.Bi
_CFPFE: Previous field terminated with escape.
.Be
.Bi
CSB_RSB is the address of a setjmp buffer describing the environment at
the reparse point.  If this value is non-NULL, then if a reparse is
required, a longjmp() operation is performed using this setjmp buffer.
.Bi
CSB_INP is the address of the input-character routine to use.  If this
value is non-NULL, then this routine is called to get each character of
input.  No line editing or special interactive characters are recognized
in this mode, since it is assumed that this will be used for file input.
Note especially: this facility is not yet implemented, however the
definition is provided for future expansion.  Thou shalt always leave
this NULL, or write the facility thyself.
.Bi
CSB_OUT is the inverse correspondent to the previous element (CSB_INP).
It is the address of a routine to process output from the command library.
Please see the warning in the CSB_INP description about not being implemented.
.Bi
CSB_PMT is the address of the prompt string to use for command parsing.
The command library takes care of prompting, so make sure this is filled
in.
.Bi
CSB_BUF is the address of the buffer to put user input into as s/he is
typing it in.
.Bi
CSB_BSZ, an int, is the number of bytes which can be stored in CSB_BUF;
i.e., it is the buffer size.
.Bi
CSB_ABF is the address of an atom buffer.  Some (if not all) parsing functions
involve extracting some number of characters from the input buffer and
interpreting or simply returning this extracted string.  This buffer is
necessary for those operations.  It should probably be as large as the
input buffer (CSB_BUF), but it is really up to you.
.Bi
CSB_ASZ, an int, is the number of characters which can be stored in CSB_ABF;
i.e., it is the size of that buffer.
.sp 1
** Note ** CSB elements from here to the end do not have to be initialized
by the calling program.  They are used to store state information and are
initialized as required by the library.
.Bi
CSB_PRS, an int, contains the parse index.  This is the point in the command
buffer up to which parsing has been achieved.
.Bi
CSB_FLN, an int, is the filled length of the command buffer.  This is the
number of characters which have been typed by the user.
.Bi
CSB_RCD, an int, is a result code of the parse.  This is the same value
which is returned as the result of the COMND() procedure call.
.Bi
CSB_RVL is a union which is used to contain either an int or an address
value.  The names of the union elements are: _INT for int, _ADR for
address (note that a typecast should be used for proper address assignment).
This element contains a value returned from some parse functions which return
values which are single values.
For example, if an integer is parsed, its value
is returned here.
.Bi
CSB_CFB is the address of a command function block for which a parse was
successful.  This is significant in cases where there are alternative
possible interpretations of the next command line field.
.Be
.sp 3
The parse of each element in a command line involves, as well as the
Command State Block just described, a Command Function Block which
identifies the sort of thing to be parsed.  This block is defined
in a structure named "CFBs", which has a corresponding typedef
named "CFB".  Elements of the CFB, named "CFB_xxx", are as follows
(in the order they appear in the structure):
.sp 1
.Bb
.Bi
CFB_FNC, a BYTE, is the function code.  This defines the function to
be performed.  The function codes are listed, and their actions
described, a little later.
.Bi
CFB_FLG, a BYTE, contains flags which the caller specifies to the
library.  These are very significant, and in most cases affect the
presentation to the user.  The flag bits are:
.Bb
.Bi
_CFHPP: A help string has been supplied and should be given when the
user types the help character ("?").
.Bi
_CFDPP: A default string has been supplied, and shall be used if the
user does not type anything at this point (typing nothing means typing
a return or requesting command completion).  Note that this flag (and
the default string) is ONLY significant for the CFB passed in the call
to the COMND() routine, and not for any others referenced as alternatives
by that CFB.  Note further that a default specified by the first CFB
is applied to the input stream, and not to the parsing function.  That
means that the default is subject to interpretation by alternate CFBs,
and in fact, does not even have to be appropriate for the CFB which
contains it.
.Bi
_CFSDH: The default help message should be supressed if the user types
the help character ("?").  This is normally used in conjunction with the
_CFHPP flag.  However, if this flag is present and the _CFHPP is not
selected, then the help operation is inhibited, and the help character
becomes insignificant (just like any other character).
.Bi
_CFCC: A character characteristic table has been provided.  A CC table
identifies which characters may be part of the element being recognized.
Not all functions support this table (for example, it does not make sense
to re-specify which characters may compose decimal numbers).  This
table also specifies which characters are break characters, causing the
parser to "wake up" the calling program when one of them is typed.
If this bit is not set (as is usually the case), a default table is
associated according to the function code.
.Bi
_CFDTD: For parsing date and time, specifies that the date should be parsed.
.Bi
_CFDTT: For parsing date and time, specifies that the time should be parsed.
.Be
.Bi
CFB_CFB is the address of another CFB which may be invoked if the user input
does not satisfy this CFB.  CFBs may be chained in this manner at will.
Recognize, however, that the ORDER of the chain plays an important part
in how input is handled, particularly in disambiguation of input.  Note also
that only the first CFB of the chain is used for specifying a default
string and CC table for command wake-up.
.sp 1
CFB chaining is a very important part of parsing with this library.
.Bi
CFB_DAT is defined as a long, since it is used to contain address or int
values.  It should be referenced via typecast.  It is not defined as a
union because it is inconvenient or impossible to initialize unions at
compile time with most (all?) C compilers, and initialization of these
blocks at compile time is very desirable.
This element contains data used in parsing of a field in the command line.
For instance, in
parsing an integer, the caller specifies the default radix of the integer
here.
.Bi
CFB_HLP is the address of a caller-supplied help string.  This is only
significant if the flag bit _CFHPP is set in the CFB_FLG byte.
.Bi
CFB_DEF is the address of a caller-supplied default string.  This is
only significant if the flag bit _CFDPP is set in the CFB_FLG byte, and
only for the first CFB in the CFB chain.
.Bi
CFB_CC is the address of a character characteristics table.  This is only
significant if the flag bit _CFCC is set in the CFB_FLG byte.  This is
the address of a 16-word table, each word containing 16 bits which are
interpreted as 8 2-bit characteristic entries.  The most significant bits
correspond to the lower ASCII values, etc.  The 2-bit binary value has the
following meaning, per character:
.Bb
.Bi
00: Character may not be part of the element being parsed.
.Bi
01: Character may be part of the element only if it is not the first
character of that element.
.Bi
10: Character may be part of the element.
.Bi
11: Character may not be part of the element; furthermore, when it is typed,
it will case parsing to begin immediately (a wake-up character).
Note: wake-up via this mechanism is not implemented.
.Be
.sp 1
.Pp
Don't hesitate to use CC tables; they only take up 16 bytes apiece.
.Be
.sp 2
.Pp
The function code in the CFB_FC element of the command function block
specifies the operation to be performed on behalf of that function block.
Functions are described now.
.sp 2
.ne 15
.sp 1
CFB function _CMINI: Initialize
.Pp
Every parse of a command line must begin with
an initialization call.  This tells the command library to reset its
indexes, that the user must be prompted, etc.  There may be NO other
CFBs chained to this one, because if they are, they are ignored.
.Pp
The reparse point is the point right after this call.  If the setjmp
method is used, then the setjmp environment should be defined here.
After the reparse point, any variables etc which may be the victims of
parsing side-effects should be initialized.
.sp 2
.ne 15
.sp 1
CFB function _CMKEY: Keyword parse
.Pp
_CMKEY parses a keyword from a given list.  The CFB_DAT element of the
function block should point to a table of string pointers, ending with
a NULL pointer.  The user may type any unique abbreviation of a keyword,
and may use completion to fill out the rest of a known match.  The
address of the pointer to the matching string is returned in the CSB_RVL
element of the command state block.  The value is returned this way so
that the index can be easily calculated, and because it is consistent
with the general keyword parsing mechanism (_CMGSK).
.Pp
The incremental help associated with keyword parsing is somewhat special.
The default help string is "Keyword, one of:" followed by a list of
keywords which match anything already typed.  If a help string has been
supplied (indicated by _CFHPP) and no suppression of the default help is
specified, then the initial part ("Keyword, ") is replaced with the
supplied help string and the help is otherwise the same.  If a help string
has been supplied and the default has been supressed, then the given help
string is presented unaltered.
.sp 2
.ne 15
.sp 1
CFB function _CMNUM: number
.Pp
This parses a number.  The caller supplies a radix in the CFB_DAT
element of the function block.  The number parsed is returned
(as an int) in the CSB_RVL element of the state block.
.sp 2
.ne 15
.sp 1
CFB function _CMNOI: guide word string
.Pp
This function parses a guide word string (noise words).  Guide words
appear between significant parts of the command line, if they are in
parentheses.  They do not have to be typed, but if they are, they must
match what is expected.  If the previous field ended with command
completion, then the guide words are shown automatically by the parser.
.Pp
An interesting use of guide word strings is to provide alternate sets
with the command chaining feature.  The parse (and program) flow can be
altered depending on which string was matched.
.sp 2
.ne 15
.sp 1
CFB function _CMCFM: confirmation
.Pp
A confirmation is a carriage return.  The caller should parse a confirmation
as the last thing before processing what was parsed.  Since carriage return
is by default a wake-up character, requiring a confirmation will (if you don't
change this wake-up attribute) require that the parse be completed with no
extra characters typed.  A parse with this function code returns only a
status.
.sp 2
.ne 15
.sp 1
CFB function _CMGSK: General storage keyword
.Pp
This call provides for parsing of one of a set of keywords which are not
arranged in a table.  Often, keywords are actually stored in a file or
in a linked list.  The caller fills in the CFB_DAT element of the command
function block with the address of a structure named CGKs (typedef CGK),
which contains the following elements:
.Bb
.Bi
CGK_BAS: A base address to give to the fetch routine.  Does not matter what
this is, as long as the fetch routine understands it.
.Bi
CFK_CFR: The address of a keyword fetch routine.  The routine is called
with the CGK_BAS value, and the address of the pointer to the previous
keyword.  It is expected to return the address of the pointer to the
next keyword, or with the first one if the passed value for the previous
pointer is NULL.
.Pp
When this function completes successfully, it returns the address of the
pointer to the string in the CSB_RVL element in the command state block;
otherwise (for unsuccessful completion), it must return NULL.
Please see the description of the _CMKEY function code for a description
of help and other processing.
.Pp
Note: the General Keyword facility can be used to do special pre-processing
of candidate strings, such as hiding keywords which are not appropriate for
the program state, user, access, etc.
.Be
.sp 2
.ne 15
.sp 1
CFB function _CMSWI: Parse a switch.
.Pp
This is intended to perform switch matching, but it currently is not
implemented and will return a result code _CRIFC (invalid function code)
if you try it.  Basically it is a placeholder for an unimplemented function.
.sp 2
.ne 15
.sp 1
CFB function _CMTXT: Rest of line
.Pp
This function parses the text to the end of the line.  Note that this does
not parse the trailing break character (i.e. the carriage return).
The text is returned in the atom buffer which is defined (by the caller)
by the CSB_ABF and CSB_ASZ elements of the command state block.
.sp 2
.ne 15
.sp 1
CFB function _CMTOK: token
.Pp
This function will parse an exact match of a particular token.  A token
is a string of characters, whose address is supplied by the caller in
the CFB_DAT element of the command function block.  This function is
mainly useful for parsing such things as commas and other separators,
especially where it is one of several alternative parse functions.
It returns no value other than its status.
.sp 2
.ne 15
.sp 1
CFB function _CMUQS: unquoted string
.Pp
This function parses an unquoted string, consisting of any characters
other than spaces, tabs, slashes, or commas.  This set may of course
be changed by supplying a CC table.  The unquoted string is returned
in the atom buffer associated with the command state block.
.sp 2
.ne 15
.sp 1
CFB function _CMDAT: parse date/time
.Pp
This function parses a date and/or time.  The caller specifies, via
flag bits in the CFB_FLG byte of the command function block (as identified
above) which of date, time, or both, are to be parsed.  The date and
time are returned as the first two ints in the atom buffer which is
associated with the command state block.  Note that both date and time
are returned, regardless of which were requested.
.Pp
Note further that the _CMDAT function is not fully implemented as of this
writing.
.sp 4
.ce 1
Calling the COMND library
.sp 1
.Pp
All that you need to know to use the above information is how to call
the command library.  Basically, there is one support routine: COMND().
It is used like this:
.sp 1
.nf
         status = COMND (csbp, cfbp);
.fi
.Pp
Here, "csbp" is the address of the command state block, and "cfbp" is the
address of the command function block.
The COMND() routine returns an int status value, which is one of the
following:
.Bb
.Bi
_CROK: The call succeeded; a requested function was performed.  The address
of the matching function block is returned in the CSB_CFB element of the
command state block, and other information is returned as described above.
.Bi
_CRNOP: The call did not succeed; nothing matched.
.Bi
_CRRPT: The call did not succeed because the user took back some of what
had already been parsed.  In other words, a reparse is required, and your
program must back up to the reparse point.  Note that if you specify a
setjmp buffer address in the CSB_RSB element of the command state block,
you will never see this value because the COMND library will execute a
longjmp() operation using that setjmp buffer.
.Bi
_CRIFC: The call failed because you provided an invalid function code in
the command function block (or in one which is chained to it).  One of us
has made a programming error.
.Bi
_CRBOF: Buffer overflow.  The atom buffer is too small to contain the
parsed field.
.Bi
_CRBAS: Invalid radix for number parse.
.Bi
_CRAGN: You should not see this code.  It is reserved for a support-mode
call to the subroutine library.
.Be
.sp 2
.Pp
When you use the setjmp method of command reparsing, it is usually enough
to check the result against _CROK only, everything else being treated the
same (error in input).
.bp
.sp 2
.ce 1
Installing the COMND library
.sp 2
.Pp
This part of the document describes the modules which come with the
COMND library kit, and what you might have to look at if the code does
not instantly work on your system (which will probably be the case if
your system is not the same kind as the one which you got it from).
.Pp
The files which come in the COMND kit are as follows:
.Bb
.Bi
COMND.R - Source for this document, in a form suitable for the public
domain formatting program called "roff4".
.Bi
COMND.DOC - This document.
.Bi
COMND.EDT - Edit history for the subroutine library.
.Bi
MEM.H - A file of my (Mark Mallett) definitions which are used by the
code in the command subroutine library.
.Bi
COMND.H - Command library interface definitions.
.Bi
COMNDI.H - Command library implementation definitions.
.Bi
COMND.C - Primary module of the COMND library.  Contains user input
buffering and various library support routines.
.Bi
CMDPF1.C - First module of parse function processing routines.
.Bi
CMDPF2.C - Second module of parse function processing routines.
.Bi
CMDPFD.C - Contains the date/time parse function routines.  This is
included in a separate module so that it can be replaced with a stub, since
few programs (that I have written, anyway) use this function, and it does
take up a bit of code.
.Bi
CMDPSD.C - A stub for the date/time parsing functions.  This can be linked
with programs which do not actually use the date/time parse function.
.Bi
CMDOSS.CPM - Operating system specific code which works for CP/M.  This
is provided as a model for the routines which you will have to write for
your system.
.Bi
DATE.CPM - Date/time support routines for version 3.0 of CP/M.
This is a module containing routines to get the date and time from the
operating system, and to encode/decode these values to and from internal
form.  This is provided as a model; you will probably have to rewrite them
for your system.  Note, you don't need these routines if you don't use
the date and time parsing function (if you use the stub instead).
.Be
.bp
.sp 2
.ce 1
Your Improvements
.sp 2
.Pp
If you improve this library or make it work on a new operating system, I'd
appreciate hearing about it so that I can maintain a proper version.  Also
please maintain the edit/version history in the file COMND.EDT.
.sp 2
.nf
                                        Mark E. Mallett
                                        c/o M-TEK
                                        P.O. box 6357
                                        Nashua NH 03114

                                        voice: 603 424 8129
                                        data:  603 424 8124
                                           (1200 baud, allow system
                                            time to boot)
.fi

.ds F1
.nf
************************************************************************
************************************************************************
************************************************************************



		X    X    X    X     X    X    X
		X    X    XX   X     X     X  X
		X    X    X X  X     X      XX
		X    X    X  X X     X      XX
		X    X    X   XX     X     X  X
		 XXXX     X    X     X    X    X

		   XXXXXXX  X     X   XXXXX
		      X     X     X  X
		      X     X     X  X   XXX
		      X     X     X  X     X
		      X      XXXXX    XXXXX


		 (UNIX TERMINAL USER'S GUIDE)
.fi
.sp 2
.in 12
.ll 58
This textbook was written specifically for the UNIX systems operated
by the Department of Defense, and may contain references to features
available only on those systems.
.nf
.sp
.ll 65
.in 0
.ce 3
***********************************************************************
***********************************************************************
***********************************************************************

.fi
.sp 15
.tl '''Thomas Leary'
.tl '''March, 1980'
.ds H1 TUG
.bp
.rs
.sp 15
.in 10
.ll 58
.fi
.ti +16
Acknowledgment
.sp
The "UNIX TUG" is based upon the "Harvard-Radcliffe Student
Timesharing System Terminal User's Guide"(HRSTS-TUG). Permission
to use the HRSTS-Tug was graciously granted by Lew Law and Tucker
Taft of the Harvard Science Center. Inquiries concerning the
HRSTS-TUG should be directed to the Director of Technical
Services - Harvard Science Center, Cambridge, Mass.
.ll 65
.in 0
.; beginning of main body of text
.pn 1
.bp
.af % 1
.ds F1 - % -
.ds Fd
.nr aa -1
.P1 "Scope of this Manual"
The information presented in this manual is intended
as an introduction to UNIX as well as a beginners reference
guide to the commonly used portions of the system.
The vast majority of the topics covered are of a basic
nature. Where more advanced topics are introduced
they are identified by an asterisk following the section title.
This manual is not intended as a bible for the system,
but should guide the average user through getting
an account, getting "onto" the system, using available
programs and languages, getting "off" the system,
and interpreting and correcting common errors
encountered along the way.
.PP
For additional information consult the "PWB/UNIX User's Manual"
(obtainable from your system
administrator or from Joe Clemmons at 4734s).
.PP
The PWB/UNIX User's Manual contains the in-depth
information on commands, system calls, file structure and
protection, and file formats needed for systems
programming endeavors.
.PP
For technical assistance or general questions consult
your system administrator or someone from the UNIX
Support Group.
.bp
.P1 INTRODUCTION
.P2 "What is UNIX?"
UNIX is a general-purpose, multi-user, interactive
operating system developed by Bell Laboratories. It is currently run on the
Digital Equipment Corporation's 11-series computers as well as the
machines of several other manufacturers.
It was initially designed for research in operating systems, languages
and a host of other topics. However, it has been put to use in a wide variety
of ways, among which text preparation occupies a prominent place.
.PP
The original version of UNIX was introduced in 1969. The version
which is currently being used within the Agency is a revised
version that was released by Bell Laboratories in February 1971.
As a result of this revision, UNIX is
presently  a  system  of  many unique capabilities. It contains a
number of features seldom offered even in  much  larger  systems,
such as:
.sp
.L3
it allows use of upper and lower-case characters as well as
special characters in the formation of file names.
.L3
files are created directly at the terminal with very little
effort.
.L3
it allows a file to be created by alternate methods.
.L3
it has several facilities which may be used to edit a file.
.L3
it performs many functions by just using a few keys rather
than having to type a complete input line.
.L3
it has a  symbolic command language that is easy to use
.PP
There are many other unique features which could be cited; however,
we hope you will find that the most important characteristics of
UNIX are its simplicity, elegance, and ease of use.
.PP
Primary utilization of UNIX is the preparation and formatting
of documents and textual materials, and software for other systems.
.PP
UNIX supports some common computer languages such
as FORTRAN and BASIC
as well as some esoteric
languages such as C, PASCAL and LISP.
Also, there are a variety of games availabe on the system. They range in
complexity from simple games like hangman and tic-tac-toe, to a
multi-player world conquest game called EMPIRE.
.bp
.P2 "Where is UNIX?"
.PP
Several UNIX systems are located at both the Operations Building
and FANX. Most of these systems support terminal rooms as well as
terminals in the offices of regular users. A terminal connected to
ELROND is located in the FANX Learning Center. UNIX documentation
is close at hand for most of the "public terminals."  Also,
persons familiar with UNIX are not hard to find and are usually
more than willing to provide assistance.
.P2 "Network Facilities"
.PP
Most UNIX systems permit users access to a network of interconnected
systems. If you feel that you can profitably use the network facilities
get in touch with your system administrator. He should be able to
secure access privileges for you and provide user documentation relating
to the network.
.P2 "How to Get an Account"
.PP
Prospective users must submit an application for an account on the system.
If the application is approved, the new user will be issued a login
name and login password, and will be assigned to a group. Each user should
know his group ID, user ID and the names of the other members of his
group. For information about changing groups, finding groups and adding
members, consult the system administrator.
.PP
Each user is assigned a login directory and is empowered to create additional
directories. The user assumes responsibility for the contents of his
directories from both the security and housekeeping standpoints. Details
concerning the user's security responsibilities will be discussed in a
section of this document. The user's housekeeping responsibilities are
straightforward enough - junk files should not be allowed to accumulate.
Some housekeeping functions are performed by the system itself. There exists
a program called the "midnight skulker" which automatically deletes all
user files named a.out, *.o, *.bak, .{* and core  that have not been used
in the past fourteen days.
.P2 "UNIX Security"
.PP
In order to maintain a high level of security:
.L3 8 2
Each user should change his password immediately after getting a UNIX
account, and thereafter at least monthly, or whenever a password is
compromised.  In order to change a password, type "passwd",
the system will
then prompt for the old and the new passwords, and record the change.
Please note that passwords must have a minimum of five characters,
and should not be easily derived.  Do not use your phone number,
license plate, spouse's name, or birth date as a password.
.L3
Users should understand the meaning of modes of protection and ownership
for both ordinary files and directories.  All files may be created readable
by users other than the owner, and will remain readable unless the owner
takes explicit action to change the read/write/execute privileges.
The command to change file mode is "chmod";
a full explanation of available
options can be displayed by typing "help chmod".
.L3
Since files can be created readable it is important for all users to realize
that every unprotected file could be read by anyone on the PLATFORM network
(i.e., contractors, employees involved in other projects, thrill seekers, idle
browsers, etc.).  The user must take precautions to secure
classified, or otherwise sensitive, data which is on the system, just as he
would to secure any other classified or sensitive information.
Since systems vary in terms of clearance levels, the user must check with
the systems administrator to find out the clearance level for a specific
system.
.L3
Line printer output which is classified must be so designated by the user.
The "opr" command used to print out files has a
.I
-C"classification"
.R
option which prints the desired classification on the burst page.  An
example of this is
.I
opr -C"hush-hush"
.R
[filename].  Classified listings should be picked up promptly, and the user
should take care to pick up only his own listings.
.P2 "Consulting and User Assistance"
.PP
The user's first source of assistance is the systems administrator.
If the systems administrator is unable to resolve the problem, contact one
of the Agency gurus.
Mr. Howard Rumerman, T3332, 4052s, will be able to give the name
of someone who may help you.
.ne 7
.PP
If the problem does not require immediate attention, try the mail program.
The following names are standard and should get some response:
.sp
.L3
gripe  - complaints regarding operations or administration
.sp
.L3
system - general complaints about the software
.sp
.L3
guru   - complaints of a serious nature with wide impact
.PP
The "who" and "write"
commands that are discussed below
can often be helpful in getting immediate assistance
from other users.  Look for the login names root, bin, or unix.
.P3 "Terminal Malfunctions"
.PP
Occasionally, terminals are known to misbehave.  In the event that
the terminal does not recognize or respond to commands typed on the keyboard,
or does not prompt for a login:
.sp
.L3 8 2
Check to see that the terminal is turned on, plugged in, and connected
to a comm line.
.L3
Check to see if the terminal has gone off-line, if it has, depress the
on-line button on the upper right of the keyboard to get back on-line.
.L3
If the terminal is on-line, hit the break key, then return.  If after several
attempts there is still no response, turn the terminal off, turn it back on
again and try to log-in. For persistent problems contact operations.
.E3
.PP
For further information, read the Programmer Information Bulletin entitled,
"The Care and Feeding of Terminals" (see sec 4.4.1 of this doc.) and
Chapter 7 of the PWB/UNIX User's Manual, which contains descriptions of each
terminal type.
.P2 "On-line Documentation Aids"
.PP
In addition to the printed documentation described below
in sections 1.8, 1.9 and 1.10 a considerable body of UNIX
documentation is kept on-line.  Also, there are systems programs
available to produce copies on your terminal, etc..
If you are not using a hard copy terminal, the material will whiz
by on the CRT screen too fast to read.  For this reason, when any of
the commands  discussed in this section are issued, add
the command "br"(for browse) as a filter. This will allow you to view
the output one page at a time. The exact manner in which the
browse command is used is illustrated in the following section.
.br
.ne 8
.P3 "The UNIX Manual"
.PP
The user's manual is kept online so that the user may gain access to it
at any time.  To do so the user must type in this line:
.sp
.ti 5
man [chapter in which your topic is located] [topic].
.sp
An example of this is the following:
.sp
.ti 23
man 1 who
.sp
which will give the user the manual
page of the "who" command in chapter I.
As noted above, to view the output one
page at a time you use the "br" command as a filter:
.sp
.ti 22
man 1 who | br
.sp
To obtain a copy on the line printer one types:
.sp
.ti 22
man 1 who |opr
.sp
To obtain a copy of the table of contents use
the following command with the "opr" or "br" filters
as desired:
.sp
.ti 22
man 0 toc
.sp
.in 9
.ti -9
Note: (a)The use of filters or pipes is explained in some detail
in section 3.6.2. Essentially, a pipe will cause the output of
the program to the left of the pipe to be used as input to the
program to the right of it.
.br
.ti -9
~~~~~~(b)The last character on each command line must be the
newline character, which is entered by hitting the RETURN key.
For the sake of simplicity, the newline character will not be included
in the examples in this text. However, the user should keep in mind the fact
that commands are not executed until the RETURN key is hit.
.in -9
.P3 Help
.PP
A second on-line aid is the "help"
command.  This command by itself
will display a list of topics for which helpful information
is available. Help on a specific topic can be obtained
by using the command with the topic as an argument.
For example:
.sp
.ti 25
help fortran
.sp
For help in correcting errors the user should
use the "help" command in the following fashion:
.sp
.ti 23
help [error number]
.sp
.ne 5
For further information read
the description of "help"  in chapter I of the manual.
.P3 News
.PP
A third on-line aid is the "news" command. Like the "help"
command, if the user types "news"
a list of available news items will be displayed.
By entering a command of the following general form:
.sp
.ti 26
news [item]
.sp
information on any one of a number of available topics can be obtained.
.br
.ne 10
.P3 "Other manuals and documents"
.PP
On-line documentation exists for manuals and Programmer Information
Bulletins (PIB).  To obtain printed copies contact the operations staff,
or, Joe Clemmons, T12.
.br
.ne 10
.P4 "Documents and User Manuals"
.PP
There are several specialized manuals maintained on-line.
These manuals are of less wide-spread interest than the major
UNIX manuals.  They cover a specific software product with more
detail than the UNIX manual page.
The manuals available at this time are the following:
.sp
.in 5
.nf
\(sq  bas       \- The UNIX basic manual
\(sq  ded       \- Directory Editor (Graphical Shell)
\(sq  f4p       \- FORTRAN IV Plus
\(sq  fortran   \- RT-11 FORTRAN IV
\(sq  lisp      \- Univ. of Md. Lisp Interpreter
\(sq  ms        \- UNIX Mail System
\(sq  roff      \- Runoff (ROFF) Tutorial
\(sq  tbl       \- Manual on the tbl program
\(sq  trekman   \- Star Trek (UCB Version)
\(sq  wbs       \- Work Breakdown Structure
		   (Project Management)
.sp
.in -5
.fi
These manuals can also be obtained from operations.
.PP
The command "doc [manual name]"
will cause a copy of the manual to be
displayed on your terminal.
The argument, "manual name", is selected from the list of available manuals.
.sp
.in 6
.ti -6
Note: the doc command will cause the document to appear on the screen
via a scrolling up action that is without pauses. To cause it to appear
a page at a time the program should be piped to the br program.
.in -6
.br
.ne 5
.P4 "Programmer Information Bulletins"
.PP
Programmer Information Bulletins (PIBs) are used to provide
short notes and details on some esoteric features of UNIX.  They
also are used to provide information on items of general
interest.
.ne 20
The following Programmer Information Bulletins are available
as of the distribution date of this terminal user's guide:
.sp
.in 5
.nf
pib-76-002~~~~~~The Care and Feeding of Terminals
pib-76-005~~~~~~USERS' GUIDE TO THE "EM" MACROS
pib-76-006~~~~~~UNIX/H-6060 File Transfers
pib-76-007~~~~~~UNIX <--> 370 Tape File Transfers
pib-76-009~~~~~~An Abecedary to Xeroxing
pib-76-011~~~~~~RJE Status Commands for the 370
pib-77-002~~~~~~Improvements in the Rand Editor
pib-78-001~~~~~~Application for Account on UNIX
pib-78-002~~~~~~Version 7 C Compiler
pib-78-003~~~~~~Information for C Programmers
pib-78-004~~~~~~Information Guide For UNIX
pib-78-005~~~~~~FORTRAN IV Plus for UNIX
pib-78-006~~~~~~User's Guide to the "NM" Macros
pib-78-007~~~~~~Supplement to the UNIX Manual
pib-78-008~~~~~~8080 Cross Assembler
pib-78-009~~~~~~NROFF/TROFF
pib-78-010~~~~~~PORTS
.in -5
.fi
.PP
To obtain a list of Programmer Information Bulletins type
"doc pib"
and a list will be displayed on your terminal.
.PP
To obtain a given bulletin type
"doc pib [yy ###]", where
yy is the year and ### is the number of the bulletin.
.P2 "UNIX Documentation"
Each user should have the following documents:
.P3 "The PWB/UNIX User's Manual"
.PP
"The PWB/UNIX User's Manual"
consists of 10 chapters of which 8 are available in hard copy
(the other two are on-line).
Review the manual material to familiarize yourself
with its contents and style.  Read the forward and introduction.
The table of contents will give a list of available system
programs, monitor calls, and library routines.
A permuted index is useful for finding the correct entry
for a page.
.br
.ne 20
.sp
The chapters are arranged by topic in the following manner:
.sp
.nf
.in 8
Chapter~~~~~~~~~Topic
.sp
I~~~~~~~~~~~~~~~Commands
II~~~~~~~~~~~~~~System Calls
III~~~~~~~~~~~~~Subroutines
IV~~~~~~~~~~~~~~Special Files (Devices)
V~~~~~~~~~~~~~~~File Formats and Conventions
VI~~~~~~~~~~~~~~User Maintained Programs
VII~~~~~~~~~~~~~User Maintained Subroutines
VIII~~~~~~~~~~~~System Maintenance
IX~~~~~~~~~~~~~~FORTRAN IV Plus
X~~~~~~~~~~~~~~~INGRES
.in -8
.fi
.P3 "Documents for the PWB/UNIX Time-Sharing System"
.PP
The
"Documents for the PWB/UNIX Time-Sharing System"
contains information on the following topics:
.sp
.nf
.in 5
PWB/UNIX Documentation Roadmap
PWB/UNIX - Overview and Synopsis of Facilities
The UNIX Time-Sharing System
The Unix Time-Sharing System - A Retrospective
UNIX for Beginners
A Tutorial Introduction to the UNIX Text Editor
Advanced Editing on UNIX
PWB/UNIX Shell Tutorial
NROFF/TROFF User's Manual
PWB/MM - Programmer's Workbench Memorandum Macros
PWB/UNIX Manual Page Macros
Typesetting Mathematics - User's Guide(Second Edition)
C Reference Manual
Programming in C - A Tutorial
A General-Purpose Subroutine Library for PWB/UNIX
A New Input-Output Package
.in -5
.fi
.sp
It must be emphasized that the documents listed above
represent only a cross-section of those available in the
"Documents for the PWB/UNIX Time-Sharing System."
For a full listing the user should consult the document's table
of contents.
.sp
For the beginner intent on making his way through the documents
the entry "PWB/UNIX Documentation Roadmap" constitutes a
good starting point. Two other documents that the novice UNIX user
should consult are "UNIX for Beginners" and
"Tutorial Introduction to the UNIX  Text Editor."
.br
.ne 8
.P3 "Supplement to the PWB/UNIX User's Manual"
.PP
The "Supplement to the PWB/UNIX User's Manual"
contains manual pages which are to be added to or replace pages in the
"PWB/UNIX User's Manual".
These reflect changes to existing software or software added to
the UNIX systems since the original software was distributed by
Western Electric.  Included are a new Table of Contents and a new
Permuted Index, both of which
should be scanned carefully, since they may be the only way
older users become aware that potentially useful
software has been installed.
.P3 "UNIX Reference Card"
.PP
The UNIX Reference Card is a handy list of UNIX commands (including
NROFF and TROFF), which provides a brief description of every command it
contains.  It is an excellent short-cut for finding commands, but it is no
match for the User's Manual in terms of detail and completeness.
Also note that the URC dates from l975, and does not reflect more recent
modifications made to the software.
.P3 "GTSS Documentation"
.PP
There are a variety of GTSS documents that can be obtained from Joe
Clemmons of T12. The following documents relate to Platform network use:
.sp
.L1 8 2
User Telnet (GTSS Document #5) - This is a user's guide for the user
telnet program. User telnet allows local users to access portions of
the Platform network.
.L1
User's Manual for UNIX Controller FTP (GTSS Document #4) - This document
specifies the commands that are recognized by the controller for file
transfer protocols as well as what responses are to be expected from the
controller.
.L1
User's Manual for UNIX File Handling - File Transfer Protocol (GTSS
Document #2) - This document specifies how the File Handling - File Transfer
Protocol program is used in transfering files across the Platform network.
.L1
GTSS Software Manual (GTSS Document #1) - This manual documents the software
which comprises the current version of the Generalized Terminal Subsystem
(GTSS).
.sp
.in 0
Additional GTSS documentation is available on topics that are of a more
specific nature and/or less widespread utility:
.sp
.L1
Data Entry and Display Subsystem User's Guide (DEDS) [GTSS Document #6] -
This document is intended to show C Language programmers how to display
formatted data on the terminal screen and store data for later retrieval
using the DEDS software package. (Warning: DEDS does not work on all
terminals)
.L1
TSQUARE: Terminal Subsystem for Query and Retrieval (GTSS Document #3) -
This document serves as a user's guide to TSQUARE. The originators of
TSQUARE describe it as a modest UNIX-based query and retrieval system.
.L1
Platform FTP User and Technical Manual - This document specifies the
commands that are recognized by the File Transfer Protocol program.*
.E1
.sp
* Platform FTP is currently being phased out in favor of Controller FTP
.sp
.P2 "Advanced Reading"
.PP
The documentation described in the previous section is insufficient
for those who want to write their own programs
in C or assembler, or who intend to do serious work
in the writing of manuals, reports and other kinds of documents.
.br
.ne 10
.P3 "Text Processing"
.PP
People working with the preparation of documents
should begin by consulting Section T of the
"Documents for the PWB/UNIX Time-Sharing System"
(several of the entries in this section have been referenced above).
Two entries in this section are particularly useful:
"NROFF/TROFF User's Manual" and "Typing Documents
on the UNIX System: The -ms Macros for TROFF and NROFF."
Another macro package used in conjunction
with NROFF is documented in the on-line PIB:
"USERS' GUIDE TO THE "NM" MACROS"(PIB-78-006)
There is also a manual page for each macro set:
"MM(I)", "MS(VII)", "TMAC.NAME(VII)", and "MNM(VII)".
.P3 "Programming"
.PP
People planning to write programs for UNIX should learn the C language.
A good place to start is with the book
"The C Programming Language"
by Kernighan and Ritchie,  (Prentice-Hall, 1978). Copies of this
are available in the Central Library.  There are two PIB's available
( PIB-78-002 and PIB-78-003 )
which give useful information on C programming.
.PP
C-related topics are covered in two entries in
the "Documents": "A New Input-Output Package" and
"A General Purpose Subroutine Library for PWB/UNIX."
Also, chapters 2,3,4,5 and 7 of the "PWB/UNIX User's
Manual" contain information that is directly applicable
to C programs.
.P2 Training
.PP
Two courses are available through the Training School:
.sp
.nf
.in 5
\(sq  A self-paced course on UNIX for the
    Beginner, MP-ll9
.br
\(sq  A self-paced course on C programming, MP 220
.in -5
.fi
.PP
Other courses and seminars can be organized by the school
if there is sufficient need.  Contact your training co-ordinator
for more details.  Special advanced seminars can also be scheduled through
T3332 if sufficient operational need exists.


.bp
.af % 1
.ds F1 - % -
.ds Fd
.ds H1  ~
.ds H1 TUG
.P1 "The UNIX Shell"
.P2 "What is the UNIX Shell?"
.PP
On UNIX the standard command interpreter is called the
UNIX Shell (or simply the "shell").
This is a program which interprets your
typed commands and issues the appropriate system
requests to have the commands executed.
The shell has been designed to make interaction
with UNIX as simple as possible and to prevent users
from making careless mistakes.
.P2 "Command Format"
.PP
When the shell is ready to accept
commands it types the prompt "%" followed
by a space.
.PP
The general format of shell commands is:
.sp
.ti 8
% command-name argument argument . . .argument<nl>
.sp
The "%" is typed by the shell and the user types
the command name, a space, and then
any arguments to the command.  The arguments are 
separated from each other by a space.  The <nl>
represents a newline character which is appended
to the end of the command line by hitting the carriage return.
It serves as a signal to
the shell that the command is complete and that execution
may begin. As noted above, it will not be included in forthcoming
command examples.
.PP
As an example of how UNIX shell commands work consider the
following command:
.sp
.ti 21
% mv automobile car
.sp
This is the "mv" command (short for move) with the
arguments "automobile" and "car".  Here the arguments
are file names, and the "mv" command changes the name
of the file "automobile" to "car" (by moving the first file to
the second and then removing the first one).
.sp
Consider the following command:
.sp
.ti 26
% date
.sp
.ne 10
This is the "date" command which requests the
date and time.  The command has no arguments.  After
the user types the above, the system responds
.sp
.ti 15
Thu Feb 7  12:57:57  EST 1980
.br
.ti 15
%
.ne 4
.sp
The computer has typed the date and time, as requested, followed
by a new prompt, telling the user that the
system is ready to accept another command.
.ne 3
.PP
Several commands may be typed on a single line if
they are separated by semicolons.
.ne 6
.P2 "Command Abort"
.PP
A command can be aborted by typing "@"
(see sec. 2.2.1)
at any time before typing the final <nl> which
actually starts the execution of the command.  
The system starts on a new line
to indicate that the command was aborted.
RUBOUT and BREAK will usually have the same effect as "@,"
but "@" is the preferred method of aborting a command.
.ne 5
.P2 "Help: Requesting Information on Various Topics"
.P3 "Help: The Shell Program"
.PP
"Help" is a shell program which provides information about a wide
range of UNIX commands, system calls, programs, programming
languages and games. The format of the help command, which could
not be more simple, is as follows:
.sp
.ti 24
help <topic>
.sp
You can obtain a list of the topics with which the help command can
be used by typing help (followed, of course by a carriage return).
With a small group of programs (telnet, pftp-srvr and pftp-user)
the format of the help command is slightly different:
.sp
.ti 20
help <program> <topic>
.PP
As an example of how the help program works consider the following.
The command:
.sp
.ti 22
help telnet tenex
.sp
will produce the following display:
.br
.ne 15
.sp
.nf
______________________________________________________________
|[set]  tenex                                                 |
|           Purely  a  convenience  command,  this  puts   the|
|           terminal  into  charmode  with local echoing off -|
|           the preferred mode for TENEX connections.  See the|
|           `charmode'    and   `echo'   commands,   and   the|
|           elaboration on connections for more detail.       |
|_____________________________________________________________|
.sp
.fi
Though help seldom returns a detailed response to your query,
the response is adequate for most purposes.
.P3 "The Help Feature Built-In"
.PP
Some of the programs that run on UNIX have the help feature built
into them. By typing a question  mark in response to a prompt,
the system will display a list of commands that can be used with
the program. The built-in help feature can be illustrated by the
msg program. In response to the command "msg,"  the following
display will appear:
.sp
.nf
_____________________________________________________________
|Readmail -- version 2.3 of 3/2/79                           |
| 1   160 char  1 Feb 1980 elrond at ELROND "password change"|
|1 message                                                   |
|For help type ?                                             |
|->?                                                         |
|                                                            |
|Commands are:                                               |
|b[ack-up and print message]                                 |
|c[urrent message is] <message no> in file <filename>.       |
|d[elete] <message no>...                                    |
|e[xit and overwrite current mail file to reflect deletions] |
|                                                            |
|____________________________________________________________|
.fi
.sp
The list of commands has been abbreviated, since what is shown
above more than suffices for the purposes of illustration.
.P2 "UNIX Shell Variables (*)"
.PP
The Unix shell employs twenty-six string variables, $a through $z,
which offer the user a means of storing information for quick
retrieval. Retrieval is accomplished through a simple shorthand notation.
In general, shell variables can be used in two ways - interactively and as a
component of shell programs. The manner in which shell variables are used
interactively will be illustrated below. The use of these variables in shell
programs is a more complex topic which will not be covered here. At the
conclusion of this section a short bibliography of documents relating
to shell programming has been appended.
.br
.ne 10
.P3 "Using Shell Variables Interactively"
.PP
Shell variables can be broken down into two categories. Those in the
range $a through $m are initialized to null strings at the begining
of execution and are modified only by  explicit user request. In the second
category of shell variables, ranging from $n through $z, there are
several that are given specific initial values. These values can in most
cases be modified during execution.
.P4 "User-defined Shell Variables"
.PP
The way in which user-defined shell variables work is best illustrated
by practical examples. Suppose that you are frequently using several
directories and would like to have an easy way to refer to files in
these directories. You can do this by defining the shell variables "$a"
and "$b" in the following fashion:
.sp
.ti 20
= a /usr/tml/tug
.sp
.ti 20
= b /usr/tml/hold
.sp
This is a simple assignment of a shell variable. (Note: if the argument
that is being passed to "$a" contains any special characters it must be
surrounded by quotes.) The information that is stored in "$a" can now
be recalled by a command like "echo $a", which will cause the contents of
the shell variable to be displayed on the CRT screen.
The same assignment of a current working
directory to "$a" could be accomplished in a slightly different fashion
by piping the output of the "print working directory" program
(pwd for short) to the variable.
This is done in the following way:
.sp
.ti 25
pwd | = a
.sp
More will be said about the use of pipes in a section which follows.
.PP
Before proceeding to a consideration of the pre-defined shell variables
it should be noted that user-defined variables can be used
as arguments to UNIX commands. For example, suppose you have the
variables "$a" and "$b" defined in the following way:
.sp
.in 20
= a /usr/tml/tug
.sp
= b /usr/tml/hold
.in -20
.sp
The following example of the copy command (which has the general form:
cp <old file> <new file>):
.sp
.ti 18
cp $a/chap1 $b/hold1
.sp
will cause a copy of the file chap1 in the tug
directory to be placed in the file hold1 in the directory named hold.
.PP
As a final example of the use of user-defined shell variables consider
the following:
.sp
.in 15
cd /sys/source/s1/cref
.br
pwd |= a
.br
cd /pwb2/source/s1/cref
.br
pwd |= b
.br
ls -l $a/cref.c
.br
ls -l $b/cref.c
.br
ls -l $a/cref.c $b/cref.c
.in -15
.ne 10
.P4 "Pre-defined Shell Variables"
.PP
The shell variables with special meanings work in the same way as
the user defined variables. The only difference between the two,
obviously, is that it is not necessary to assign arguments to the
pre-defined variables. As an example of how the pre-defined variables
work consider the following:
.sp
.ti 29
$s
.sp
This variable is initialized to the user's login directory (and
as a result the login directory becomes the default directory).
You can substitute this in a UNIX command in the same way in which you
substitute user-defined shell variables. (For example, cp $s/chap1
$s/hold/hold1, with /usr/tml being the login directory,
will cause the file "chap1" in "usr/tml" to be copied into
the file "hold1" in "/usr/tml/hold".)
It is possible for the user to modify most of the pre-defined variables.
(For example, $s could be changed from the login directory to
some other directory and that directory would then become the default
directory.) This is a risky operation, however, because most of the
pre-defined variables were established for a reason.
.PP
For a description of the full range of pre-defined shell variables see
the section entitled SH(I) in "PWB/UNIX User's Manual. The same material
is covered in "PWB/UNIX Shell Tutorial" in the "Documents for the PWB/UNIX
Time-Sharing System."
.P3 "Using the Shell Variables in Shell Programs"
.PP
As noted above a discussion of the use of shell variables in shell
programming is beyond the scope of this user's guide. If the reader
is interested in pursuing the topic of shell programming it is
suggested that the following documents be consulted in the order listed:
.sp
.E3
.L3
"UNIX Time-Sharing System," Documents, see section 6
.L3
"UNIX for Beginners," Documents, see section entitled "The Shell"
.L3
"SH(I)," "PWB/UNIX User's Manual"
.L3
"The Unix Time-sharing System - A Retrospective," Documents,
see section entitled "User Interface"
.L3
"PWB/UNIX Shell Tutorial," Documents
.E3
.sp
Note: "Documents" refers to "Documents for the PWB/UNIX Time-Sharing
System"
.br
.ne 10
.P2 "Redirection of Input and Output and Pipes"
.P3 "Redirecting Input and Output"
.PP
Another very useful feature of the shell is the 
redirection of input and output.  Most commands
are written to use communication channels
called the standard input and output.
Usually these channels  communicate with your
CRT. However, this may be changed so that the
command reads input or writes output on the file which
you specify.
To change the standard input add "<filename" to the 
command.  To change the standard output add
">filename" to the command.  For example, if you had
a file named "input.txt" which contained the input for a
fortran program named "prog1.out",
.sp
.ti 18
prog1.out <input.txt
.sp
would use the file "input.txt" as the input to the program.
The command "date >output.txt" would
put the response from the date command into the file "output.txt".
.PP
The standard input and output are restored after the 
command's execution.
.sp
.P3 "Pipes"
.PP
Pipes are another very useful feature.  Typing a vertical
bar(|) between two commands means that the output
of the first command is used as input to the next
command.  For example, you might want a line-printer copy
of your output. If the file is executable, you
simply type a command of the following sort:
.sp
.ti 20
toy.out | opr
.sp
which causes a FORTRAN program to be executed and its output
sent to the line printer.
If the file is not executable it must first be directed to some program.
The output from this program can then be piped to the line printer.
For example, consider the following command:
.sp
.ti 20
nroff junk | opr
.sp
This directs UNIX to use the file "junk" as input to the text formatting
program (i.e. nroff) and then "pipe" the output of the program to the
line printer.
.br
.ne 10
.P2 "UNIX Commands"
.PP
In this section short descriptions of the most popular UNIX
commands will be presented.
.br
.ne 8
.P3 "Information Commands"
.sp
.I "question mark(?)"
.PP
"?" lists the complete set of UNIX commands.
.sp
.I help
.PP
"help" is used to obtain helpful information
about UNIX, the shell, and various programs.
.sp
.ti 27
% help
.sp
will list the available help topics.
.sp
.ti 24
% help snafu
.sp
will list helpful information on topic "snafu".
.sp
.I who
.PP
"who" lists the Lognames of the users
logged onto the system.
.sp
.I where <ttyx>
.PP
"where", with a terminal identifier as an argument,
will display the location of the specified terminal.
For example, "where ttyp" will produce the following display:
.sp
.ti 13
ttyp  A1D63 FANX 2 R812  7379
.sp
which gives you the room number, building, division and
secure phone number associated with the specified terminal.
.sp
.ne 5
.I name
.PP
"name" will give static information about
any user with an account on the system.  It expects
a list of Lognames (see Login {II.1.E}) or last-names.
For example, "name tml" will yield the following display:
.sp
.ti 13
tml	R812	Thomas M. Leary
.sp
which gives you the user's logname, division and full name.
.sp
.ne 5
.I date
.PP
"date" responds with the current date
followed by the time of day, e.g.
.sp
.ti 15
% date
.ti 15
Wed Feb 13 15:02:05 EST 1980
.sp
.I ss
.PP
"ss" (for system status) prints a list consisting of the following
items: the terminal identifier, the state of the particular process
(run, slp, etc.), the user ID, the process ID, the priority of the
process, the size of the process, the amount of core time used so far
and the command which initiated the process. The display generated
by ss will resemble the following
.sp
.nf
TTY STATE    UID    PID     PRI     SIZE     TIME  COMMAND
p:  *slp     unix  2345   PSLEP   29+ 10     0:21  opr
h:  *RUN*    tml     78     108        5     0:09  e expand.c
.fi
.sp
.I du
.PP
"du" (for disk usage) gives the number of blocks contained
in all files and (recursively) directories within each
specified directory or filename. The response generated by
the "du" command will resemble the following:
.sp
.in 15
% du /usr/tml/tug
.br
11	/usr/tml/tug/sys-6
.br
122	/usr/tml/tug/training
.br
1177	/usr/tml/tug
.in -15
.sp
The final figure represents the total number of blocks in
the directory. The preceeding two give the block totals
for two sub-directories.
.sp
.ne 15
.P3 "Terminal Status"
.sp
.I stty
.PP
"stty" (for set teletype) is used to set and test the status of
your terminal.  
The command stty with no arguments will list the
current characteristics of your terminal.
.sp
.in 13
speed 9600 baud
.br
erase = '#'; kill = '@'
.br
even odd -nl echo -tabs cr1
.br
Terminal type is Superbee Model 2
.in -13
.sp
The first line of the typeout contains the
speed at which you are communicating
with the system. The second line lists the erase and
kill characters that the system recognizes (coming from the
type of terminal that you are on). The third line lists the
type of parity allowed(both even and odd in this case) and
indicates that a carriage return is allowed for a newline(-nl),
that all characters typed are echoed back, that tabs are replaced
by spaces when printing and that the delay for the carriage
return is set for approximately .09 seconds(cr1). The fourth and
final line of the typeout displays the system's idea of the
type of terminal being used.
.sp
.I "stty [arguments...]"
.PP
The stty command can be used with a set of arguments to set
certain I/O options on the current output typewriter.
There are many characteristics which vary from
terminal to terminal.  For example, some terminals
have built in tab stops, and a special character directs
the terminal to advance to the next stop.  For those
terminals which don't have this feature, it is
necessary for the system to simulate a tab by
expanding it into a number of spaces.  Also, some terminals
(such as the Delta Data 7000) can operate at a variety of speeds,
and it is often convenient to speed them up or slow them down.
For a fuller explanation of how the stty command works and a
list of valid arguments see STTY(I) and STTY(II) in the
"PWB/UNIX User's Manual."
.sp
.I "Normal mode"
.PP
Many programs change some of the characteristics
of the terminal while they are running.  In teco, for example,
<esc> prints as a dollar sign.  If these programs complete in a normal
fashion they restore the terminal to the state it was in
before entering the program.  If, however, the program
terminates abnormally, you may have to restore
your terminal to its proper state.  This may be done by typing:
.sp
.ti 23
% stty normal
.sp
Your "preferred state" may differ from the "normal" one,
necessitating additional changes.
.ne 10
.P3 "File and Directory Manipulation"
.sp
.I ls
.PP
"ls" (for list) lists the name of some or all of the files in your current
directory. Thus, the command:
.sp
.ti 25
% ls
.sp
will list all of the files in your current directory. The "ls"
.br
.ne 10
command can be used with wildcard characters to
display certain categories of files within your current
directory. For example, the command:
.sp
.ti 24
%ls *.bak
.sp
will cause all the backup files to be displayed. If  you
want to display the files in some directory other than your
current one it is necessary to specify the complete pathname
of the directory. The following command illustrates this:
.sp
.ti 21
% ls /usr/tml/MP119
.sp
.I "ls -l"
.PP
"ls -l" is similar to the "ls"
command, but it gives more information about each file.
It includes the size (in bytes or characters), owner,
modification date, protection information on each file
and the number of links to the file. (If one also
wants to know how many blocks the file occupies on the disk
it is necessary to type the command "ls -s".)
.sp
.I mv
.PP
"mv" (for move) changes the name of a file in your
directory.  If the new filename already exists, it
will be deleted. The format of the command is as
follows:
.sp
.ti 15
% mv <old filename> <new filename>
.sp
.I cp
.PP
"cp" (for copy) makes a duplicate copy of a file
with a new name.  After giving a copy command
you will have two files, identical except for
their names. The format of the cp command
is identical to that of the mv command.
.sp
.ne 5
.I rm
.PP
"rm" (for remove) removes a file from your directory.
Once a file is deleted it is gone forever, so
be certain you really want to delete a file before
you perform a delete command. If you are going
to delete several files simultaneously using a
wildcard character (for example, "rm *.bak")
the system will prompt you to confirm your
intentions (by typing in "y" for yes).
.sp
.I cat
.PP
"cat" (for concatenate) allows you to concatenate the contents
of specified files. For example, the following command will
concatenate the contents of "junka" and "junkb" and display it on the
screen:
.sp
.ti 20
% cat junka junkb
.sp
The result of a concatenation can be placed in another file by
using the redirection convention (i.e.cat junka junkb > junkc).
.PP
The cat command can also be used to display the contents of a
file. This is accomplished by the using the command with only one
argument - i.e. the name of the file to be displayed. For example,
the following command:
.sp
.ti 25
% cat junka
.sp
will cause the contents of "junka" to appear on the screen in a
rapidly scrolling-up motion.
.sp
.I br
.PP
"br" (for browse) allows you to view a specified file on your terminal
page by page. For example the command:
.sp
.ti 25
% br junk
.sp
would type the file junk on your screen with a
pause after each page of the file. The "br" command
knows what type of terminal is being used and will cause
special characters to be displayed in a way suited to the
terminal type. For example, understriking will be displayed
as reverse video on CRTs but as underlining on the line printer.
.sp
.ne 10
The "br" command can also be used as a pipe.
For example the command:
.sp
.ti 23
nroff junk|br
.sp
will cause the file "junk" to be formatted (i.e."nroffed")
and then displayed on your CRT screen on a page by page basis.
.sp
.I diff
and
.I cmp
.sp
The commands "cmp" and "diff" are two different ways of doing
essentially the same thing - locating differing lines in two
files that are being compared.
.sp
With the cmp command the system
displays the position number of the character and the line number
at which the difference occurred. The general form of the command is
.sp
.ti 18
% cmp <file1> <file2>
.sp
If the two files are identical no response is made.
.sp
The diff commands works in a somewhat different fashion.
This command, which has the same format as the cmp command,
displays the differing lines in the two files as well as their
locations. The response the command generates looks as follows:
.sp
.in 10
2c2
.br
< in which diff works.
.br
---
.br
> in which diff tends to work.
.in -10
.sp
(Note: in the line identifier construct the number(s) to the left
of the c indicates the differing line(s) in file1 and the number(s) to the
right of the c indicates the differing line(s) in file2.
The 2c2 resembles a Text Editor command that could be used to
regenerate file2 from file1.)
.sp

.bp
.af % 1
.ds F1 - % -
.ds Fd
.ds H1  ~
.ds H1 TUG
.P1 "Using UNIX"
.P2 "Using a Terminal"
.P3 "Putting the Terminal On-line"
.PP
Two types of CRT terminal predominate within the Agency's UNIX
community - the Delta Data 7000 and the Superbee. The procedure
for going on-line vary between the two. For the Superbee only
two steps are involved. One must turn on the power (the power
switch is located in the rear of the terminal, in the lower left
hand corner) and then hit the on-line key (top row, left side of
the keyboard). If the login prompt doesn't appear hit the BREAK key
a couple of times. For the Delta Data one must complete a series
of "down-line loading" procedures. A set of instructions illustrating the
process should be posted somewhere in the vicinity of the terminal.
.P3 "Logging in"
.PP
Once the terminal has been put on-line the login prompt will appear on the
screen together with the computer name and the version of the UNIX operating
system being run. Respond to the login prompt by typing your login name
followed by a carriage return. When you have done this the system will issue a
password prompt, to which you respond by typing in your password.
Your password will not be echoed on the screen for obvious reasons.
Once your login name and password are verified the system will display
the message of the day, the quote of the day and the like. Following
this the prompt character(%) will be displayed, indicating that you are
talking to the UNIX command interpreter (i.e. the shell).
.PP
If you receive a response such as "no directory," notify operations.
If you receive a response of "Permission denied," it means that you are
not authorized to use the particular terminal you're trying to log onto.
Try another terminal or ask operations or the system administrator to
give you access rights to the terminal, (assuming that it's a terminal you
should be allowed to use).
.PP
If the system responds to your attempt to log in with the message
"login incorrect:" you have made an error in typing in either your login
name or your password. Repeat the login procedure in order to gain
access to the system.
.br
.ne 20
.P3 "Logging Out"
.PP
The UNIX shell will take commands to manipulate files and invoke
languages. When you are all through running type "logout" or
"detach." (Detach, in contrast to logout, will not kill jobs
that are running "in background". More will be said about background
jobs at a later point in the manual.)
.P3 "Useful Information About Your Terminal"
.PP
There are two keys on the terminal which act like shift keys on a
typewriter. These are the shift key and the control key. To type a
"control letter" hold down the control key and press the letter key.
Control functions serve a variety of purposes on the system. For
.tr #^
example CTRL-T, which is often written #t, will echo "System
.tr ##
Status: Normal" if the system hasn't crashed on you. The shift key
works the same way on a terminal as it does on the typewriter. It is the
means of typing upper case letters and many special characters.
When using the shift key one should be careful to avoid hitting
the shift lock. This is a common problem on both the Superbee and
Delta Data terminals.
.P3 "Several Common Problems"
.PP
If your terminal is typing only in upper case letters check to
see that you have not accidently hit the shift lock key. If the
shift lock is not on and you are locked into upper case type
in "stty -lcase" (it will appear on the screen in upper case) and
you will be returned to lower case.  Alternately, you can free
yourself from upper case by logging out and then
logging back in.
.PP
There are a whole range of terminal problems which can crop up,
most of which are not amenable to a quick and easy solution. If you
find that your terminal is acting up you should notify your
system administrator. He will be able to notify the person(s) responsible
for terminal maintenance.
.PP
If you suddenly find that the characters you are typing are not
appearing on the screen it may be that the system has crashed.
You can check on this by firing off a quick call to your system
administrator or by hitting CTRL-T. If the system has not
crashed CTRL-T will return the following message: "System status: Normal."
Obviously, if the system has crashed you'll get no response from
CTRL-T. As you can well imagine the latter method of checking for
a system crash is considerably easier on your system administrator's
peace of mind. If the system has crashed, hit the clear key to
clear the screen ( a good idea from the security point of view) and
wait for the login display to reappear. (Note: the clear function key
is available only on Superbee terminals.) If you were using the RAND Editor
at the time of the crash there is a way to (sometimes) recover the work
done during the editing session. The following sequence of commands:
.sp
.ti 15
% cp /tmp/rettyx.login-name retty
.br
.ti 15
%editor -retty
.sp
will usually cause your editing session to be reproduced. (You must
substitute your terminal number for "x" in the above command and your
own login name for "login-name.")
.P2 "The Printer Terminals"
.PP
Three types of printer terminal are used with Agency computers
running UNIX: the Anderson Jacobson 832, the Qualterm X100 and
the GSI 300Q. These terminals are used to obtain hard copy which
is of a better quality than that produced by a line printer.
They produce print which is equivalent to typewriter print and
can use either standard line printer paper or typing paper.
.PP
When using the printer terminals it is essential that modes
and speeds be properly set using the "stty" command. Thus,
if moving from a CRT (which runs at 2400 baud) to an AJ 832
(which runs at 300 baud) a command of the following sort should be
entered on the CRT:
.sp
.ti 23
stty 300 -tabs
.sp
The "-tabs" argument sets UNIX for software tabs, which will
get you around the problem of the printer terminal not having
its hardware tabs set. Alternately, you could set the hardware
tabs on the printer by typing the following command once you
have moved to the printer terminal:
.sp
.ti 28
tabs
.sp
Now we are prepared to consider what you do once
the move has been made from the CRT to the printer terminal.
.PP
The three types of printer terminal work in essentially the
same way and the operating procedures are not overly
complicated. Assuming that you are working with an
Anderson Jacobson that is on-line and has its switches properly set, you can
print out a formatted document by entering a command of the
following sort:
.sp
.ti 18
nroff -T832 -s filename
.sp
This should be typed on paper that is not part of the document
and <carriage return> must not be entered until new paper has been inserted.
The "-T" argument indicates to the text processor (i.e. nroff) the
type of printer terminal that is being used so that it can take advantage
of its features. The following are the "-T" arguments for the three types
of printer terminal:
.sp
.ne 5
.in 15
-T832.....Anderson Jacobson 832
.br
-T100.....Qualterm X100
.br
-T3000....GSI 3000
.in -15
.sp
The "-s" argument will cause the printer to stop at
the end of each page, allowing a new sheet of paper to be inserted. Once new
paper has been entered you must hit <carriage return> again to begin
printing.
.PP
For more information on using the printer terminals it is
suggested that the user consult the following documentation:
.sp
.L3
"The Care and Feeding of the AJ," (a "PIB in the making").
.P2 "Special Characters"
.P3 "Editing Typing Mistakes"
.PP
If an error is made while typing in a line, there are several
control characters designed to help you undo the error:
.sp
.in 5
.ti -5
#----Erases the last character typed. Successive uses of this
control character will erase characters back to, but jot beyond the begining
of the hine*
.sp
.ti -5
@(---Erases all characters that precede it jn the line. To reiind
you the termhnah wihl hove the cursor tj a new hije*
*hn )5
.P3 "Ckntrohling Typeout"
.PP
There are two keys which serve the function of controlling
the flow of output to the CRT screej:
.sp
.in 10
.ti )10
CTRH-S----This key suspends output. It is particularly useful
for giving the user time to read what is being displayed on
a CRT screen. If the user does not hit CTRL-S output will
quickly pass off the screen in a continuous "scrolling up"
action. (The scrolling up continues until the end of the output is
reached.)
.sp
.ti -10
CTRL-Q----This key will cause the flow of output to be
continued from the point at which it was interrupted.
.in -10
.br
.ne 10
.P3 "Interrupting a Command in Progress"
.PP
At certain times it may be useful to terminate a command before
it acccomplishes what it was intended to accomplish. There
are three ways of terminating a command that apply to different
sets of "terminating circumstances:"
.sp
.ne 10
.in 10
.ti -10
RUB OUT---This key terminates the current command, and returns
.br
.ti -10
DELETE~~~~control to the shell, with a few exceptions. For example, if a
command is given to type a file, and after the first few lines you don't
want to see the rest, then type RUB OUT and you will receive another
prompt sign(%). In some languages, such as BASIC and PPL, RUB OUT
is used to interrupt infinite loops, and return to the language's
command level.
.sp
.ti -10
CTRL-\\----This key has the same effect as RUB OUT, but it also produces
a core dump file of the running command. This key should only be
used when you are debugging a program of your own and want to save
a core image. The resultant CORE file can be examined using "db,"
"adb" or "cdb" (see the UNIX manual and the write-up in section 4 of
this document).
.sp
.ti -10
PANIC-----The PANIC key is the most general means of terminating
a command in progress - it works under all circumstances. It is,
however, the most drastic way of terminating a command. Rather than
returning control to the command level it logs the user off the system.
Thus, PANIC is best reserved for situations in which nothing else will
work. (Note: to make PANIC work it may be necessary to hit the key
twice in quick succession.)
.in -10
.P3 "Terminal End of File"
.PP
In the interactive mode an end of file is marked in the following
fashion:
.sp
.in 10
.ti -10
CTRL-D----This key when typed at the begining of a line has the effect
of producing an end-of-file from the terminal. Programs which process
their input until they reach end-of-file can be typed into directly
by ending input with CTRL-D.
.in -10
.P2 "Upper-Lower Case"
.PP
The UNIX system was designed for full character terminals
(i.e., terminals with upper and lower case letters), on which
it is much easier to type commands in lower case.
To enable terminals which type only upper case to use
the system, one of the terminal modes which is supported
by UNIX causes all upper case characters to be
translated into lower case.
This means that the upper case characters you
see on your terminal are actually being seen by the
programs on the system as lower case.
For this reason, all of the documentation on UNIX
uses lower case when talking about typing in and receiving output from
the system.
.PP
It is possible to enter an upper case character to the
system by preceding it with a "\\".  This allows
a user to type full upper and lower case from a terminal
which has only capital letters.
.sp
.ne 10
.P2 "Files"
.PP
The concepts of files and directories are familiar to
anyone who has used time-sharing before, but for anyone
who is used to batch computing which uses cards for input, these
terms will be new.  UNIX users are assigned a
particular directory which contains their files.
A file is simply text or data with
a unique name within a particular directory.
.PP
Imagine the file structure as being a library.
Users have their own private stacks (directories) in
which they keep their books (files).  Each book in a users
stack must have a different name, but many users may
have the same book in their own stacks.
Any book in the library can be described by its stack
and its name, and in the UNIX system, any file can
be specified by its directory and its name.
.PP
Although the name of a stack and book is sufficient
to identify a unique book in the library, it is necessary
to know where the stack is in order to
actually find the book.  The
set of instructions, or map, leading
to a particular file (stack and book) is called its "pathname".
If users wish to reference a file which is not in
their directory, they must specify the path from the beginning
of the file structure to the individual file.
At each directory along the way, the proper path
to the file must be chosen from among the subdirectories and other
files within that directory.
For example, the path "/usr/games/hangman" directs
the system to start at the beginning of the file
structure (the leading "/"), go to the directory
named "usr", within "usr" find the directory named
"games", and therein find the file named "hangman".
.PP
Files have several attributes of interest as well as their
names.  Each file has a length which is measured in bytes.
Each byte can contain a single character, or 8 binary bits of
information.
The physical device which contains the files (called a "disk") organizes
information in chunks of 512 bytes, so files are
quantified in blocks of 512 characters.  Thus, a file
less than 512 characters long takes up one block, 513 up
to 1024 characters long takes up two blocks, and so on.
For files of more than eight blocks, there is an
extra block of overhead for every 256 blocks of the file.
See the "PWB/UNIX User's Manual"
for more detail.
.PP
For purposes of accounting, file sizes
are measured in blocks.
The total number of blocks your files and directory
are using can be found using the "du" command described
below in the section on UNIX information commands (sec. 3.7.1).
.PP
Another attribute of a file is its "protection".  This
is a facility which allows you to give other users
either free access to your file, or  access limited as you
desire.  See the "chmod" command described below (sec. 3.7.3)
for more information on file protections.
.P2 "File Names and Wild Cards"
.PP
File names on UNIX are up to 14 characters long, and composed of
any characters.  Do NOT use "." as the first character
of a file-name, since the system utilities for cataloging
files will not list such files.  In addition, the file-names
"." and ".." are reserved for system use.  (See the PWB/UNIX
User's Manual.)
.PP
In naming files certain conventions are usually
observed so that the nature of the file can readily be
discerned from its name.  The convention followed
on UNIX is to end the file-name with a period followed
by a few characters which indicate the type of the file.
These suffixes are called extensions, and a few common ones on UNIX are:
.sp
.in 10
filename.ppl	a source file for PPl
.br
filename.f4p	a source file for FORTRAN
.br
filename.bas	a source file for BASIC
.br
filename.dat	a data file for FORTRAN or BASIC
.br
filename.bak	a source file backup
.br
filename.c	a source file for the C language
.in 0
.PP
It is often inconvenient to type complete file
names, particularly if the same action is to be
performed on many files.
To simplify the specification
of filenames, certain "wild cards" are allowed which can
be used in order to make a single string of characters refer
to many files
when they are typed to the shell.  These "wild card" characters
are:
.sp
.in 10
.ti -10
*~~~~~~~~~"*" matches any string of characters.  Thus "*" alone
is all file-names in your directory.  "foo*" would be all file-names
beginning with "foo", and so on.
.sp
.ti -10
?~~~~~~~~~"?" matches any single character.  Thus "?" alone
is all file-names one character long.  "foo?" would be all file-names
four characters long beginning with "foo".
.sp
.ti -10
[~~~~~~~~~"[xyz]" matches just the characters "x", "y" and "z".
"[a-g]" matches any single character from  "a" through
"g".  Thus "foo[abc]" describes the file-names "fooa", "foob",
and "fooc", and is equivalent to "foo[a-c]".
.in 0
.PP
Some examples of wild cards using the rm command are:
.sp
.in 20
.ti -20
rm *.ppl~~~~~~~~~~~~this deletes all files with the extension "ppl"
e.g., file1.ppl, text.ppl, a.ppl
.sp
.ti -20
rm *~~~~~~~~~~~~~~~~this deletes all files (so be careful)
.sp
.ti -20
rm file?.bas~~~~~~~~this deletes file0.bas, file1.bas,
filez.bas, etc.,
but does not delete file1a.bas or file.bas
.sp
.ti -20
rm file[a-s].bas~~~~this deletes filea.bas, filec.bas, etc.,
but does not delete file1.bas or filet.bas
.sp
.in 0
.in 10
.ti -10
Note: (1) It doesn't make sense to use wild cards for
destination files.  Wild cards are useful in referring
to files, but they can never be used to make new 
files.
.sp
.ti -10
~~~~~~(2)~When you try to delete several files with the "*"
wildcard you will be presented with a request for confirmation.
This takes the following form:
.sp
.ti 24
glob: [Confirm]
.sp
If you wish to confirm, type y and hit the carriage return; if not, type n
and hit return. The action is carried out only after confirmation
is received.
.in 0










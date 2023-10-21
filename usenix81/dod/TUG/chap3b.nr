.ds F1 - % -
.ds Fd
.ds H1  ~
.ds H1 TUG
.I chmod
.sp
"chmod" (for change mode) changes the protection of a file.
It takes an octal protection mode (see below) and a list of
filenames or directories and sets the desired privileges on
them. There are three categories of protection
mode applying to the three categories of usage - owner, group
and all users. The octal code for setting privileges within the three
categories is as follows:
.sp
.in 10
.I
Mode~~~~~~~~~~Meaning
.R
.sp
400~~~~~~~~~~Readable by owner
.br
200~~~~~~~~~~Writable by owner
.br
100~~~~~~~~~~Executable by owner
.br
~40~~~~~~~~~~Readable by owner's group
.br
~20~~~~~~~~~~Writable by owner's group
.br
~10~~~~~~~~~~Executable by owner's group
.br
~~4~~~~~~~~~~Readable by all users
.br
~~2~~~~~~~~~~Writable by all users
.br
~~1~~~~~~~~~~Executable by all users
.in -10
.sp
The three octal digits are used to set user privileges for all
categories of user. For example, the default protection mode for
non-executable files is 640. This assigns read and write privileges
to the owner, read privileges to the owner's group and no
privileges to the general user. The corresponding default protection
mode for executable files is 750.
.sp
.I ln
.PP
"ln" (for link) is used to create a new filename for
an existing file.  That is, after the ln command has
been performed, the file will have two names.
Going back to the library analogy used above,
the ln command would be analogous
to writing a second name on a book.  Deleting a file
is analagous to erasing the
name from the binder of a book in the library, and if
all of the names have been erased, then the book (file) is
discarded. The format of the ln command is quite straightforward:
.sp
.ti 13
% ln <old filename> <new filename>
.sp
For example the command "ln circus mycircus" would allow you
to reference the file circus using the filename mycircus.
ln is most commonly used to reduce typing when
referencing another users file.  If I was an avid player
of the game "hangman", I might tire of typing the
name "/usr/games/hangman" everytime I wanted to
run the program, so I might use the ln command to link the file
with the name "hang" in my directory.
.sp
.I opr
.PP
"opr" queues files to be printed on
your system's line printer(s). The general form of
the opr command is as follows:
.sp
opr [-ncopies] [-iindentation] [-Cclassification] [-crb]
[-pprinter] [filename(s).....]
.sp
.ne 8
As an example of how the opr command works consider the
following:
.sp
.ti 10
opr -3 -i10 -Csupersecret -p1  job1 job2
.sp
This command will send to the printer three copies of the fides job1 and job2
with an indent of 10, a classification of supersecret,
and with the job being sent to remote printer 1.
.sp
.ne 10
.in 7
.ti -7
Note:~~There are three other arguments to the opr command (-c -r -b)
which are of a somewhat specialized nature. For an explanation of their
use see OPR, PRINT(I) in the "PWB/UNIX User's Manual."
.in -7
.sp
.I lpq
.PP
"lpq" (for line printer queue) causes a list of jobs in the line printer
queue to be displayed on the screen. The list will include the job number(a star
following the job number indicates that it is in the process of
being printed), the login name of the person who submitted the job,
the number of bytes in the job and the identifier of the printer on
which the job will be printed.  The order of the jobs on the queue
list is the order in which they will be printed. The lpq
command can be used with two arguments by the general user. -r
is used with lpq for removing jobs from the line printer queue. -r
used with a job number will cause that job to be removed. If the job
number is omitted all the user's own jobs in the queue will be removed.
If a job is already being printed -r will have no effect upon it.
To abort a job that is in the process of being printed it is
necessary to use lpq with the -k argument. (Use -k for printer0
and -k1 for printer1.) Both -r and -x can be
used by the general user only to terminate jobs which he himself owns.
.sp
.I chdir
or
.I cd
.PP
"chdir" or "cd" (for change directory) changes the "working directory" of the
user. In the library analogy (II.5) chdir would be
analogous to a user walking over to another
stack and sitting down.  After the move, any book (file)
names used would refer to the new stack (directory) the
user is associated with.
Chdir with no argument will change the directory to $s (i.e. your
login directory or whatever directory you have defined as you default
directory). If you want to move to a directory that is
contained within or was created within the current directory
you can use chdir with the name of the directory only.
Otherwise you must specify the full pathname of the directory
to which you are moving. For example, if the current
directory is /usr/tml and tug was created within this directory
you can move to tug by simply typing "chdir tug". If, on the
other hand you want to move from /usr/tml/tug to cprograms,
a directory that was not created within /usr/tml/tug, you
must type "chdir /usr/tml/cprograms".
.br
.ne 10
.sp
.I mkdir
.PP
This is a UNIX command which makes a subdirectory in the
current working directory. For example,
.sp
.ti 22
mkdir pplfiles
.sp
makes a directory called pplfiles.
.sp
.ne 10
.I rmdir
.PP
When a directory is empty it may be deleted by this
UNIX command.  For example,
.sp
.ti 22
rmdir pplfiles
.sp
deletes the directory "pplfiles".
.P3 Miscellaneous
.sp
.I
qotd
.R
.PP
"qotd" (for quote of the day) types some timely quotation from
one of a variety of questionable sources. You will get
a quote of the day, without even asking, when you log in for
the first time each day. If one inane saying a day is not enough
for you type "cookie" to get as many as your heart desires.
Often, you will get an "on this day in history" type saying
along with the quote of the day. You can specifically request this
by typing "today".
.sp
.I news
.PP
"news" provides the user with information concerning the
latest developments on the system. The command without any
argument will cause a list of topics on which news
is available to be displayed. The command with a topic
as an argument will cause the news on that topic to be
presented.
.P2 "Inter-User Communications"
.sp
.I write
.PP
"write" couples two terminals together so that the
users can type to each other.  For example, if you want to ask
a question of a fellow user and you know that he is logged on,
you can contact him using the write command. The write
program is quite easy to use. The command "write" with the
login name of the person you are trying to contact will
establish contact. The person at the other end of the line
will receive the following message:
.sp
.ti 12
Message from tml on tty p at ELROND
.br
.ti 15
To respond, type write tml
.sp
The person sending the message should wait for some response
from the person he's trying to contact before keying in his message.
The person who has been contacted must make a connection by
typing in "write". He can then acknowledge the attempt to
contact him by either typing in a response
which will immediately appear on the the other user's screen, or
by hitting CTRL-Y. CTRL-Y will cause the message "Go ahead <login name>"
to be displayed on both users' screens. To terminate a write
session both users must type CTRL-D, which signals an end of
transmission to the other user by causing the message "<login name>
has quit" to appear on the screen. The write program, with a slight
variation, can be used to contact other users across the net. The
following command is used to establish contact with a user on a
different system:
.sp
.ti 10
write <login name> at <host computer name>
.sp
If you want to contact any one of several users with the
telnet version of the write program type the user ids separated by blanks
and enclose the entire  string  in  double  quotes.
Write  will search for the users in the order typed and connect
with the first one available.
.sp
.I mesg
.PP
The mesg command is used to deny other users the privilege of
sending you messages via the write command. The general form of
the mesg command is as follows:
.sp
.ti 21
mesg [ n ] [ y ]
.sp
Mesg with argument n forbids messages via write by  revoking
non-user  write  permission  on the user's typewriter.  Mesg
with argument y reinstates permission.  All by itself,  mesg
reverses  the current permission.  In all cases the previous
state is reported.
.sp
.I sndmsg
.PP
The sndmsg command is used for sending mail messages to
other users. After typing in sndmsg the user will be prompted
for the recipient(s) of the message, its subject  and the
message text. The following is a sample of sndmsg prompts
and user responses:
.sp
.ne 15
.nf
_______________________________________________________________
|% sndmsg                                                      |
|To: tml                                                       |
|cc: ldn                                                       |
|Subject: nothing                                              |
|Type message, end with <EOT> (ctrl-D)                         |
|This is some sample text used for purposes of illustration.   |
|______________________________________________________________|
.fi
.sp
After the message text has been typed in the system prompts the
user with a list of message processing alternatives. This prompt
looks as follows:
.sp
queue, send, editor (or re), ed, input, file, justify, display, or ?: queue
.sp
The queue and send commands will cause the message to be
dispatched without any intervening processing. For local mail
send and queue have the same effect. There is a difference, however,
for mail being sent over the net. The send command will not return you
to the shell level until the net connection has been made and all
messages dispatched. The queue command queues the message for net
mailing and immediately returns you to the shell level. There is
one other way in which the use of sndmsg over the net differs from its
usage on a local basis.
For network mail the recipient of the message must be identified by his
host computer's name as well as by his login name. The user should
respond to the "To:" prompt as follows for network mail:
.sp
.ti 21
To: tml@elrond
.sp
For a consideration of the other processing options it is suggested that the
user consult the SNDMSG(I) entry in the "PWB/UNIX User's Manual".
.sp
.I msg
.PP
The msg command is used to read mail messages that have been
sent to you via the sndmsg command. A user is notified
that he has mail in two ways. If you are logged in and not using the
RAND Editor a message of the following type will appear on your
CRT screen:
.sp
.nf
______________________________________________________________
|                                                             |
|              <bell> [You have mail] <bell>                  |
|                                                             |
|_____________________________________________________________|
.fi
.sp
.ne 5
If you were not logged on or were using the RAND Editor when
a message was sent, you will recieve the following message
the next time that you log on:
.sp
.ti 25
You have mail
.sp
Once informed that you have mail you can go about reading it
by first typing "msg". This will cause the queue of messages in your mail
file to be displayed. The display will resemble the following:
.sp
.ne 15
.nf
______________________________________________________________
|% msg                                                        |
|Readmail -- version 2.3 of 3/2/79                            |
|  1    160 char  1 Feb 1980 elrond at ELROND"password change"|
|  2   7644 char 19 Feb 1980 Dlm at ELROND   [MP-220 file]    |
|  3    118 char 25 Feb 1980 To: tml         nothing          |
|  4    142 char 25 Feb 1980 To: tml         nothing          |
|  5    233 char 25 Feb 1980 To: tml at elro Test             |
|  6    233 char 25 Feb 1980 To: tml at elro test             |
|6 messages                                                   |
|For help type ?                                              |
|-> ?                                                         |
|_____________________________________________________________|
.fi
.sp
Typing a question mark after the arrow for help, as is shown above,
will cause a list of commands that can be used with the msg
program to be displayed. The following represents a selection of
these commands:
.sp
.nf
______________________________________________________________
|Commands are:                                                |
|d[elete] <message no>...                                     |
|e[xit and overwrite current mail file to reflect deletions]  |
|h[eaders of messages:] <message no>...                       |
|n[ext-message-is:]                                           |
|s[ave mail and exit]                                         |
|t[ype] <message no>...                                       |
|q[uit]                                                       |
|_____________________________________________________________|
.sp
.fi
The commands are more or less self-explanatory. "delete" will cause the
specified message to be deleted from the mail file. "type" will cause the
the message to be displayed on the screen. "quit" will cause
you to exit the msg program without any changes being made hn the mail
file. "exit" will cause you to exit the prograi with the iahh fhle behng
overwrittej tj rebhect ajy chajger* Save( jj the jther hajd( shhh cause
the prkbrah tj be exhted shth the cjjtejtr jb the hahh bhhe behjb
rehjred brjh the hahh bhhe ajd trajsferred tj a bhhe cahhed "sarehahh"*
Fjr a dercrhpthjj jb the jther bjhhajdr that are used shth the hsb
prjbrah see the ejtrx HRC(H( hj the "UJHX Prjbrahier"s Hajuah*"
*rp
*je 5
*hj 2
*th (2
Jjte: It is only necessary to type the first letter of
each of the commands used with "msg".
.in -6
.br
.ne 15
.P2 Languages
.PP
UNIX supports a number of general purpose programming languages
as well as several "language-like" text editors and macro
packages. The languages that are supported by UNIX will be
discussed in more detail in the section 4. At this point
it will suffice to merely list the languages:
.sp
.in 10
fortran~~~~~~~~~~ppl~~~~~~~~~~~~~~~basic
.br
f4p~~~~~~~~~~~~~~as~~~~~~~~~~~~~~~~pascal
.br
fc~~~~~~~~~~~~~~~cc~~~~~~~~~~~~~~~~rc
.br
lisp
.in -10
.P2 "Logout and Login"
.sp
.I Logout
.PP
There are several ways of exiting from the shell (at its topmost
level - i.e. when you are working with a prompt). One way is to simply
type in "logout". This method of exiting can cause problems in some
circumstances because it will cause all background jobs to be killed
(See the note on background jobs below.) "detach" is an alternative
method of exiting the shell that leaves background jobs unaffected.
Finally, exiting the shell can be accomplished by keying in a CTRL-d.
CTRL-d, like logout, will cause background jobs to be killed.
.sp
.in 7
.ti -7
Note:  A process can be caused to run in background by following the
command which initiates the prgcess with the character "&".
The process is said to run in background because the shell doesn't
wait for the process to be completed before accepting more
commands but returns to a "state of readiness"  while the process
is still running.
.sp
Detaching or logging out will "blitz" your terminal. Any output
that a background job might want to write to the terminal
will be lost. This is to prevent output from one user's background
job from annoying the user who succeeds him at the terminal. Also,
it should be noted that network connections are terminated by a
logout.
.in -7
.sp
.I Login
.PP
The login command is used to change from one user account
to another. Using this command
.br
.ne 8
simply involves typing in "login". This will cause the system to
prompt you for your lochj jahe ajd parsrjrd* Hhke "detabh(" hjchj
shhh jjt jhhh ajx jjbs that are rujjhjc hj babjbrjujd*
*br
*je 15
*P2 "Perhpherah Derhber"
*PP
UJHH jbbers a rhde rarheth jb prjbrahr bjr urhjb
perhpherah derhber hj bhhe tbajrberr* Bjth UJAH(tj(UJAH
ajd bertahj UJHH(tj(JJJ(UJHH trajrberr baj be abbjhhjdated*
Sererah auhhhharx pbjbrahb are arahhabhe tj ahhjr ehbhurhre
ure jb a perhpherah derhbe*
*rp
*P2 "Arrhbjhjb ajd De(arrhbjhjb Perhpherah Derhber"
*PP
Bebjre urhjb a perhpherah derthat it
be "assigned" for your use.
.sp
.I Assign(ass:)
.PP
Assign(ass) allows a user to obtain a  peripheral  device  dedicated
for  his exclusive use.  The argument(s) specify each
peripheral which the user wishes to reserve. The command used
without any arguments will display a table that lists the available
devices by ID and indicates the user (if any) to which they are
currently dedicated. The table will resemble the following:
.sp
.ne 15
.nf
_______________________________________________________________
|                 D E V I C E     T A B L E                    |
|      Device  Owner                        Comments           |
|      ------  ----- ------------------------------------------|
|1.    tap0          /*                                      */|
|2.    mt1           /*                                      */|
|3.    ht0     tml   /*                                      */|
|4.    rk0     mtl   /*                                      */|
|5.    cat     dlm   /*   Graphic Systems C/A/T typesetter   */|
|6.    nmt0          /*   no rewind mt0                      */|
|7.    nht0          /*   no rewind ht0                      */|
|______________________________________________________________|
.fi
.sp
The device IDs signify the following: tap refers to DECtape; mt indicates
mag tape; ht refers to high density mag tape(1600 bpi); rk indicates
cartridge disk; cat specifies the phototypsetter; and nmt and nht
refer to non-rewindable mag tape and high density mag tape respectively.
.sp
.ne 10
.I Deassign(deas)
.PP
The deassign(deas) command is used to release peripheral devices
that have been assigned to a particular user via the assign command.
The command used with a device ID as an argument will cause that
particular device only to be released (provided, of course, that the
device is assigned to the user). The command used without ajy arguments
will cause all the devices assigned to the user to be released. The
commands used for exiting the shell (i.e.logout and CTRL-d)
will also cause all the devices assigned to the user to be released.
.ne 10
.P3 "Common Programs for Peripheral Use"
.sp
.I tp
.PP
tp is used to copy UNIX files from disk to tape (DECtape or mag tape)
and vice-versa. The general form of the tp command is as follows:
.sp
.ti 23
tp [key] [name...]
.sp
The key consists of any of several functions and function
modifiers (indicated by letters and digits) that control the
specific peripheral device. "Name" in the above
command refers to the file or directory names which are to be
operated upon by tp. What follows is a list of tp commands
for popular peripheral device applications:
.sp
.in 15
.ti -15
tp tvm0        This command will cause a copy of a tape's
table of contents to be sent to the standard output. Before
transferring files from tape to disk it's good to know what's
on the tape so that you can be selective with regard to what will
be transferred.
.sp
.ti -15
tp crvm0       This command writes files on mag tape. Since no
file name is specified the current directory(.) is assumed by
default. All files that are in the current directory and
all files that are in any directories in the current directory
will be written on the tape.
.sp
.ti -15
tp crvm0 hold  This command wil cause the file or directory
named "hold" to be written on mag tape.
.sp
.ti -15
tp xvm0        This command will cause all files that are
on a mag tape to be written on disk, creating any
necessary files or directories in the process.
.sp
.in -15
.ne 5
.in 6
.ti -6
Note: File and directory names used with the tp command must
be specified exactly. tp will not recognize as identical filenames
which are logically the same in UNIX (i.e. "./abc" and
"/usr/dmr/abc").
.in -7
.sp
.ne 10
.I tar
.PP
tar is a Seventh Edition UNIX peripheral program which
closely resembles tp. It should be noted, however, that tar offers
a number of advantages over tp. In a general vein, tar works effectively
with long pathnames and a large number of files, both of
which can cause problems for tp. Also, tar is more compatible
with the full range of UNIX systems. In a more specific vein,
tar offers a number of function modifiers that are not
available with tp. For a full listing of functions and
modifiers see the tar(I) listing in "UNIX Programmer's Manual,
Seventh Edition."
.sp
.I filex
.PP
The filex program closely resembles tp. The major difference
between the two is that filex can read and write DOS-11
formatted DECtapes and magtapes whereas tp works in an
exclusively UNIX environment. The general format of the
filex command is as follows:
.sp
.ti 20
filex key [file....]
.sp
The keys used with the command are for the most part the same as
those used with tp. (For a full listing of the requests and
modifiers constituting the key see the Filex(VI) listing in
the "PWB/UNIX User's Manual.") As an example of the filex command
consider the following:
.sp
.ti 20
filex rvm0 hold.txt
.sp
This command will cause the UNIX-formatted file "hold.txt" to be
written on magtape in DOS-11 format.
Note that the DOS file naming conventions are used.
.sp
.I rtpip
.PP
The rtpip program closely resembles filex. The essential difference
between the two is that rtpip can read and write DEC RT-11
formatted DECtapes and RK05 cartridges, whereas, as noted above, filex works
with DEC DOS-11 tapes. The general format of the
rtpip command is as follows:
.sp
.ti 20
rtpip key [file....]
.sp
The key is composed of two characters: one specifies the
function to be performed and the other identifies the RT-11
device to be accessed. The two most important
function key characters are r and x: r copies files
.I to
the RT-11 device and x copies them
.I from
the RT-11 device (currently DECtape or RK05 cartridge).
As an example of the rtpip command consider the
following:
.sp
.ti 22
rtpip r0 junk
.sp
This will cause the UNIX file junk to be copied to RT-11
device 0 (DECtape). As a result a copy of junk will be transferred from disk to
the file 'JUNK.~~~' which is on tape in RT-11 directory format. If no file
by the given name exits it will be created, otherwise an existing file
of that name will be over-written. For more information on the rtpip
command see the entry RTPIP(I) in the "PWB/UNIX User's Manual."
.sp
.I dd
.PP
dd is a program designed for copying and converting a file from
"foreign" (i.e. non-UNIX) format to UNIX format and vice-versa.
The general form of the dd command is as follows:
.sp
.ti 18
dd [option=value].....
.sp
The option-value combination refers to such things as the
input block size, filename and character set (to be converted).
(For a fuller description of the options and values see the
entry DD(I) in the "PWB/UNIX User's Manual".)
In its most popular application dd is used to copy tapes
originating on non-UNIX systems to UNIX format. For
example, suppose you wanted to convert to UNIX format a
tape that is in a popular IBM format - EBCDIC tape blocked
ten 80-byte card images per record. The command used to accomplish
this appears as follows:
.sp
.ti 3
dd   if=/dev/rmt0  of=x  ibs=800  cbs=80  conv=ascii,lcase
.sp
The first option-value combination specifies the input filename.
This is actually the descriptor of the physical device upon
which the tape to be converted is mounted. The second option-value
combination specifies the output filename. The file that is copied
from the tape will be called "x". The third combination indicates
the input block size in bytes. For our IBM tape a block size of
ten 80-byte card images constitutes an 800 byte input block size.
The fourth option-value combination specifies the conversion
buffer size. The conversion buffer is used when acsii or
ebcdic conversion is required. In the case being considered
80 ebcdic characters are read into the conversion buffer,
are converted to ASCII,
any trailing blanks are removed, a newline character is added(
and the hine is then sent to output. The process cknthnues
until the end nf file is reached. The final jption-value
combination hndicates the type of cjnvershoj that is tk take
phace betweej hnput and output* Hn respkjse tk the
ckmiand behng cjnsidered ebcdic character whhl be cjnverted
tk archi and upper case characters to lower case ones.
.PP
For more information on dd see the entry "DD(I)" in the
"PWB/UNIX User's Manual".
.sp
.ne 10
.I cpio
.PP
cpio is a program which copies files from disk to tape and
vice-versa. The command assumes three different forms
that vary according to the task to be accomplished:
.sp
.in 10
.ti -10
cpio -o~~~This command takes a list of pathnames from the
standard input and copies the files and associated status
information onto the specified output (usually magtape).
.sp
.ti -10
cpio -i~~~This command is the reverse of the previous one.
It will return to disk files that have been placed on an
output medium (again, usually magtape) using the cpio -o
command.
.sp
.ne 10
.ti -10
cpio -p~~~This command combines the functions of cpio -o
and cpio -i. It causes files to be copied from the
standard input to an output medium and back again in a
single operation. The files being moved are assigned to a
specified new directory, which directory is in turn
assigned to the current directory.
.in -10
.sp
The three commands, in particular cpio -i and cpio -p,
have a number of options. These options accomplish such
things as the creation of new directories where needed.
(For a full listing of the options see the entry cpio(I)
in the "PWB/UNIX User's Manual.") As an example of how
the commands work consider the following:
.sp
.in 15
ls | cpio -o > /dev/mt0
.sp 2
chdir olddir
.br
find . -print| cpio -pdl newdir
.in -15
.sp
The first of the above commands causes a listing of the current
directory's contents to be piped to cpio -o which in turn directs
the listed files to be written out onto magtape. In effect, the
contents of the current directory are archived. The second command is
somewhat more complicated. find recursively descends the directory
hierarchy from the pathname, in this case the pathname is that of the
the current directory - olddir. The listing of the files located by
find is piped to cpio which in turn causes the files to be copied out
to the standard output and then returned as the contents of the directory
"newdir." The options d and l tell the system to create new directories
where needed and to link files rather than copy them when possible.
The net effect of the command is that a copy of the current directory,
the directory "olddir", is made and entered in the new directory ("newdir").
.PP
For more information on cpio see the entry "CPIO(I)" in the "PWB/UNIX
User's Manual".
.sp
.I mtio
.PP
mtio is a program for analyzing, copying and extracting files
from "foreign" (i.e.non-UNIX) magtapes. In the analyze mode the
program tries to figure out what a foreign tape looks like -
i.e. the size of records, the number of records per file, the
size of files, etc. As an example of the analyze mode of the
program consider the following command:
.sp
.ti 22
mtio -tv /dev/rmt0
.sp
This will cause a "verbose"(-v) table of contents(-t)
of the 800 bpi tape on drive 0 to be directed to the standard output.
.br
.ne 5
.PP
In the copy mode the program will copy multiple
file tapes of varying record sizes from one device to another,
changing the density of the tape in the process if so desired.
As an example of the copy mode of the program consider the
following command:
.sp
.ti 17
mtio -c /dev/rmt1 /dev/rht0
.sp
This will cause the 1600 bpi tape on drive 0  to be copied(-c)
onto the 800 bpi tape on device 1.
.PP
Extraction is the final mode
of operation available with the mtio program. In the extract
mode, which presumes that you know how the foreign tape is
formatted, the program will transfer files from tape to disk
with an arbitrary character delimiter being inserted after each
record if so desired. As an example of the extract mode consider
the following:
.sp
.ti 18
mtio -x junk /dev/rmt0
.sp
This will cause all files on the tape mounted on drive 0 (800 bpi)
to be extracted onto disk, each as a separate file, filenames being
junkaa, junkab, etc.
For a full consideration of mtio see MTIO(I) in the "PWB/UNIX User's
Manual(Supplement)."
.P2 Games
.PP
UNIX offers several games.  However, they are not available for
use during working hours. To play a game type the following command:
.sp
.ti 25
% games
.sp
The games program will check to see if it is okay to play games
at the current time. If it is, a list of available games will be
displayed. To play the game of your choice you simply type in
the name of the game (with any necessary arguments) and the
it will begin.

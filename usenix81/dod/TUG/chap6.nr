.P1 "Appendix B: GENERAL UNIX COMMANDS"
.sp
.in 16
.ti -16
who~~~~~~~~~~~~~Returns info on who is logged on the system
and what terminal they are logged onto.
.sp
.ti -16
write abc~~~~~~~Allows you to send messages to user abc providing
his permissions will allow.  After typing write
abc, enter the messages you wish to send, ter-
minate with ctrl-d. User abc must be logged in.
.sp
.ti -16
pwd~~~~~~~~~~~~~Print Working Directory.  Lets you know where
in the UNIX file system the terminal is positioned.
.sp
.ti -16
ls -l~~~~~~~~~~~Creates a directory print of the files at the
current working directory.  The command "dir" does
the same.
.sp
.ti -16
chdir aa/bb/cc~~The command "chdir" is used with the proper argu-
ments to move within the UNIX filesystem from
directory to directory.
.sp
.ti -16
e filename~~~~~~Is used to call the RAND EDITOR to edit or create
the file name supplied.  Don't use re on any files
other than those within the "opr" account.
.sp
.ti -16
opr filename~~~~Will schedule a line printer listing of the file
name supplied.  Permissions are required for the
file.
.sp
.ti -16
cat filename~~~~Will cause the contents of the file supplied to
be printed or listed on the terminal or console.
.sp
.ti -16
msg/ms~~~~~~~~~~Is used to read or send mail.  See the UNIX manuals
for it's proper use.
.sp
.ti -16
ctrl-d~~~~~~~~~~This combination given to a prompt will cause the
terminal to be logged out.
.bp


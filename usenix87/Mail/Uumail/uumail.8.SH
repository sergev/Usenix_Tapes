case $CONFIG in
   '') . config.sh ;;
esac
$echo "Extracting uumail.8 (with variable substitutions)"
cat > uumail.8 <<!GROK!THIS!
.RL "Baylor College of Medicine"
.TH UUMAIL 8
.fi
.ad b
.SH NAME
uumail \- rewrite address & route mail using uucpmap database 
.br
uupath \- print the uucp path to a host
.SH SYNOPSIS
.B uumail [ \fIoptions\fR ] \fIaddress\fR
.br
.B uupath \fIhostname\fR
.SH DESCRIPTION
.B Uumail
is designed to be used as a mail delivery program to correctly
route mail over uucp connections.
.SS Standard Options
.IP "-f\fIaddress\fR" 16
The
.I -f
option sets the address of the sender of the mail. If this flag 
is not used, the sender will be established by usings environmental variables
(like 
.B LOGNAME
and
.B USER
) or using getlogin(3).
.IP "-om" 16
The
.I -om
option causes the mail to also be sent to the sender of the message.
.IP "-oc" 16
The 
.I  -oc
option causes
.B uucico 
to be started immediately after queuing the mail.
The default just queues the mail.
.IP "-N" 16
The -N option stops the use of
.B uuname
to determine neighboring hosts. Only the information in the database
will be used for routing information.
.IP "-h" 16
The -h option causes no From_ line to be added to the beginning of file.
This is useful when uumail is being used as a mailer for sendmail(8).
.SS Compile-time Configurable Options
!GROK!THIS!
if test X$debug = Xdefine
then
cat >>uumail.8 <<!ADD!THIS!
.IP "-d[1-6]" 16
The
.I -d 
option turns on the limited debugging facility built into the
mailer.  In debug mode, the mailer does not actually mail anything, but
tells you what it would do if it did do it. The level of debugging can
be set by following the 
.I -d
flag with a number between 1 and 6.
!ADD!THIS!
fi
cat >>uumail.8 <<!EVEN!MORE!
.IP "-g[A-Z]" 16
If your 
.B uux(1)
supports grading of transactions, the
.I -g 
option can be used to set the
grade of this mail. A grade of \fIC\fR is used by default.
!EVEN!MORE!
if test X$noalias = X
then
cat >>uumail.8 <<!ADD!ALIAS!
.IP "-n" 16
The
.I -n
option will prohibit the use of infomation provided by the system alias
file. Information in users'
.I .forward
files will still be utilized if present.
!ADD!ALIAS!
fi
cat >>uumail.8<<!MORE!STUFF!

.SS Arguments
.IP \fIhost!user\fR 16
where host is a system node name on the network and user is the login
name of the addressee.
.IP \fIuser@host.domain\fR
same as above with the addition of a domain specifier like
.B .ARPA, .GOV, .COM, .EDU
etc.
.SH FILES
!MORE!STUFF!
if test Xnoalias = X
then
cat >> uumail.8<<!ADD!MORE!ALIAS!
.IP "$aliases" 20'
System-wide alias file
.IP ".forward" 20
Individual user's mail forwarding file
!ADD!MORE!ALIAS!
fi
cat >>uumail.8 <<!OK!HERE!
.IP "$database{.pag|.dir}" 20
Path file produced by pathalias.
!OK!HERE!
if test X$log = Xdefine
then
cat >>uumail.8<<!ADD!MORE!LOG!
.IP "$logfile" 20
Log of uumail activity.
!ADD!MORE!LOG!
fi
cat >>uumail.8 <<!THE!END!
.SH "SEE ALSO"
pathalias(1), address(1), mh(1), sendmail(8), uux(1), uuname(1)
.br
.B RFC 822 "Standard for the Format of ARPA Internet Text Messages"
.br
.B RFC 976 "UUCP Mail Interchange Format Standard"

.SH AUTHORS
.br
Stan Barber, Baylor College of Medicine
.br
Getpath routine by John Donnelly, University of Illinois
.br
Gethostname routine by Peter Honeyman, Princeton
.br
Resolve routine written by the UUCP project
.br
Aliasing scheme borrowed from the MH mail handling system
!THE!END!


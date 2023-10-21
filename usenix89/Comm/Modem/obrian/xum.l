.tr ~ \ 
.TH XUM l local
.SH NAME
xum \- Xmodem file transfer for BSD 4.x Unix Systems (pronounced Zoom!)
.SH SYNOPSIS
.BR "xum
.PP
.SH DESCRIPTION
.I Xum
will transfer a text or binary file using the xmodem (a.k.a. modem7)
protocol. All commands to \fIXum\fR are entered into a menu which offers
these choices:

Selection of \fIbinary\fR or \fItext\fR file transfer.

Selection of \fIsend\fR or \fIreceive\fR from/to the host system.

\fIQuit\fI \fIXum\fR and return to \fIUnix\fR.

.SH NOTES
\fIXUM\fR needs to have a valid TERM entry in your environment to
display the menu.

\fIBinary\fR mode will transfer a file completely intact with no
characters added or deleted. \fIBinary\fR mode should be used for
uploading \fIWordstar\fR files. Then these files should be "cleaned up"
for use with \fIUnix\fR (see man entry for \fIwsstrip\fR).

\fIText\fR mode will, on upload, strip out all occurances of the
carrage return and control Z characters. On download, \fItext\fR mode
will add a carrage return before every linefeed (newline) character.

.pp

.SH AUTHOR
XUM was written by:
John Paul O'Brien, Coordinator of Technical Support
Nova University Computer Center
3301 College Ave.
Ft. Lauderdale, Fl. 33314
(305) 475-7633
{allegra, ucf-cs, usfvax2}!novavax!john
.PP
.SH BUGS AND CAVEATS
Receive mode is too slow. Needs to be fixed. Should also include
a command line mode to bypass menu constraint (ie. xum -st file).

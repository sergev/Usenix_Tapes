.tr ~ \ 
.TH WSSTRIP l local
.SH NAME
wsstrip \- text file stripper to remove "bad" control characters
.SH SYNOPSIS
.BR "wsstrip <infile >outfile
.PP
.SH DESCRIPTION
.I Wsstrip
strips out unwanted control characters from text files usually created
by uploading a \fIWordStar\fR document file via kermit binary transfer.
The control characters stripped out are:
.PP

	any byte with the 7th bit turned on (larger than 0x80)
	has that bit stripped off to normalize the character

.br
	any control character that is not \\n, \\t, bell
.br

.pp

.SH EXAMPLE
For this example we will assume the file to be processed is text.bad.
We will generate the file text.good.
.PP
.sp 1
We type:   wsstrip <text.bad >text.good
.sp 1
.pp
.SH AUTHORS
WSSTRIP was written by:
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
.PP
.SH BUGS AND CAVEATS
Options should be installed for different flavors of stripping.
Should also not use redirection (looks messy).

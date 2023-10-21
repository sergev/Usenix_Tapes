.TH NEWSFEEDLIST 1 "12 November 1986"
.SH NAME
newsfeedlist \- invoke news feeder repeatedly
.SH SYNOPSIS
.B newsfeedlist
[arguments]
.SH DESCRIPTION
.I newsfeedlist
takes a list of arguments
and calls the simultaneous multi-site news feeder with each one in turn.
.PP
Interspersed with the arguments may be any of the following options:
.TP 8
.B \-num
set the maximum batch size
.TP
.B \-
restore the maximum batch size to the default
.TP
.B \-hsite
add
.B site
to the list of
.I hold
sites
.TP
.B -h
delete the list of
.I hold
sites
.TP
.B //
any argument beginning with this string is ignored
.SH "SEE ALSO"
newsfeed(1)
.SH AUTHOR
Stephen J. Muir, Lancaster University, England, UK

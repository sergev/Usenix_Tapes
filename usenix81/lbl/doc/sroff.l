.TH SROFF UA1
.ad
.br
.SH NAME
sroff \- text formatting for Sanders printer
.SH SYNTAX
.B sroff
[ option ] ...
[ file ] ...
.SH DESCRIPTION
.I sroff
formats text in the named
.I files
for
printing on a Sanders Media 12/7 printer and
is similar to
.I nroff
and
.I troff,
from which it is derived.
The
.I Nroff/Troff user's manual
applies to
.I sroff
as well, with exceptions listed in the
.I Sroff User's Manual.
In general,
.I sroff
works similarly to
.I nroff,
although it also has many of capabilities of
.I troff.
.PP
If no
.I file
argument is present, the standard input is read.
An argument consisting of a single minus
.RB ( \- )
is taken to be
a file name corresponding to the standard input.
The options, which may appear in any order so long as they appear
before the files, are:
.TP "\w'\f3\-m\f1name 'u"
.BI \-o list
Print only pages whose page numbers appear in
the comma-separated
.I list
of numbers and ranges.
A range
.IB N - M
means pages
.I N
through
.IR M ;
an initial
.I -N
means
from the beginning to page
.IR N ;
and a final
.IR N \-
means
from
.I N
to the end.
.TP
.BI \-n N
Number first generated page
.IR N .
.TP
.BI \-m name
Prepend the macro file
.BI /usr/lib/tmac/tmac. name
to the input
.IR files .
.TP
.BI \-r aN
Set register
.I a
(one-character) to
.IR N .
.TP
.B \-i
Read standard input after the input files are exhausted.
.TP
.B \-q
Invoke the simultaneous input-output mode of the
.B rd
request.
.TP
.BI \-F name
Mount set of fonts
.I name.
This option is dependent on the fonts mounted in the printer.
Examples are
.B H10
for Helvesan 10 point Regular and Italic and
.B M12
for Messenger 12 (underlining when in font 2).
.TP
.B \-t
Direct output to the standard output instead
of the sandes printer.
.\".TP
.\".B \-w
.\"Wait until phototypesetter is available, if
.\"currently busy.
.SH FILES
.ta \w'/usr/lib/tmac/tmac.*  'u
/tmp/ta*	temporary file
.br
/usr/lib/tmac/tmac.*	standard macro files
.br
/usr/lib/sfont/*	font width tables for
.I sroff
.br
/dev/sanders	printer
.SH "SEE ALSO"
.I Sroff User's Manual
.br
J. F. Ossanna,
.I Nroff/Troff user's manual
.br
B. W. Kernighan,
.I
A TROFF Tutorial
.br
seqn(UA1), tbl(UA1)

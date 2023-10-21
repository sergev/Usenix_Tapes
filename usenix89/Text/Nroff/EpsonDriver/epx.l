.TH EPX l
.SH NAME
epx \- nroff postprocessor for epson printers
.SH SYNOPSIS
\fBepx\fP [ -\fBi\fP ] [ -\fBn\fP ]
.SH DESCRIPTION
.I Epx
is a postprocessor for
.IR nroff (1)
which generates printer codes for Epson compatible printers.
Usage is almost always:
.sp
.in +0.5i
nroff -Tepson files | col -fx | epx | lp -dEpson_R
.sp
.in
or equivalent.
.I Epx
reads from
.IR stdin
and writes to
.IR stdout .
It supports the built-in bold and italic fonts, half-line spacings
for super, and subscripts, and all of the greek and special characters.
.sp
It has been written to be compatible with
.IR col (1)
using the
.RB - x ,
and
.RB - f
options.
Output must be sent to the print spooler in raw mode,
using
.B lp -dEpson_R.
.I Epx
requires that the nroff driver table
.RB /usr/lib/term/ tabepson
exist and be used with
.I nroff.
.sp
Normally
.I epx
underlines words that would be italicized in
.IR troff (1).
The
.RB - i
option causes the printer to use true
.IR italics ,
(if possible).
If the printer is not capable of printing in
.I italics
the text will \fBnot\fP be underlined.
The
.RB - n
option puts the printer in Near Letter Quality mode.
No
.I italics
are possible in NLQ mode, so when the
.RB - n
option is used, italics are turned off, and any italic characters will
be underlined.
.SH SEE ALSO
nroff(1), col(1), lp(1), term(7).
.SH AUTHOR
Thomas E. Tkacik  (University of Virginia)
.SH BUGS
The greek and special character sets were designed using a Canon printer.
These characters may or may not look right, depending on the actual
printer used.
Perhaps, italics and NLQ mode should not be mutually exclusive.

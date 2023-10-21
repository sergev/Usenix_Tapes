.TH TCP 1 "21 Aug 1984"
.UC 4
.SH NAME
tcp \- tape copy with one tape drive
.SH SYNOPSIS
.B tcp
[
.B \-i
] [
.B \-o
] [
.B \-1..15
] [
.B \-c
] [
.B -nxx
]
.br
.SH DESCRIPTION
Need to copy a tape, but only have a
single tape drive?
.I Tcp
will do it.
In the
.I \-i
mode
.I tcp
reads in all the files of a tape,
copying them to disk along with enough
information to reconstruct the record
blocking. The tape is assumed to be
terminated with the usual double
tape marks.  In output mode (
.I \-o
)
.I tcp
reads the files and rebuilds the tape
exactly as it was. The
.I \-n
option allows several tapes to be saved
in a single directory, if that is
required, by giving each a separate
number (0-99), and then specifying
that number on both input and output.
The
.I \-c
option specifies that a conversational
mode is to be entered in the event
of tape read errors, in which the user
can specify the action to be taken
when such an error occurs: ignore the
error (copy whatever was received; this
may not give desired results on some
systems), retry the I/O, or abort the
entire copy operation. (Note that the
.I \-c
option is only available on Berkeley
systems with mag tape ioctls.)
The numeric options allow the tape drive
default selection to be changed. The
input operation uses /dev/rmtX, where
X can be specified from 0 to 15, while
on output /dev/nrmtX is used (which
can normally only accept 0-3 and 8-11
as valid drive numbers). In both cases
the default drive is drive 0.
.SH DIAGNOSTICS
The usual usage diagnostic and
others for cannot open and tape
read errors.
.PP
Exit status is 0 normally, 1 for
usage error and 2 for tape read error.
.SH DEFINES
.I MAXBUF
defines the maximum size of a tape
block. Default at 16000 works with both
4.2bsd and S3 on an 11/44.
.I MTIO,
if defined, compiles in the conversational
option, which requires the mag tape
ioctls from the Berkeley distributions.
.SH AUTHOR
Lyle McElhaney

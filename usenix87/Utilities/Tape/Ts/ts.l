.TH TS 1 "20 Aug 1984"
.UC 4
.SH NAME
ts \- tape format synopsis
.SH SYNOPSIS
.B ts
[
.B \-1...15
]
.br
.SH DESCRIPTION
.I Ts
will read a tape mounted on the
specified tape drive an display
a synopsis of the files and record
structure of the tape. The tape
drive used is /dev/rmtX, where X
is specified by the option. Default
is to /dev/rmt0.
.PP
The format of the display is:
.nf
.sp
	file 0:
		Records 0 through 2 (3 records), length = 512
	file 1:
		Records 0 through 0 (1 records), length = 512
		Records 1 through 20 (20 records), length = 10240
	End of tape reached.
.sp
.fi
The signal SIGINT is caught by
.I ts
and the report is flushed up to the
last block read, then
.I ts
aborts.
.SH DIAGNOSTICS
The usual usage diagnostic and
others for cannot open and tape
read errors.
.PP
Exit status is 0 normally, 1 for
usage error, 2 for tape read error,
and 3 for SIGINT caught.
.SH DEFINES
.I MAXBUF
defines the maximum size of a tape
block. Default at 16000 works with
4.2bsd and S3 on an 11/44.
.SH AUTHOR
Lyle McElhaney

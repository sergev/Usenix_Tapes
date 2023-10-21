.TH GP 1
.SH NAME
old.gp \- get page
.SH SYNOPSIS
.B old.gp
\-sn \-em
.SH DESCRIPTION
.PP
.I Old.gp
is a filter to grab pages out of SROFF'ed input.
Where
.I n
specifies a starting page (default is zero),
and
.I m
specifies the ending page to be copied
(default is infinity).
The start page must be at least as large as the end page.  
.I Old.gp
reads its input from the standard input and outputs to standard output,
so it may be used with pipes.
To read from or write to a file, use the < and > characters.
.SH Examples:
To pull pages ten through fifteen from file 'foo' and put
the resulting pages in file 'bar', use the command:

    old.gp -s10 -e15 <foo >bar

To SROFF the file 'foo', and send the output to file 'bar',
but keep only pages from eight until the end of the file,
use the command:

	old.sroff -msanders -t foo | old.gp -s8 -t5000  >bar

.SH Bugs:
.PP
If used with a file that hasn't been SROFFed with the proper macros,
nothing happens.
.PP
At present the pages are numbered from zero.
This takes care of the dummy first page most people use with sroff to
clear the Sander's.
.SH Author:
Jeremy Epstein, University of New Mexico
.SH Revisions:
Stanly Steinberg, University of New Mexico
.SH "More revisions:"
Suzanne O'Dell, Lawrence Berkeley Laboratory
.SH "SEE ALSO"
old.sroff(1), old.seqn(1), new.sroff(1), new.seqn(1), new.gp(1)

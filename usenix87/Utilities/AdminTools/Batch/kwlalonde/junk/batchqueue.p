.TH BATCHQUEUE PUBLIC "UofW"
.SH NAME
batchqueue \- show jobs waiting in specified batch queues
.SH SYNOPSIS
.B /usr/public/batchqueue
[-q queue_name]
[ -all | userid ... ]
.SH EXAMPLES
.nf
	/usr/public/batchqueue
	/usr/public/batchqueue -all
	/usr/public/batchqueue idallen
	/usr/public/batchqueue -all idallen fbaggins
	/usr/public/batchqueue -q 'vlsi*'
	/usr/public/batchqueue -q 'vlsi*' -all
.fi
.SH DESCRIPTION
.I Batchqueue
lists the files waiting in each of the specified batch
queues, with the head of the queue at the top and the
most recent entries at the bottom.
The queue_name can be any regular expression needed to
specify the desired batch queue(s).
If you have permission, the command that created the spool file
is displayed after each entry.
If you want to cancel one of your entries, see
.IR batchcancel (public).
.PP
An input file name of #nnn in the any of the troff or spice
queues indicates that you supplied standard input to the
.IR typeset (1)
or
.IR spice (CAD1)
command.
.I Typeset/spice
copied that input to a temporary file, awaiting the time that the
batch processor gets around to running your job.
The real file name of the temporary input file is /tmp/tyesetSTDIN_nnn
or /tmp/spiceSTDIN_nnn;
only the
.I nnn
part is shown in the listing from
.IR batchqueue ,
for brevity.
.PP
If no arguments are given, all waiting files are displayed.
The \-all option restricts the listing to your own files, plus
files belonging to any other userid names on the command line.
.PP
See
.IR batch (1)
for a description of queue priorities.
.SH FILES
.nf
.DT
/usr/spool/batch/*/cf[a-z]*	\- spool files
.fi
.SH AUTHORS
Ian! Allen, University of Waterloo
.br
Jim Barby, University of Waterloo
.SH "SEE ALSO"
batchcancel(public), batch(1), batchd(8), typeset(1),
cifplot(CAD1), spice(CAD1)
.SH BUGS
It is too slow, because it's a shell script.
.PP
User names that are not valid awk(1) regular expressions will cause
.IR awk (1)
to yell at you a lot.

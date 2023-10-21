.TH BATCHCANCEL PUBLIC "UofW"
.SH NAME
batchcancel \- cancel jobs waiting in specified batch queues
.SH SYNOPSIS
.B /usr/public/batchcancel
[-q queue_name]
jobid [ jobid ... ]
.SH EXAMPLES
.nf
	/usr/public/batchcancel 128
	/usr/public/batchcancel '12?'
	/usr/public/batchcancel '*'
	/usr/public/batchcancel -q 'vlsi*' '*'
.fi
.SH DESCRIPTION
.I Batchcancel
cancels the named files waiting in the specified batch queues.
The queue_name can be any regular expression needed to
specify the desired batch queue(s).
The arguments should match unique numeric suffixes of the
batch spool file names you want cancelled.
(Do not prefix the name with "cf"; use only the trailing numbers.)
.PP
The spool file names can be listed by the
.IR batchqueue (public)
command.
Normal users can only cancel their own jobs, and the suffix
need only be unique among their own jobs.
The Super-User can cancel any job, and the suffix must
uniquely identify the job from among all the others in the
specified batch queues.
.PP
The jobid is really the right half of a GLOB pattern to be
used in a 
.IR find (1)
command, so any GLOB expression will do.
If the jobid '*' or '' is used, all queued files submitted
by a user in the specified batch queues will be cancelled.
.PP
The queue entry is not removed, but it is overwritten with a
file that says "JOB CANCELLED".
The file is made readable, so others can see that the job is
cancelled.
.SH FILES
.nf
.DT
/usr/spool/batch/*/cf[a-z]*     	\- batch spool files
.fi
.SH AUTHORS
Ian! Allen, University of Waterloo
.br
Jim Barby, University of Waterloo
.SH "SEE ALSO"
batchqueue(public), batch(1), batchd(8), typeset(1),
cifplot(CAD1), spice(CAD1)
.SH BUGS
It won't cancel files that have already begun to execute.
You must use
.IR ps (1)
with the \-x option to find already-running batch files, and then use
.IR kill (1)
to kill them.
.PP
It is too slow, because it's a shell script.
.PP
File names that are not valid GLOB patterns will cause
.IR find (1)
to yell at you a lot.

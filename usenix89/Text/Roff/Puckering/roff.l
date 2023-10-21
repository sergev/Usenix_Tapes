.\" @(#)roff.l 2.0 87/01/20 COGNOS Inc.; 
.TH ROFF 1L "20 January 1987" "Cognos Inc."
.SH NAME
roff \- nroff/troff to printer or terminal with eqn or tbl
.SH SYNOPSIS
.IP \fBroff\fR 5
[\^\fB-d\fR\^]
[\^\fB-p\fR\^] 
[\^\fB-eqn\fR\^]
[\^\fB-tbl\fR\^] 
[\^\fB-m\fImacro\fR\^]
.br
[\^\fB-col\fR\^] 
[\^\fB-colcrt\fR\^]
[\^\fB-mail\fR\^]
[\^\fB-test\fR\^] 
.br
[\^\fIoptions\fR\^]
\fIfile\fR ...
.SH DESCRIPTION
.IX  "roff command"  ""  "\fLroff\fP \(em nroff using -mcognos"
.IX  "document production"  "roff command" 
.IX  "nroff" "macros" "-mcognos"
The \fBroff\fR command
is used to format documents with \fBnroff\fR or \fBptroff\fR and to
simplify the task of applying pre-processors (like \fBtbl\fR and 
\fBeqn\fR) to the input and post-processors (like \fBcolcrt\fR and 
\fBmore\fR) to the output.
.LP
Input is pre-processed by \fBtbl\fR or \fBeqn\fR if the \fB-tbl\fR or
\fB-eqn\fR options are specified, or if the file name contains ".tbl" or
".eqn".
.LP
If the input is pre-processed, then the output is, by default, filtered
through the post-processor \fBcolcrt\fR or through \fBcol\fR to remove
any vertical motions that would appear as non-printing characters at
the terminal.  The filtered output is then passed to \fBmore\fR, or to
the command specified by the environment variable $PAGER.
.LP
Normally, the formatter \fBnroff\fR is used.  When the \fB-p\fR option
is specified the formatter \fBproff\fR is used instead.  Output is then
typeset and sent to the laser printer.  No post-processing is performed
in this case.
.LP
Text can be piped to \fBroff\fR by using "-" as the filename,
as in the command "cat\ junk\ |\ roff\ -".
.SH OPTIONS
.IP \fB-col\fR
Forces the output from \fBnroff\fR or \fBptroff\fR to be filtered
through \fBcol\fR before being piped to $PAGER.  This eliminates most
vertical motions that would show up as control sequences in the output,
except for linefeed suppression, which can be handled by most line printers. 
Useful when piping \fBnroff\fR output to a regular printer.
.IP \fB-colcrt\fR
Forces the output from \fBnroff\fR or \fBptroff\fR to be filtered
through \fBcolcrt\fR before being piped to $PAGER.  This eliminates all
vertical motions that would show up as control sequences in the output,
making it suitable for display at the terminal.
.IP \fB-d\fR
Output to disk.  When this option is selected, the formatter \fBnroff\fR
is used and if pre-processing is done, then
the output is filtered through \fBcol\fR
.ul 1
without
being passed to $PAGER afterwards.  This option is suitable for saving
the formatted output in a file or for piping it directly to \fBlpr\fR.
.IP \fB-eqn\fR
Forces the input files to be pre-processed by \fBeqn\fR before being
piped to \fBnroff\fR or \fBptroff\fR.
.IP \fB-mail\fR
Specifies to \fBptroff\fR that the user is to be notified by mail when
printing has completed.  The actual \fBptroff\fR option is \fB-m\fR, but
this is overridden by \fBroff\fR's own \fB-m\fR option (see below).
.IP \fB-m\fImacro\fR
Specifies the macro package to be used by \fBnroff\fR of \fBptroff\fR.
If \fB-m\fR is specified, no macro package is used.
If omitted altogether, the \fI-mcognos\fR package is used, 
except when the filename matches one of the following patterns:
.sp
.ta +5n +15n
.nf
.ul 1
	Pattern	Macro Package
	*.[1-9]	-man
	*.l	-man
	*.man	-man
	*.nms	-ms
	*.nme	-me
	*.nroff	(none)
.ta	
.fi
.IP \fB-p\fR
Causes output to be sent to the printer.  The formatter \fBptroff\fR is
used, rather than \fBnroff\fR, and all post-processing is ignored (i.e.
\fBcol\fR, \fBcolcrt\fR, or $PAGER).
.IP \fB-tbl\fR
Forces the input files to be pre-processed by \fBtbl\fR before being
piped to \fBnroff\fR or \fBptroff\fR.
.IP \fB-test\fR
Enables test mode.  In this mode, \fBroff\fR will form the command and
echo it to the terminal, but not execute it.
.IP \fIoptions\fR
Any of the options available in \fBnroff\fR or, when \fB-p\fR is
specified, in \fBptroff\fR and \fBtroff\fR.
.SH FILES
.ta \w'/usr/man/manl/roff.l\0\0'u
.nf
/usr/local/bin/roff	shell
/usr/man/manl/roff.l	this document
.fi
.SH SEE ALSO
mcognos(l), nroff(1), troff(1), ptroff(1), eqn(1), tbl(1), 
col(1), colcrt(1), more(1), lpr(1)
.br
.I "Formatting Documents on the Sun Workstation"
.SH BUGS
.IP 1. 4n
When roffing a table to the terminal, \fBcolcrt\fR is used to filter out
half-line motions.  Unfortunately, it also replaces true underlines (in
the non-table portions of the text) with a separate line of dashes.
.LP
Please report any other bugs to Gary Puckering, COGNOS Inc. 
.br
(E-mail garyp@cognos).

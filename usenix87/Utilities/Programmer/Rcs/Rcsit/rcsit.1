...
... $Header: rcsit.1,v 1.12 86/03/27 14:05:08 mcooper Exp $
... 
... $Log:	rcsit.1,v $
... Revision 1.12  86/03/27  14:05:08  mcooper
... fixed syntax errors.
... 
... Revision 1.11  86/03/12  14:11:05  mcooper
... *** empty log message ***
... 
... Revision 1.10  86/02/03  14:23:07  mcooper
... Added description of ``Pascal'' types.
... Cleaned up table displays.
... Deleted most of the log messages.
... 
... Revision 1.9  85/12/19  14:22:41  mcooper
... Added updates from Joe Chapman for TeX.
... 
... Revision 1.8  85/12/16  17:33:33  mcooper
... Changed .template.* --> .header.*
... More appropriate name.
... 
...
.TH RCSIT 1l
.SH NAME
rcsit \- A program to prepare files for RCS
.SH SYNOPSIS
rcsit
[
.B \-chfsmMxp
] [
.B \-qad
] [
\-I\fIflags\fR
] [
\-R\fIflags\fR
] [
\-H\fIdirectory\fR
] [
\-F\fIheader-file\fR
]
.B file
[
.B file1 file2 ...
]
.SH DESCRIPTION
.I Rcsit
is a semi-intelligent program to prepare files for 
.I rcs(1).
This
involves putting the correct type of headings at the top of the file so
that 
.I rcs(1)
will update a header and log changes in the file.
.PP 
By default, 
.I rcsit
will use default headers ``hard-wired'' into itself for each different file
type that it ``knows'' about.  (See below for list of known file types).
.PP
If the \-H\fIdirectory\fR option is specified, then
.I rcsit
will use ``.header.suffix'' files
(where ``suffix'' is a suffix that 
.I rcsit
``knows'' about)
found in 
.I directory.
If a directory name is not specified on the command line, then
the environment variable TEMPLATES is used.
If TEMPLATES is not set, then the environment varialble, HOME is tried.
.LP
The following template files are recognized:
.sp
.DS
 Template Name             File Type
 -------------------------------------------
 .header.c                 Standard C
 .header.h                 C Include
 .header.f                 Fortran
 .header.sh                Shell Script
 .header.make              Makefile
 .header.man               Manual
 .header.p                 Pascal
 .header.tex               TeX
.DE
.PP
.I Rcsit
is ``semi-intelligent'' in that it can guess the type of headers to put in
the file by the type of file (C program, C include, makefile, shell script,
or manual).
It determines the file type by looking at the name of the file.
If the name of the file is ``Makefile'' or ``makefile'', then the
file type is taken to be for 
.I make(1).
The suffix after the last ``.'' in the file name is then used.
The following table shows the suffixes that 
.I rcsit
knows about:
.sp
.DS
 Suffix         File Type
 ----------------------------------------
 c              C Program
 h              C Include
 f              Fortran
 mk             \fIMake(1)\fR file
 sh             Shell Script
 csh            Shell Script
 [1-9]          Manual (digits 1 - 9)
 p              Pascal
 tex            TeX
.DE
.PP
If the environment variable ``RCSDIR'' is present, then 
.I rcsit
will attempt to make a symbolic link from it to ``RCS'' in
the current directory if the \-\fII\fR option is used and
there is no directory called ``RCS'' already, in the current
directory.  
If the \-\fII\fR option is used and ``RCSDIR'' is not specified
in the environment, then a normal directory called ``RCS'' will be
created.
This feature can be overrided with the \-\fId\fR option.
.SH OPTIONS
.TP 10
.B "c"
Force file type to be ``Standard C''.
.TP 10
.B "h"
Force file type to be ``C Include''.
.TP 10
.B "f"
Force file type to be ``Fortran''.
.TP 10
.B "M"
Force file type to be ``Manual''.
Note: If you also specify the ``I\fIflags\fR'' option,
.I rcsit
will run 
.I rcs(1)
to tell rcs what kind of comment string to use for the manual file.
.TP 10
.B "s"
Force file type to be ``Shell Script''.
.TP 10
.B "m"
Force file type to be ``Makefile''.  Note that this does the same thing as
the -s option does.  It just prints a different message.
.TP 10
.B "p"
Force file type to be ``Pascal''.
.TP 10
.B "x"
Force file type to be ``TeX''.
.TP 10
.B "F\fIheader-file\fR"
Use
.I header-file
as the header file.  (i.e. This file
is prepended to each of the files to be processed.)
.TP 10
.B "t"
Do not use any Template files for the headers.
.TP 10
.B "q"
Be quiet.  Don't print out what is going on.  Only error messages are
printed.
.TP 10
.B "d"
Do not attempt to make the directory ``RCS''.
See above for more info.
.TP 10
.B "a"
Turn off auto guessing of file type.
.TP 10
.B "I\fIflags\fR"
Check In file.
Run RCS's 
.I ci(1)
on the file with ``flags'' being passed onto 
.I ci(1) 
as the command line arguments.
.TP 10
.B "R\fIflags\fR"
Run 
.I rcs(1)
with ``flags'' as the command line arguments.
.SH FILES
/tmp/rcsit*	- temporary buffer
.br
~/.header.*   - template files to use as the header.
.br
.SH AUTHOR
Michael Cooper (mcooper@usc-oberon.ARPA)
.SH SEE ALSO
ci(1), make(1), rcs(1)
.SH DIAGNOSTICS
Complains about the usual stuff.  (i.e. - the specified file doesn't exist,
or it can't read it...just things like that.)
.SH BUGS
Acts oddly in the presence of beautiful women.

.TH RENAME
.SH NAME
rename - change the names of a set of files.
.SH SYNOPSIS
\fBrename [-i] file [...] <str1>^|=<str2>\fR
.SH DESCRIPTION
.PP
Rename will change the names of all the given files according to the
command specified as the last argument.  The -i (interactive) option
will prompt the user if the resulting file already exists.

.PP
Here are the valid commands to given in the last argument and what they
stand for:
.br
.in +6c
.ti -5c
<str1>=<str2>	: The string str1 is looked for in each file name and is
replaced with str2.
.ti -5c
<str>=		: The string str is deleted from each file name.
.ti -5c
<str>^		: The string str is added as a prefix to each file name.
.ti -5c
^<str>		: The string str is added as a suffix to each file name.
.ti -5c
<str1>^<str2>	: Combines last two operations.

.PP
The last argument must contain either a `=' or a `^'.

.SH AUTHOR
Gilles Chartrand
.SH BUGS

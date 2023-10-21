.TH APPTS 1 
.SH NAME
appts \- display current or selected day's appointments
.SH SYNOPSIS
.B appts 
[ month day year ] [ filename(s) ]
.sp
where
.I        month
is a numeric value between 1 and 12,
.I        day
is a numeric value between 1 and 31,
.I        year
is a numeric four digit value and
.I        filename(s)
is the name of the data file(s) to access.
.SH DESCRIPTION
.I Appts
is a program which uses the data stored by "vcal" or "eappts" to
display the user's appointments to standard out.
.PP
When a date argument is present,
.I appts
will display appointments for the requested month, day and year.
The default argument values are the current month, day and year.
When one or more filename arguments are used,
those data files will be searched.
.PP
Five companion programs are available to use with the
.I appts
program.  These are:
.br
.sp
.nf
vcal            calendar and notes program
eappts          enter appointments from the UNIX command line
autocall        set appointment reminders for the current day
lcal            displays month's appointments in tablular form
callme          sets a reminder prompt
.fi
.sp
For further information on these, reference the appropriate "man"
entries.
.SH AUTHOR
Mike Essex
.SH FILES
appts
.br
~/.appointments
.br
.SH "SEE ALSO"
vcal(1), eappts(1), autocall(1), lcal(1), callme(1)
.SH BUGS
There are year 2000 time bombs.

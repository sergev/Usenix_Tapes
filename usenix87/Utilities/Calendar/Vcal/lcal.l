.TH LCAL 1 
.SH NAME
lcal \- display current or selected month's appointments
.SH SYNOPSIS
.B lcal 
[ month year ] [ filename(s) ]
.sp
where
.I month
is a numeric value between 1 and 12,
.I year
is a numeric four digit value and
.I filename(s)
are the data files to be searched.
.SH DESCRIPTION
.I Lcal
is a program which uses the data stored by "vcal" or "eappts" to
display the user's appointments to standard out.
.PP
When an argument is present,
.I lcal
will display appointments for the requested month and year.
The default argument values are the current month and year.
If no filename is specified ~/.appointments will be searched.
If one or more filenames arguments are used those files will
be searched.
.SH AUTHOR
Mike Essex
.SH FILES
lcal
.br
~/.appointments
.br
.SH "SEE ALSO"
vcal(1), appts(1), eappts(1), autocall(1), callme(1)
.SH BUGS
There are year 2000 time bombs.

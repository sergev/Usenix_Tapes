.TH EAPPTS 1 
.SH NAME
eappts \- enter appointments
.SH SYNOPSIS
.B eappts [ filename ]
.SH DESCRIPTION
.I Eappts
is a program which is used to enter appointment data into the user's
~/.appointments file by default or the file specified in the optional
filename argument.
.PP
The user is prompted for a date, time and message.
The date is entered as three numeric values separated by spaces.
A two or four digit year value should be used.
.PP
Data entered using
.I eappts
may be used by four companion programs.
These are:
.br
.sp
.nf
vcal            calendar and notes program
appts           displays current or seleted day's appoinments
autocall        set appointment reminders for the current day
lcal            displays the month's appointments in tabular form
.fi
.sp
For further information on these, reference the appropriate "man"
entries.
.SH AUTHOR
Mike Essex
.SH FILES
eappts
.br
~/.appointments
.br
.SH "SEE ALSO"
vcal(1), appts(1), autocall(1), lcal(1), callme(1)
.SH BUGS
There are year 2000 time bombs.

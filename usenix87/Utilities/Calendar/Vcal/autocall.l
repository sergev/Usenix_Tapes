.TH AUTOCALL 1 
.SH NAME
autocall \- appointment reminder program
.SH SYNOPSIS
.B autocall [ filename(s) ]
.SH DESCRIPTION
.I Autocall
is a program which will start appointment reminders running in
the background.  At the desired time the user will be reminded
that it is time for the appointment.
.I Autocall
searches ~/.appointments by default or if filename(s) are
specified it accesses those files.
.PP
Data used by
.I autocall
may be entered using either of two companion programs.
These are:
.br
.sp
.nf
vcal            calendar and notes program
eappts          enter appointments from the UNIX command line
.fi
.sp
For further information on these, reference the appropriate "man"
entries.
.SH AUTHOR
Mike Essex
.SH FILES
autocall
.br
~/.appointments
.br
.SH "SEE ALSO"
vcal(1), eappts(1), callme(1), lcal(1), appts(1)

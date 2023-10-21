.TH VCAL 1 
.SH NAME
vcal \- visual calendar and notes program
.SH SYNOPSIS
.B vcal 
[ month year ] [ filename ]
.sp
where
.I month
is a numeric value between 1 and 12,
.I year
is a numeric four digit value and
.I filename
is the data file to be accessed.
.SH DESCRIPTION
.I Vcal 
is a visual calendar program which allows the user to enter notes
and appointments for each day of the month.  It can be used to
replace the "paper" desk or wall calendar with an electronic one
which can be accessed from any terminal or modem connected to the
computer system.
.PP
When a date argument is present,
.I vcal
will display the requested month and year.  The default argument values
are the current month and year.  When a filename argument is used
.I vcal
will uses that file instead of the default ~/.appointments file.
If ~/.appointments is not used the filename of the current file
will appear at the top of the
.I vcal
calendar.
.SH OPERATION
.I Vcal
is entered from UNIX by typing the
command followed by an optional month and year value.  A calendar
will be displayed on the screen with the cursor located on the first
day of the month.  The cursor may be moved to different days
in the following manner.
.br
.sp
.nf
key             function
---             --------
j               move cursor down
k               move cursor up
l               move cursor to the next day
h               move cursor to the previous day
b               move cursor to the beginning of the month
e               move cursor to the end of the month
.sp
.fi
.PP
Previous and following months may also be accessed.
.br
.sp
.nf
key             function
---             --------
p               previous month
n               next month
.sp
.fi
.PP
Up to eighteen appointments may be entered on each day of the month.
These may be displayed, modified or cleared in with the following keys.
They will operate on the day selected by the current cursor position.
.br
.sp 4
.nf
key             function
---             --------
space           display appointments for the selected day
m               enter or modify appointments
c               clear all appointments for the selected day
.sp
.fi
.PP
When the 'm' is pressed
.I vcal
enters notes mode.  While in this mode notes may be added, modified,
deleted, yanked or put.
.br
.sp
.nf
key             function
---             --------
j               move cursor down
k               move cursor up
i               begin data insertion
ESC/CR          end data insertion (only while in insert)
d               delete the selected line
y               hold the current line
p               insert the held message
CR              exit notes mode and return to the calendar
.sp
.fi
Data must begin with the appointment time in 24 hour format
(no ':' should be used).  Entering new data will erase the current
message on that line.  A held line will be available to insert until
it is changed or the user exits
.I vcal.
A line which is deleted using the 'd' key will be placed in the hold
buffer.
.PP
When the cursor is on the calendar a carriage return will cause the
the appointment data to be saved and
.I vcal
to exit.  Break or delete will cause
.I vcal
to exit without updating the ~/.appointments file.
.PP
A 'H' will display help messages.  A calendar help message
will be printed when
the cursor is on calendar days.  A notes help message
is printed when the cursor is in the notes area.
.PP
Five companion programs are available to use with the
.I vcal
program.  These are:
.br
.sp
.nf
appts           display current or selected days appointments
eappts          enter appointments from the UNIX command line
autocall        set appointment reminders for the current day
lcal            displays month's appointments in tabular form
callme          sets appointment reminders
.fi
.sp
For further information on these, reference the appropriate "man"
entries.
.SH AUTHOR
Mike Essex
.SH FILES
vcal
.br
/etc/termcap
.br
~/.appointments
.br
.SH "SEE ALSO"
appts(1), eappts(1), autocall(1), lcal(1), callme(1)
.SH BUGS
.I Vcal
may not work with extremely brain damaged terminals.  There must be
a 'CM' entry in the termcap for 
.I vcal
to draw the screen.  There are year 2000 time bombs.
.br

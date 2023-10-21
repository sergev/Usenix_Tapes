.TH ALARM L "21 January 1986"
.SH NAME
alarm \- a periodic alarmer for suntools
.SH SYNOPSIS
\f3alarm\fP [\f3-h\fP \f2min\fP] [\f3alarmdb\fP]
.SH DESCRIPTION
.LP
\f2alarm\fP reads an alarm database and signals the user whenever
one of the alarms becomes active.  If \f3alarmdb\fP is not
specified, ~/.alarm will be used.
.LP
The format of the lines in the alarm database is as follows:
.LP
.RS
date    time    message
.RE
.LP
Lines starting with a # and blank lines are ignored.  `date' can be any of
the following.  Minimum abbreviations are in caps, and case is insignificant:
.LP
.RS
.nf
Sundays       Mondays      Tuesdays       Wednesdays
THursdays     Fridays      SAturdays
.fi
.LP
Daily         Any
.LP
month/day
.RE
.LP
Specifying `month/day' creates a "one-shot" alarm; the others are periodic.
`time' is in the form hour:minute (based on a 24-hour clock; 0:00 - 23:59).
`message' is anything you want.  The program will read in this file, and
when an alarm is to go off, it will do one of the following:
.RS
.TP
1)
if the window is closed (iconic), the icon will
flash to indicate an alarm.  Open the window to
acknowledge the alarm.  Now, since the window is
open,
.TP
2)
if the window is open, the entire line will be written out.
.RE
.LP
If you change your alarm database (~/.alarm or `alarmdb'), you have to tell
the program to re-read the file (if you wish).  To do this, run ~/.alarmreset,
which the alarm program creates when it starts up.
.SH OPTIONS
.LP
There is only one option:
.TP
\f3-h\fP \f2min\fP
Specify the initial history value, in minutes.  \f2alarm\fP will
signal alarms in the last \f2min\fP minutes when initially starting
up.  The default time is 60 minutes.
.SH ENVIRONMENT
.LP
\f2alarm\fP looks at the environment variable ICONDIR for the directory
containing the icons.  The default directory is defined in the Makefile.
.SH "SAMPLE \f3~/.alarm\fP FILE"
.nf
#
#  alarm database for /usr/local/bin/alarm
#
#  copyright 1986 Exploding Penguin Software
#
.sp
#
#  Weekly alarms
#
mon     10:50   GRIP meeting at 11:00
wed     12:20   Tech meeting at 12:30.
fri     11:50   Faculty meeting at 12:00
.sp
#
#  Daily alarms
#
daily   16:50   Time to go home!
daily   17:00   What are you still doing here?
.sp
#
#  One-time alarms
#
1/22    15:50   Bentley seminar at 4:00.
1/24    15:20   Seminar at 3:30.
.fi
.SH BUGS
.LP
Invalid dates and time are accepted (4/38 = 5/8; 5:80 = 6:20).  Error checking
is only done on the format of the input, not on the values contained therein.
.LP
The program may or may not work correctly when Daylight Savings Time goes into
effect.  It also may or may not work in different time zones.  These cases
have not been tested.  If you have any problems, please contact the author.
.SH FILES
.LP
.ta 2.8i
.nf
/usr/local/bin/alarm	The program
$ICONDIR/alarm_off	Icon for quiescent state
$ICONDIR/alarm_on	Icons for active state
$ICONDIR/alarm_on2	   (toggles between these two)
~/.alarm	Alarm database
~/.alarmreset	Alarm signaller
.fi
.SH AUTHOR
.LP
Samuel Black (black@unc)

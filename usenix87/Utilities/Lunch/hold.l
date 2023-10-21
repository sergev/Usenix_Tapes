.TH HOLD local "10 September 1985"
.SH NAME
hold \- temporarily disable an unattended terminal.
.SH SYNOPSIS
\fBhold\fP
.SH DESCRIPTION
This program blocks the terminal generated signals SIGQUIT, SIGINT, and
SIGTSTP until the user supplies the password of the \fIlogin\fP user
associated with the terminal.  The password prompt includes this user
name.
.LP
Alarm signals are not blocked.  This is a feature.
.LP
Of course, this program does not protect the terminal from being physically
removed, disconnected, etc.
.SH SEE ALSO
passwd(1)

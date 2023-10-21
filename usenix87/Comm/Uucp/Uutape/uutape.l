.TH uutape local
.SH NAME
uutape \- make a tar tape of spooled uucp traffic
.SH SYNOPSIS
.B uutape
system [gradeletter]
.SH DESCRIPTION
.I uutape
will build a tar tape of spooled uucp data files, suitable for
reading in on the remote system relative to spool/uucp
.PP
If
.I gradeletter
is not specified
.I uutape
will build the tar with all spooled news and mail files, otherwise
only those with the appropriate grade letter are selected.
.PP
.I uutape
Attempts to avoid spooled uucp commands for other than news
and mail.
.SH AUTHOR
Andrew Macpherson. <andrew@stc.UUCP>
.SH BUGS
Untested with uusend commands, ignores mode information in Cmd files.

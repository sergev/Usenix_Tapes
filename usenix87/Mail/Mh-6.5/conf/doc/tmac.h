.\"	@(MHWARNING)
.\"    Try to keep only one copy of the documentation around
.\"    by re-defining macros and so forth.
.\"
.fc ^ ~
.\"    I pity the fool who tampers with the next line...
.ds ZZ -man
.de SC					\" Title section
.TH \\$1 \\$2 MH [mh.6]
..
.de NA					\" Name section
.SH NAME
..
.de SY					\" Synopsis section
.SH SYNOPSIS
.in +.5i
.ti -.5i
..
.de DE					\" Description section
.in -.5i
.SH DESCRIPTION
..
.de Fi					\" Files section
.SH FILES
.nf
.ta \w'@(MHETCPATH)/ExtraBigFileName  'u
..
.de Pr					\" Profile section
.SH "PROFILE\ COMPONENTS"
.nf
.ta 2.4i
.ta \w'ExtraBigProfileName  'u
..
.de Ps					\" Profile next
.br
..
.de Sa					\" See Also section
.fi
.SH "SEE\ ALSO"
..
.de De					\" Defaults section
.SH "DEFAULTS"
.nf
..
.de Ds					\" Defaults next
.br
..
.de Co					\" Context section
.fi
.SH CONTEXT
..
.de Hh					\" Hints section
.fi
.SH "HELPFUL HINTS"
..
.de Hi					\" History section
.fi
.SH HISTORY
..
.de Bu					\" Bugs section
.fi
.SH BUGS
..
.de En
..
.de ip
.IP "\\$1" \\$2
..

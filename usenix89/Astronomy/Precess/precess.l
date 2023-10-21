.TH PRECESS local "24 Feb 1986"
.UC 4
.SH NAME
precess \- precess coordinates from one equinox to another
.SH SYNOPSIS
.B precess
[
.B final 
.B equinox
]
.SH DESCRIPTION
.I Precess
precesses the standard input to the final equinox specified in either the
execution line or within the standard input.  Precessed coordinates are sent
to the standard output. The calculation uses Newcomb's precession method for
the mid-equinox. This program was translated from Vax fortran.
.PP
The format for input is 
.PP
OBJECT H M S +/- D M S STARTING_EQUINOX [FINAL_EQUINOX]
.PP
Note that the final equinox must be included somewhere. If it is specified
in the standard input it will be used otherwise the equinox in the command
line will be the default. Precessed data is compatible with the input.  Also 
note that the sign of the Declination MUST be specified ( either + or - ).
.SH BUGS
We'll see. This was stolen from the Yerkes IBM 1130 many years ago.
.SH SEE ALSO
Any of our other fine Astro programs.
.SH AUTHOR
Sean Casey (...oddjob!gizmo!sean) 2/24/86;
.PP
Yerkes Observatory, Williams Bay, WI 53191

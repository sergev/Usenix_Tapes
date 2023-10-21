#
#  form letter format
#
F
I BMACLIB/bibinc.fullnames
.ds CH
.ds CF
.de ][		\" format address automatically
.ds CR %
.Ab
\\*([A
.if !~\\*([T~~ \\*([T
.if !~\\*([D~~ \\*([D
.if !~\\*([I~~ \\*([I
.if !~\\*([N~~ \\*([N
.if !~\\*([C~~ \\*([C
.if !~\\*([Z~~ \\h'1i'\\*([Z
.Ae
.if !~\\*([L~~ .De \\*([L
..
.de Dy
.sp 2
.ti 4i
\\*(DY
..
.de Ab			\" Address begin
.DS L
..
.de Ae			\" Address end
.DE
..
.de De			\" Dear John: salutation
.LP
Dear \\$1:
..
.de Sl			\" The beginning of the end
.DS I 4i
..
.de Se			\" The end of the end
.DE
..
.de Cc			\" copy to (opps, more after the end)
.DS L
.if t .ta 2m
.if n .ta 4m
cc:	\\$1
.if !~\\$2~~ \&	\\$2
.if !~\\$3~~ \&	\\$3
.if !~\\$4~~ \&	\\$4
.if !~\\$5~~ \&	\\$5
.if !~\\$6~~ \&	\\$6
.DE
..

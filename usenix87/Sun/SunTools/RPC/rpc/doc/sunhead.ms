.\"
.\" Header File for Sun Manuals
.\" Use with -ms macros
.\" Adapted from TIS course header
.\" Sally Rutter, The Instruction Set
.\" 19th November 1985
.\"
.nr PS 11
.nr VS 14
.ds Un \s-1UNIX\s0
.\" define string Un = UNIX in smaller point size
.fp 7 L
.\" Put constant width font in position 7
.fp 6 LB
.\" Put bold constant width font in position 6
.\" The next 2 definitions are for constant width displays
.\" LS for display start and Lf for display finish
.de LS
.DS \\$1 \\$2 \\$3
.nf
.ft 7
.ps 10
.vs 12
.nr Tw \w'a'*8
.ta \\n(Twu,+\\n(Twu,+\\n(Twu,+\\n(Twu,+\\n(Twu,+\\n(Twu,+\\n(Twu
..
.de Lf
.ft
.fi
.ps 11
.vs 14
.DE
..
.\" The next definition is for text in constant width font.
.de LW
.ie\\n(.$ .nr ;G \\n(.f
.el.ft 7
.if\\n(.$ .if !\\n(.$-2 \&\f7\\$1\fP\\$2
.if\\n(.$-2 \{.ds }i
.if\\n(.f2 .ds }i \^
.ds}I \&\f7\\$1\fP\\$2\\*(}i
'br\}
.if\\n(.$-2 .if !\\n(.$-4 \\*(}I\f7\\$3\fP\\$4
.if\\n(.$-4 .if !\\n(.$-6 \\*(}I\f7\\$3\fP\\$4\\*(}i\f7\\$5\fP\\$6
.if\\n(.$ .ft \\n(;G
..
.\" The next definition ensures that there is at least
.\" 0.5i left on the page when a sub-heading is called
.rn SH Sh
.de SH
.br
.ne 1.5i
.Sh
..
.\" The next definition is simply a need macro
.       \" NE - need space
.de NE
.br
.ne 2i
..
.de Pb
'bp
..
.wh -1i Pb
.\" Need the definition of index (IX) from somewhere ...


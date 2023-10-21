.ll 72
.pl 72
.wh 0 hd
.wh 60 fo
.de hd
'sp 5
..
.de fo
'bp
..
.tr ~
.		TITLE
.de TL
.sp 15
.ce
.bd "\\$1"
..
.		AUTHOR
.de AU
.sp 15
.ce
by
.sp 2
.ce
\\$1
..
.		DATE
.de DA
.sp 3
.ce
( Dated \\$1 )
..
.		INSTITUTE
.de VU
.sp 3
.ce 4
Wiskundig Seminarium,
Vrije Universteit,
De Boelelaan 1081,
Amsterdam.
..
.		PARAGRAPH
.de PP
.sp
.ti +5
..
.nr CH 0 1
.		CHAPTER
.de CH
.nr SH 0 1
.bp
.in 0
.bd \\n+(CH. ~ "\\$1"
.PP
..
.		SUBCHAPTER
.de SH
.sp 3
.in 0
.bd \\n(CH.\\n+(SH. ~ "\\$1"
.PP
..
.		INDENT START
.de IS
.sp
.in +5
..
.		INDENT END
.de IE
.in -5
.sp
..
.		DOUBLE INDENT START
.de DS
.sp
.in +5
.ll -5
..
.		DOUBLE INDENT END
.de DE
.ll +5
.in -5
.sp
..
.		EQUATION START
.de EQ
.sp
.nf
..
.		EQUATION END
.de EN
.fi
.sp
..
.		ITEM
.de IT
.sp
.ta 13
.in 0
.bd "ISO~\\$1\\t\\"
..
.		IMPLEMENTATION 1
.de I1
.IS
.ti -3
1.~\\
..
.		IMPLEMENTATION 2
.de I2
.sp
.ti -3
2.~\\
..
.		TEST OFF
.ds OF \\xbtest off:~\\y
.		TEST ON
.ds ON \\xbtest on:~~\\y
.		LABEL
.nr RF 0 1
.de RF
.ta 6
.in 5
.ti -5
[\\n+(RF]\t\\
..
.de LB
.ti -3
.ta 4
\\$1\t\\
..

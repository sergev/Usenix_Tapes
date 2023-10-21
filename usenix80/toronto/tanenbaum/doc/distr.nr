.de bd
\\$1
..
.ll 72
.wh 0 hd
.wh -7 fo
.de hd
'sp 4
.ns
..
.de fo
'bp
..
.de IN INDENT
.in +\\$1
..
.de OU OUTDENT
.in -\\$1
..
.de CS CASE
.sp
.ti -2
-\ \\
..
.de PA PARAGRAPH
.ti +5
.ne 3
.sp
..
.de HD HEADER
.br
.ne 7
.sp 2
.bd "\\$1."
.PA
..
.nr CH 0 1
.de CH CHAPTER
.nr SE 0 1
.HD "\\n+(CH.\ \\$1"
..
.de SE SECTION
.nr SB 0 1
.HD "\\n(CH.\\n+(SE.\ \\$1"
..
.de SB SUBSECTION
.HD "\\n(CH.\\n(SE.\\n+(SB.\ \\$1"
..
.de AP APPENDIX
.HD "EXAMPLE:\ \\$1"
..
.bd "               Parametrization of software distributions"
.sp
.ce
Johan W. Stevenson
.sp
.ce
Wiskundig Seminarium, Vrije Universiteit, Amsterdam.
.sp
.PA
A consequence of the lack of support for UNIX is,
that these systems diverge quite rapidly.
Moreover, because most UNIX systems are used at universities and
other educational or research institutions,
UNIX users tend to repair bugs, improve programs and
develop new software themselves.
Because a central organization for collecting and distributing bug reports
and new software is missing, UNIX systems will continue to diverge.
One of the great disadvantages is that the software exchange
will become more difficult.
.PA
Last year we tried to distribute some of our home made programs,
notably our Pascal system.
Because the software was not stabilized when we started distributing,
we had to make several distribution kits.
Each time it took several days to collect all files,
to produce a 'run' file and a 'READ_ME' file, and to test it.
The results were discouraging.
Each time we overlooked some little, but necessary, changes we made
to the standard software.
Each time the 'READ_ME' file was too long and too cryptic to be
understood except by very experienced UNIX users.
Each time we failed to note some of the problems concerned with
installation on machines without hardware floating point unit
or without an 11/45 type memory management unit.
.PA
To save you the same troubles, we shall give you some hints.
First of all, what does a good distribution kit look like?
.IN 5
.CS
Installation must work on the standard UNIX system as distributed by Bell Labs.
Non-standard software needed for installation, like archivers, compilers,
assemblers and loaders, must be included.
The distribution medium must not give problems for standard systems
(do not use special tape or disk formats).
.CS
If special hardware is needed, this must be clearly indicated.
Examples: 11/45 type memory management because some processes need more
than 64k bytes logical address space, special terminals with cursor addressing
or graphic capabilities.
.CS
All sources must be included. Binaries alone are useless.
.CS
All places likely to be changed to fit the installers installation must be
clearly indicated.
These places include
all occurrences of pathnames,
cursor addressing routines and other terminal dependant features,
and places where a trap to the floating point interpreter must be included.
.CS
Besides the normal documentation like tutorials, reference manuals and 'man'
files, there must be an installation guide, preferably on a file
called 'READ_ME'.
This file must explain which changes to the standard UNIX software are
necessary, which hardware is needed and where possibly changes
are to be made.
.CS
There must be a thoroughly tested shell command file, preferably
called 'run', compiling and installing all distributed programs.
.OU 5
.HD "Adapt to standard UNIX"
It is the distributors task to make the distribution kit suited for the
original UNIX system, the common ancestor of source and target installation,
because only he knows of the changes made to his system.
Because most programs are not written to be distributed, a lot of additional
effort is required by the distributor.
It appears to be extremely difficult to identify all places making use
of local features, simply because one gets used to these features.
.PA
However, there are two powerful aids.
The first is to maintain a list of local features to be checked
when making distributions.
This list may also be useful for new users of your system.
An example of such a list is given in the appendix.
The second and most valuable aid is a copy of original UNIX
to test your distribution kit.
A final test may be to let an arbitrary UNIX user install it on an
original UNIX system.
.PA
To illustrate that it is really very hard to find all trouble spots,
we give you two examples.
.IN 5
.PA
In the early days we noticed that the kernel saves the floating point
registers on a process switch without changing the floating point status
to double mode.
This means that only 32 bits are saved and restored, whereas there may be
valuable information in the least significant 32 bits.
This information is lost.
Most C programs do not have any problems, because they do not change
the floating point status.
The results of Pascal programs however were unpredictable.
There is an easy fix, but we forgot to include it in the distribution kit.
.PA
Occasionally the library routine 'fcvt' did wild stores when converting
small floating point numbers, because it did not check for array bounds.
Even when such bugs are included in the check list, you might overlook
that the routine 'fcvt' is used.
.OU 5
.HD "Ease the installers job"
.br
It is the installers responsibility to adapt the distribution to his
system.
But the distributor must ease the installers job by pinpointing
the installation dependent code.
Each hour spent by the distributor may save several hours each time
the software is installed.
It asks for some imagination to predict which code must be changed to fit
another installation.
Some examples are: pathnames and cursor addressing options.
.PA
Many pathnames are used more than once.
For instance the pathname of a library used by some compiler
you are distributing will occur in the compiler source
as well as in the 'run' file.
Installation would be a lot easier if
each pathname has to be changed only once and if
all pathnames are found in the same file.
.PA
For C programs these conditions can easily be satisfied by using
the '#include' facility and by concentrating all pathnames in
a single file, say 'local.h'.
The same pathnames can be used in command files as follows:
.IN 5
.CS
give your old 'run' file the name 'run.c'.
.CS
insert at the front of 'run.c'
	#include "local.h"
.CS
replace all explicit pathnames in 'run.c' by identifiers defined
in 'local.h'.
.CS
add the following 'run' file:
	cc -P run.c
	sh run.i
	rm run.i
.OU 5
.PA
The flag '-P' causes only the C-preprocessor to be run.
The file with suffix '.i' contains the output of the preprocessor.
Note that strings, surrounded by double quotes, are treated correctly
by the shell and that C-type comments are allowed in 'local.h',
because they are removed by the preprocessor.
.PA
The preprocessor is not only useful for shell command files.
Several times we used it for assembly programs as well.
A nice example is the UNIX kernel.
All assembly routines for the kernel are concentrated in one
assembly file.
However, there are two different versions.
The file 'm45.s' is for systems
with separate address spaces and, as option, a hardware floating point unit.
Systems without these features use 'm40.s'.
This caused several problems.
.PA
First, the 11/60 has hardware floating point, but
no separate address spaces,
so there had to come a third version 'm60.s'.
Second, there are some other differences between PDP-11 machines,
notably the presence of a UNIBUS map device on 11/70 machines.
Code was included in the kernel to test the global variable 'cputype'
at run time.
Third, while 70% of these two or three files could be identical,
it was clear that they suffered from the multiple copy update problem.
At several places the code was different without doing a different job.
.PA
All these problems may be resolved by merging these files into
the file 'm.c'.
The preprocessor should account for the differences.
Translation of the file 'm.c' can be done as follows:
	cc -P m.c
	as m.i
	mv a.out m.o
.PA
Some aspects of the preprocessor may cause problems when applying it
to non-C files:
.IN 5
.CS
only C-type identifiers may be defined and replaced.
.CS
only C-type strings are recognized; this is relevant, because
strings have to be copied literal.
.CS
only C-type comments are recognized; within comments there is no text
replacement.
.OU 5
.PA
Another disadvantage is, that too many control lines like #ifdef and #define
result in hard to understand files,
especially if the #ifdef's are nested.
.PA
But when used moderately it is valuable tool making software exchange easier.
Moreover, the preprocessor is available on every UNIX installation.
Making a distribution kit is not harder, because the 'READ_ME' file
can be much shorter, and installation is a lot easier.
.AP "Check list"
.IN 2
.CS
do not use Yale shell commands like 'cd', 'set' and 'newbin'.
.CS
avoid the use of separate address spaces where possible.
.CS
especially avoid the use of separate address spaces together with
floating point instructions, because the floating point interpreter
needs mapped address spaces.
.CS
do not use local library routines
or include them in your distribution.
.CS
the following library routines may cause problems elsewhere, because we
repaired some bugs:
.IN 4
.sp
pipe: register r2 was not saved before use.
.sp
ecvt/fcvt: they did some wild stores for small reals.
.sp
atof: leading '+' was not recognized, not even in the exponent.
.sp
wait: used nargs() before, so could not be used together with
separate address spaces.
.OU 4
.CS
the following changes to the kernel may affect portability:
.IN 4
.sp
the times system call has been changed in accordance with
the diff-listing ditributed by Bell.
.sp
saving of floating point registers is changed so it works
for processes using floating mode.
.OU 4
.CS
do not use the -o flags of 'cc', 'as' and 'ld'.
.CS
avoid empty replacement text for '#define', because this may cause the
next line to be skipped.
.CS
Because some tables are increased always check your distribution
on a original UNIX system.
.OU 2
.AP "The files 'local.h' and 'run.c' for Pascal"
The parameters to be tuned by the installer of Pascal are found on the
file 'local.h', which looks like:
.sp
.nf
/* comment out if you do not have separate I and D */
#define SEPARATE_ID	1

/* comment out if you do not have hardware floating point */
#define HARDWARE_FP	1

/* path names */
#define	AS_PATH		"/usr/bin/as"		/* assembler */
#define	LD_PATH		"/usr/bin/ld"		/* loader */
#define PC_PATH		"/usr/bin/pc"		/* analog of cc */
#define	PEM_PATH	"/usr/ovl/pc:pem"	/* compiler proper */
#define	LIST_PATH	"/usr/ovl/pc:list"	/* list errors */
#define	OPT_PATH	"/usr/ovl/pc:opt"	/* Em1 optimizer */
#define	PDP_PATH	"/usr/ovl/pc:pdp"	/* translate to 'as' */
#define	RT0_PATH	"/usr/lib/pc:rt0.o"	/* run time startoff */
#define FRT0_PATH	"/usr/lib/pc:frt0.o"	/* idem, with fptrap */
#define	BSS_PATH	"/usr/lib/pc:bss.o"	/* end of bss */
#define	EMLIB_PATH	"/usr/lib/pc:emlib.a"	/* Em1 library */
#define	PRLIB_PATH	"/usr/lib/pc:prlib.a"	/* Pascal library */

/* do not change the five zero's at the end */
#define TMP_PATH	"/usr/tmp/ptmp00000"
.fi
.sp
.ce
-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
.sp
The Pascal run file makes use of several run files in subdirectories.
Only the file 'run.c' is preprocessed, the others are plain
command files.
Here follows the file 'run.c':
.sp
.nf
#include "local.h"

: indicate if 'pem' may use both address spaces, I and D
: if not, then only moderate sized programs may be compiled
#define PEMFLAG 0
#ifdef SEPARATE_ID
#  define PEMFLAG 1
#endif

: boot pem by 'pc pem.e', not by 'pc pem.p'
#define PEMBOOT 1

#define FFLAG 		/* some spaces as replacement*/
#ifndef HARDWARE_FP
#  define FFLAG "-f"
#endif

cc -O -s pc.c; mv a.out PC_PATH

cc -s list.c; mv a.out LIST_PATH

chdir emlib; sh run
mv pc:rt0.o RT0_PATH
#ifndef HARDWARE_FP
mv pc:frt0.o FRT0_PATH
#endif
mv pc:bss.o BSS_PATH
mv emlib.a EMLIB_PATH
chdir ..

chdir prlib; sh run                ; mv prlib.a PRLIB_PATH; chdir ..
chdir opt  ; sh run FFLAG          ; mv opt OPT_PATH      ; chdir ..
chdir pdp  ; sh run FFLAG          ; mv pdp PDP_PATH      ; chdir ..
chdir pem  ; sh run PEMBOOT PEMFLAG; mv pem PEM_PATH      ; chdir ..
.fi

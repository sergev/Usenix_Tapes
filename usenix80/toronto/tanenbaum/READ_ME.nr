.tr ~
.ti +5
This file describes the process of installing Pascal.
We have tried to make installation as easy as possible,
especially for standard version 6 UNIX systems,
with or without hardware floating point,
with or without separate instruction and data address spaces.
At sites where the pathnames near the root of the filesystem are modified
you have to recompile the Pascal system.
But also for these sites installation is quite simple.
.sp
.ti +5
Pascal programs can be run in two different ways.
The first way is to interpret them.
The Pascal compiler produces code for a virtual machine, called EM1.
This EM1 machine can be emulated by an EM1 interpreter.
The advantages of interpretation are the fast compilation and the extensive
checks by the interpreter.
It is especially suited for teaching environments and for debugging.
.ti +5
The second way of running Pascal programs is by compilation into PDP-11 code.
The EM1 code is then translated into UNIX assembly code 'as'.
The UNIX assembler and loader are used to produce a direct executable program.
The advantage is fast execution.
.ti +5
We offer you two options for installation: one is the interpreted version only,
the second one is the interpreted version and the compiled version together.
Only the compiled version is not very useful, because you have to install
nearly all the files for the interpreted version as well.
These two options are indicated by the letters 'I' and 'C'.
.sp
.ti +5
All the sources and the installation command files are heavily parameterized
to make adaption to your UNIX system as easy as possible.
There is one file "src/local.h" where all the installation parameters
are centralized.
The C preprocessor is used to fill in these parameters in the sources and
in the command files.
See "doc/distr.nr" for a description of this method.
.sp
.ti +5
There is one nasty problem in installing this distribution:
you probably need modified versions of the assembler 'as',
of the loader 'ld' as well as of the C compiler 'cc'.
The modification in the assembler and loader have to do with
very large files. The C compiler is modified to repair two bugs
in the optimizer and to increase the maximum number of #define's.
Moreover, some modules in "/lib/liba.a" and "/lib/libc.a" must be
corrected.
You will find more information about these modifications below.
.sp
.ti +5
You should start by logging in as bin (uid=3).
It will not work if you are super-user.
If you can not become bin, then you have to change the ownership
of all files and directories on the distribution into your uid.
So you can create and remove files in the distribution tree easily.
.sp
The general installation outline is:
.in +5
.ti -4
0.~~rebuild the original distribution tree structure.
.ti -4
1.~~install the improved 'as', 'ld', 'cc' and library modules if necessary.
.ti -4
2.~~adapt "src/local.h" to your installation.
.ti -4
3.~~if you have not changed pathnames goto 5.
.ti -4
4.~~bootI
.ti -4
5.~~installI
.ti -4
6.~~testI. if ok goto 8.
.ti -4
7.~~try to fix the bug and iterate to 4.
.ti -4
8.~~if interpreted version only goto 14.
.ti -4
9.~~if pathnames in "src/local.h" have not been changed goto 11.
.ti -4
10.~bootC
.ti -4
11.~installC
.ti -4
12.~testC; if ok goto 14.
.ti -4
13.~try to fix the bug and iterate to 10.
.ti -4
14.~installation finished.
.in -5
.sp
A short explanation follows:
.sp
.in +5
.ti -4
0.~~It is very important for the command files that the original
tree structure is restored.
If Pascal is distributed by tape, you have to proceed as follows:
.in +5
.ti -4
a.~~Create a directory, say pc_dist, on a device with at least ????
free blocks, for instance a fresh RK pack.
.ti -4
b.~~goto that directory.
.ti -4
c.~~extract ./FROMTAPE from the distribution tape.
.ti -4
d.~~type
	sh FROMTAPE unit
.br
Unit must be the tape drive unit number.
This command file creates a bunch of directories and distributes the
tape contents over these directories.
It uses 'tp' to access the tape.
.ti -4
e.~~Check the current tree structure of pc_dist.
It must be the same as on the file TREE.
Links do not have to be restored, but it saves you some disk space.
.in -5
.sp
.ti -4
1.~~To compile the sources of this distribution you need several modifications
to 'as', 'ld', 'cc', 'liba' and 'libc'.
First we describe the necessary modifications:
.in +8
.sp
.ti -6
as~~~~the original version is not capable of assembling large files.
Large means that the resulting "a.out" file is bigger than 65535 characters.
Another important fix is, that the symbol table is increased from 1500
to 2500 entries.
A third modification is the addition of the "-o" flag used by "pc -C".
A last fix necessary for "pc -C" to work is, that 'as' now exits
with a decent exit status.
.sp
.ti -6
ld~~~~the original loader could not load large files.
The reason of this is, that the seeks were implemented incorrectly.
Another change is that we added a "-o" option, which is used by "pc".
By this options you can give the object file another name than the
default "a.out".
.sp
.ti -6
cc~~~~we repaired two bugs in the optimizer and increased the size
of the #define table.
.sp
.ti -6
liba~~the routine 'ecvt' did wild stores outside its character buffer.
.sp
.ti -6
libc~~the routine 'atof' did not recognize '+' signs.
.in -8
.sp
The new sources of these programs are found in the directories
"new/newcc", "new/newld", "new/newas" and "new/newlib".
Precompiled version of these programs are also found in these directories.
Each of these directories contains a "run" file and an "install" file.
For each of these programs proceed as follows:
.in +5
.sp
.ti -4
A.~~Go to the directory pc_dist/new
.sp
.ti -4
B.~~Check the install file to see that the absolute pathnames are correct.
For the new C compiler also check the file "local.h" in the directory
"src/newcc".
This file contains all pathnames previously found in "cc.c".
They are all exactly the same as for the original UNIX system.
.ti +5
The sources of the assembler and loader contains some pathnames,
for instance
	/lib/as2	the assembler overlay
	/lib/lib	to find libraries
	/tmp		for temp files
.br
Our sources contain the pathnames as in the original UNIX system.
When all pathnames are correct, type:
	sh install
.br
The old version is always saved, and the new version is copied onto
the old version.
.sp
.ti -4
C.~~Convince yourself that the new version works correctly.
Check this very carefully.
For instance, you may try to recompile the new version using 'run'.
.sp
.ti -4
D.~~If you have problems with the new version, you may try to
recompile the sources.
First restore the old version. Then type:
	sh run
.br
This makes a new version in the current directory.
Afterwards you must proceed as described under B.
Because several different C compilers are distributed by Bell,
it may happen that you have a slightly newer version
yourself.
Then you might solve some problems by only installing
the new "cc" and "c2", not "c0", "c1", "f0" and "f1".
If you can not get them working, please let us know.
.in -5
.sp
It is not very useful to proceed with the Pascal distribution itself,
if you do not succeed in making the necessary modifications
to these three programs.
Of course, if you changed these programs yourself in the same sense
as we did, you do not have to install our versions.
If you made local modifications to these programs in different aspects,
you have to merge both versions yourself.
.sp
.ti -4
2.~~When the tree structure is restored, you must change to "pc_dist/src".
This directory contains the file "local.h".
Inspect this file very carefully.
If your machine has separate address spaces (11/45 and 11/70 for instance),
you may remove the comment symbols "/* " and " */" on line 2.
If your machine has 11/45 type hardware floating point unit (or firmware
as on the 11/60), you may remove the comment symbols on line 5.
If you intend to install the interpreted version only,
you must remove the comment symbols on line 8.
If not absolutely necessary, do not change the pathnames,
because you have to do a lot more work then.
See to it, that AS_PATH, LD_PATH and TMP_DIR exist on your installation
and that all other directories mentioned in pathnames exist.
.sp
.ti -4
4.~~Type
	chdir pc_dist/src/rundir
	sh bootI
.br
bootI is the name of a three line command file,
which calls the C preprocessor to include a header file into the file "bootI.c".
This is not a C program, but a parameterized command file.
The suffix ".c" is there to trick the C compiler.
The preprocessed version gets suffix ".i" and is fed into the shell.
This same trick is used for most command files.
.ti +5
BootI recompiles all sources involved with the interpreted version and replaces
the old object files with new ones.
The boot process is quite tricky and takes quite a long time, but is fully automated.
Please do not interrupt the boot job until the message
.sp
	Interrupt allowed
.sp
appears; otherwise the tree structure will be in a intermediate state and
you have to repair it by hand.
Of course, only interrupt when necessary.
.sp
.ti -4
5.~~Type
	sh installI
.br
The object files will be moved to their correct position as specified
in "local.h"
All mentioned directories must be writable for you.
.sp
.ti -4
6.~~Type
	sh testI
.br
You must be able to compile and interpret Pascal programs now.
Four test programs are included to check this.
These are found in "src/test".
The command file should respond with:
.nf
.sp
   Program t1: 20 tests completed.Number of errors = 0
   Program t2: 21 tests completed.Number of errors = 0
   Program t3:  6 tests completed.Number of errors = 0
   Program t4:  8 tests completed.Number of errors = 0
.fi
.sp
If anything else is reported, you must try to find the error, repair it and reboot.
.sp
.ti -4
7.~~troubleshooting: we only can give you some hints.
.in +5
.ti -3
-~~if you only installed the binaries and did not boot, you may try to
reboot and install again.
.ti -3
-~~If pc gives messages like
.sp
  fatal error nn in xxx. Call for help on an expert
.sp
then program xxx died because of signal nnn.
A created core file is automatically removed by pc, unless the "-t"
option is given.
Some hints are:
.in +5
.sp
.ti -4
illegal instruction:
.br
do you have HARDWARE_FP defined in "local.h"
while your machine lacks the necessary hardware?
An 11/40 type floating point unit is not good enough, but you might try
to change the interpreter, so that it uses the 11/40 hardware floating point unit.
.sp
.ti -4
bad system call:
.br
if you changed the system calls at your installation,
you must see below.
.sp
.in -5
.ti -3
-~~if you changed the system calls with respect to original UNIX,
this may be a reason.
The only system calls used for standard pascal programs are:
	exit    1*
	read    3*
	write   4*
	open    5*
	close   6*
	creat   8
	unlink  10
	seek    19
	getpid  20
	gtty    32*
.br
These system calls are done by the EM1 instruction 'mon'.
This instruction is emulated by the interpreter (see "int/em.s").
The arguments and results are passed via the stack.
You may change the library sources in "libpr/esrc:pr.a", "libP/esrc:P.a"
and "lib2/esrc:2.a" to reflect the changes you made.
The system calls with '*' are essential for bootI, because one of
the programs (encode.boot) is precompiled.
If one of these starred system calls is changed at your installation,
you have to boot this distribution at an original UNIX system.
Another possibility is to change the interpreter routine emulating
the 'mon' instruction.
.ti -3
-~~The following programs are assumed to be present at your installation:
	cc
	as
	ld
	sh
	ar
	mv
	ln
	rm
	mkdir
	cat
	if
	echo
	exit
	cp
.br
Moreover, they must be upwards compatible with the versions of UNIX version 6.
.in -5
.sp
.ti -4
10.~Type
	sh bootC
.br
This recompiles all sources involved with the compiling version.
.sp
.ti -4
11.~Type
	sh installC
.br
The object files for the compiling
version will be moved to their correct position
as specified in "../local.h".
.sp
.ti -4
12.~Type
	sh testC
.br
This must give the same results as "testI", but faster.
If anything else is reported, you must try to find the error, repair it,
and recompile (bootC).
.sp
.ti -4
13.~The same comments as for 7.
The libraries containing system calls are
now : "libem/emsrc.a", "/libpr/ssrc:pr.a", "libP/ssrc:P.a" and "lib2/ssrc:2.a".

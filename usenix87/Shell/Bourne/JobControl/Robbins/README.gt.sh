README.gt.sh --- README file for Georgia Tech mods to the shell

This and the following eight postings should be all you need to add the
following features to the Bourne shell:

1.  BSD style job control on Berkeley Systems. Code to outwit symbolic
    links for shells which have 'pwd' built in.  Also code to print a
    resource usage summary after every command on BSD systems.  These are
    courtesy of the folks at BRL, who gave me permission to post their
    code.

2.  The ability to catch Control-D's, and force you to use "exit", also
    courtesy of BRL.  This will work whether or not you are on a BSD system.

3.  The <> I/O redirecter is now documented, and also works (courtesy of
    ihnp4!trwrb!lcc!brian's recent posting in net.unix-wizards).

4.  The shell parameter $+ gives you the parent process id of the shell.
    It will track the value of the getppid() system call.

5.  The shell will read in $HOME/.shrc on startup, if that file exists.
    This file is read *after* /etc/profile and $HOME/.profile (unlike the
    csh, which readc ~/.cshrc before ~/.login).

6.  The ~ and ~person notation is understood, both for command line arguments,
    and in the PATH and CDPATH shell variables.
 
7.  Special sequences in PS1 (the shell's prompt string) will print useful
    info at the prompt.  Currently, you can get or all or some or none of:
	the time of day
	your current directory (if the shell has pwd is built in)
	your machine's hostname
	your login name
	the current 'event' number for the ...

8.  History mechanism.  The history mechanism is powerful, yet easy to use.
    Although different from the csh's, it is somewhat more general and
    orthogonal.  The shell will save history across login sessions,
    automatically restoring on login, and saving on exit or on the exec builtin.

On a Pyramid, the shell has some additional capabilities:

9.  The 'att', 'ucb' and 'universe' commands are built in.

10. The $UNIVERSE shell variable tracks the current universe.

11. An additional sequence for the prompt to print the current universe.

*****************

I am posting diffs for the versions of the Bourne shell listed below.
Here are the instructions for setting up the makefile for each shell,
depending on your target machine and OS.  For all versions, you will need
the files history.c and homedir.c; these have been added to the makefile,
but will only be posted once.

It will help if you have the 'patch' program.  It was just reposted around
the middle of May, 1985 (Version 1.3). If you don't have it, someone at your
site or someone you know probably does.

First, unshar this article.  You wil have the following files:

README.gt.sh		# this file
Bugs			# some known bugs in what I've added
signal.c		# courtesy of BRL
ulimit.c		# courtesy of BRL
jobs.c			# courtesy of BRL
homedir.c
history.c
sample.shrc
aliases.sh

Next, make a new directory, and copy the source for the version of the shell
that you are going to modify, into it.
Copy the *unformatted* man page for that shell into the directory also.
Move history.c and homedir.c into the directory.

Find which version of the shell you have, and follow the supplementary
instructions below.

1. The Berkeley /bin/sh as distributed with 4.2, for 4.2BSD.
	Add the files signal.c and jobs.c to the directory.
	Get parts 2, 3 and 4, and run patch on them.
	Run make.

	On a Pyramid, follow the previous paragraph but don't run make yet.
	First, make sure you are in the 'ucb' universe.
	Remove all references to signal.c and signal.o from the makefile.
	Now run make.

2. The System V Release 2 shell as distributed from ATT (for the vax).
	On System V systems, the job stuff is conditionally compiled in.
	Get parts 5, 6, and 7, and run patch on them.
	Remove the -DJOBS and all references to signal.c, ulimit.c,
	jobs.c, and their .o files, from the makefile (sh.mk).
	Run make (make -f sh.mk).
	The sh.1 file will need editing to remove any reference to the 'J'
	flag, the 'I' flag, and to all the job control features.

	On BSD systems, copy signal.c, ulimit.c, and jobs.c into the directory.
	Get parts 5, 6, and 7, and run patch on them.
	Run make. (make -f sh.mk)

	On a Pyramid, copy ulimit.c, and jobs.c into the directory.
	Run patch.  Remove signal.o and signal.c from the makefile,
	and the testing for u370 for xec.c. Be sure to be in the ucb
	universe, and then run make. (make -f sh.mk)

3. The System V Release 2 shell as modified at BRL, for BRL Unix.
	Get parts 8 and 9, and run patch on them.

	There are two ways to compile this version under BRL Unix.

	A. Use the standard BSD make and cc. In this case, all you will need
	   is history.c and homedir.c.  Run make.

	B. Use the System V emulation, /usr/5bin/make and /usr/5bin/cc.
	   In this case, make sure your PATH is set properly. Remove the
	   -DBSD from the makefile, and also any references to signal.c,
	   and ulimit.c, and their .o files.  Now run /usr/5bin/make.
	
	On a Pyramid, run patch.  Edit the makefile to exclude signal.c
	and signal.o.  Be sure to be in the ucb universe, and then run make.

**********

Aliases.sh is a bunch of useful shell functions --- only of value for one
or the other of the System V Release 2 Versions of the shell.

Sample.shrc is a sample .shrc file.  In particular, it gives you some
compatibility with the Korn shell.  It sets PPID=$+, and if the ENV
environment variable is set to a file name, it will source that file
(that is how the ksh does ~/.shrc).

If you want job control to be turned on automatically (on a BSD system),
add the line
	set -J
to /etc/profile (or /usr/5lib/profile, depending).  This will turn job
control on for login shells.

***************************************

I have tested the shells on the following systems:

		Vax (BRL Unix) |  Pyramid  | 3B2 (S5R2)  | 3B20A (S5R2)
BSD shell		x	     x
S5R2 shell		x	     x		x		x
BRL/S5R2 shell		x	     x		x

As should be clear from the above table, I have only had access to four
different kinds of machines.  If you are running on some other kind of
hardware, and/or another flavor of Unix (V7, Xenix, Perkin Elmer, whatever),
and you successfully add these mods to the shell, please let me know.  Also
send me any diff listings you may have had to generate.  I am particularly
interested to know if it will still fit on a PDP-11.

***************************************

Please don't send me any flames to the effect "You should use the C-shell".
Here is a case where I can have my cake and eat it too, and this should be
a big win for people who only have System V and don't have the ksh.

I hope that these modifications to the shell help to meet a need out there
in the real world.  If you find any bugs, please let me know.

Arnold Robbins
CSNET:	arnold@gatech	ARPA:	arnold%gatech.csnet@csnet-relay.arpa
UUCP:	{ akgua, allegra, hplabs, ihnp4, seismo, ut-sally }!gatech!arnold

School of Information and Computer Science
Georgia Institute of Technology
225 North Avenue, N.W.
Atlanta, Georgia  30332
(404) 894-3658

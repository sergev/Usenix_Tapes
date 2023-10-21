.ds Fi \\u\\s-4\\n+*\\s+4\\d
.TL
Installing and Operating RFS
.AU
T. Brunhoff
toddb%crl@tektronix.csnet
decvax!tektronix!crl!toddb
.AI
University of Denver
.AB
This document describes
the installation, maintenance and adjustment
of RFS, a public domain, kernel\-resident distributed file system,
written at
Tektronix Computer Research Laboratories* for partial
fulfillment of the master's degree program at the University of Denver.
It was designed to provide complete transparency with
respect to file access and protections for all programs
whether they use local or remote files and directories.
It has been installed on VAX BSD 4.2 and 4.3 UNIX,
Pyramid 4.2/5.0 UNIX, version 2.5,
and on a Tektronix internal proprietary workstation,
called Magnolia.
The instructions are designed in a way that keeps all changes
separate from your standard sources, in hope that
it will encourage sites to try the installation.
.AE
.nr * 0 1
.FS *
RFS should not be confused with another completely different (but excellent)
implementation
from Tektronix available on the 6000 series workstation, called DFS,
and done by a separate product group.
The work on RFS was designed and written
strictly by the author of this paper at about the same
time as DFS, and draws none of its implementation details from DFS.
RFS is public domain, while DFS is proprietary.
.FE
.NH 1
Installing and Compiling
.PP
These instructions assume that you are familiar with
the C compiler, the commands \fImake(1)\fP, \fIvi\fP,
the structure of the kernel source files,
how to build a new kernel[1],
the structure of \fI/etc/passwd, /etc/hosts, /etc/services, /etc/rc.local\fP.
Installing and compiling the software should take about two to four hours
without interruption and on a lightly loaded 11/750.
.PP
The best way to install this at first, is to make everything in someone's
login directory, keeping all changes from being anywhere near permanent.
In this way
you can gain confidence comfortably that the software really
works.
All compiles can be done by an unprivilaged user that has read access
to the kernel source.
.PP
To do this,
you need the \fIremote\fP directory
(this document came from \fIremote/doc/install.ms\fP)
with all of its original contents.
Let us say that you have allocated this project to a user named \fIrfs\fP,
and that he will be installing everything beneath his login directory,
\fI~rfs\fP.
If you are reading this document,
then you probably now have the following directories
under \fI~rfs\fP:
.DS
remote
remote/RCS
remote/doc
remote/doc/paper
remote/usr.include.{VAX4.2,VAX4.3,PYR2.5}
remote/usr.src.lib.libc
remote/usr.src.lib.libc/gen
remote/usr.src.lib.libc/{vax,pyr}
remote/usr.src.lib.libc/{vax,pyr}/sys
remote/usr.sys.VAX4.2/{conf,h,machine,sys}
remote/usr.sys.VAX4.3/{conf,h,machine,sys}
remote/usr.sys.PYR2.5/{conf,h,sys}
remote/usr.sys.remote
.DE
.PP
If not,
they are easily obtained by mounting the tape (if that is the medium of this
release) or by placing the \fIshar\fP file in, say,
\fI/tmp/rfs.shar\fP.
To extract them, you would type, respectively,
.DS
% cd ~rfs; tar x
	or
% cd ~rfs; sh < /tmp/rfs.shar
.DE
.NH 2
Building the Kernel
.PP
A new kernel must be installed on the machine that is to have the
distributed file system capability.
The machines that will be accessed through RFS
do not need to have a new kernel if they are running
BSD 4.3;
however, BSD 4.2 sites have a serious bug in sys_socket.c that
causes infinite loops if there is any activity on a socket being
read using the \fIrecv(2)\fP system call.
The server uses this system call.
.PP
First,
you must set up a local copy of the kernel sources
in \fI~rfs/sys\fP
using symbolic links for most,
and copying and changing others (note that \fI~rfs/sys\fP does not
exist yet, so be patient).
The files new to the kernel are:
.DS
~rfs/sys/remote/remotefs.h
~rfs/sys/remote/rmt_data_template	(used to build rmt_data.c)
~rfs/sys/remote/remote_mkdata	(used to build rmt_data.c)
~rfs/sys/remote/rmt_exec.c
~rfs/sys/remote/rmt_final.c
~rfs/sys/remote/rmt_general.c
~rfs/sys/remote/rmt_generic.c
~rfs/sys/remote/rmt_io.c
~rfs/sys/remote/rmt_subr.c
~rfs/sys/remote/rmt_syscall1.c
~rfs/sys/remote/rmt_syscall2.c
~rfs/sys/remote/rmt_syscall3.c
.DE
The files that must be changed in the kernel are:
.DS
~rfs/sys/h/errno.h
~rfs/sys/h/param.h
~rfs/sys/h/user.h
~rfs/sys/machine/trap.c
~rfs/sys/sys/init_sysent.c
~rfs/sys/sys/kern_exec.c
~rfs/sys/sys/kern_exit.c
~rfs/sys/sys/sys_inode.c		(a semantic bug; fixed in 4.3)
~rfs/sys/sys/ufs_nami.c		(ufs_namei.c in 4.3)
~rfs/sys/sys/ufs_syscalls.c
~rfs/sys/sys/uipc_socket.c	(the \fIrecv(2)\fP bug; fixed in 4.3)
.DE
For configuration, there are a few files which must be changed
slightly:
.DS
~rfs/sys/conf/files
~rfs/sys/conf/makefile.vax	(Makefile.vax in 4.3)
.DE
Finally,
the source files for building the new system calls (see 
\fBBuilding the User Programs\fP)
must know about the new system call numbers.
This requires a
(local) change to a file normally found in \fI/usr/include\fP:
.DS
~rfs/usr.include/syscalls.h
.DE
.NH 3
Editing in the Changes (about 15 minutes)
.PP
All of these files, or the changes that need to be made to them
are found in files of the same name (plus a \fI.diff\fP for the
patch files) under \fI~rfs/remote/usr.*\fP.
A shell program named \fI~rfs/remote/shells/rfs_setup\fP
has been provided for setting up the symbolic links,
copying files,
and patching in all the changes to the kernel using the \fBpatch\fP
utility which has been posted publicly to newsgroups such as \fInet.sources\fP
by Larry Wall\*(Fi.
Some of the added kernel
software contains \fIifdef\fPs for 4.3 BSD differences
which only clutter the code and cause problems
when you run \fImake depend\fP on BSD 4.2.
\fI~Rfs_setup\fP will remove these.
.FS
If you have never heard of the \fBpatch\fP program, I don't know how you ever
found out about RFS.
Send me a note and I will mail the source to you.
.FE
Running \fIremote/shells/rfs_setup\fP will work for 4.2 and 4.3 BSD Vax systems,
and for Pyramid 90x systems running 2.5,
but for other systems, you must examine the shell
script to infer what it accomplishes.
Let me emphasize again that you are not (and should not be)
editing the standard kernel sources.
.DS
% cd ~rfs
% ./remote/shells/rfs_setup	# several questions will be asked
.DE
.NH 3
Ensuring that RFS Software Does Not Interfere with the Kernel
.PP
There are four variables defined in \fI~rfs/sys/remote/remotefs.h\fP
which must be examined to be sure that they fit in correctly with
the rest of the kernel.
Their values in the distributed software have been chosen
so as not to conflict
with VAX BSD 4.2 or 4.3, and Pyramid 2.5 software.
It will have to be changed for other Unix versions.
.IP FREMOTE
is a flag that must not conflict with any other flags associated
with the kernel \fIfile\fP structure.
Check in \fI/usr/sys/h/file.h\fP.
.IP DTYPE_REMOTE
should not conflict with DTYPE_INODE or DTYPE_SOCKET in
\fI/usr/sys/h/file.h\fP.
This new file descriptor type means the
fix to \fIino_close()\fP in \fI~rfs/sys/sys/sys_inode.c\fP must be installed.
.IP SREMOTE
marks a process as having used a remote file system at some time.
It must be a unique bit for the \fIproc\fP structure element \fIp_flag\fP
as defined in \fI/usr/sys/h/proc.h\fP.
.IP SNOREMOTE
must also be a unique bit for the \fIproc\fP structure element \fIp_flag\fP.
This flag bit prevents any processes having it from using any remote
file system.
It is used primarily by \fIrfs_server\fP to prevent ``hopping''
across more than one machine with the remote file system.
.NH 3
Setting up VAX\*(Fi Configuration (about 10 minutes)
.PP
.FS
Pyramid software does not have a configuration file.
configuration is done with command\-line shell script arguments.
.FE
The configuration file for VAX machines should now be in \fI~/rfs/sys/conf\fP,
but it is a symbolic link to the real one in \fI/usr/sys/conf\fP,
let us say, FOOVAX.
You should remove the link and copy it to this directory so
that you can add three lines to it\*(Fi:
.FS
The RFSDEBUG option gives you the ability to 
enable selective portions of the RFS kernel to give out
information about activity and problems.
See the section on debugging.
Compiling it in has a negligible performance impact (<< %1)
if it is not used.
You can compile it out later by removing \fIrmt*.o\fP,
removing the RFSDEBUG option line from the config file,
running config again, and recompiling.
.FE
.DS
options		REMOTEFS	# to compile in REMOTEFS changes
options		RFSDEBUG	# to compile in debug software (optional)
pseudo-device	remotefs		# to include the rfs software in the makefile

	so

% cd ~rfs/sys/conf
% rm FOOVAX
% cp /usr/sys/conf/FOOVAX .
% vi FOOVAX
< add in the three new lines >
% mkdir ../FOOVAX			# for the config directory
% config FOOVAX
.DE
.NH 3
Compiling the RFS Kernel (about 90-120 minutes)
.PP
If you have installed everything correctly
and if I have told you all that I should,
then you should now be able to compile the kernel.
Note that compilation is different for VAX and Pyramid.
.DS
	VAX
% cd ~rfs/sys/FOOVAX
% make depend
% make

	PYRAMID
% mkdir ~rfs/sys/FOOVAX	# not made yet
% cd ~rfs/sys/FOOVAX
% cp ../conf/* .		# copy in configuration scripts and makefiles
% makesys 64 VER=01 N	# you better check with your pyramid rep.
.DE
You should boot the kernel to assure yourself that
it won't crash and to prepare for the remaining software.
.NH 2
Building the User Programs
.PP
\fIRmtmnt\fP is to a remote file system what \fI/etc/mount\fP is
to a disk file system.
In addition,
it provides statistics on currently active remote file systems.
\fIRfs_server\fP
provides name translation to the kernel for pathnames
below a generic mount point, e.g. \fI/net/foovax/etc/passwd\fP,
and it is the system call server which a client talks to.
.PP
There are three new system calls added to the kernel,
and there are three new error numbers defined in \fI<errno.h>\fP.
The system calls
\fIremoteon(2)\fP and \fIremoteoff(2)\fP turn on and
off the remote file system,
and \fIremotename(2)\fP allows the
server and kernel to talk to each other.
Hence, you must first make a new libc,
so that \fIrfs_server\fP and \fIrmtmnt\fP compile and link
without any undefined symbols,
and any new programs that get relinked will be able to print
a reasonable message using the standard library call, \fIperror(3)\fP.
.NH 3
Building a new \fIlibc.a\fP (about 10 minutes)
.PP
The new system calls and the new error messages for \fI/lib/libc.a\fP are:
.DS
~rfs/remote/usr.src.lib.libc/gen/errlst.c.diff
~rfs/remote/usr.src.lib.libc/{vax,pyr}/sys/remoteon.c
~rfs/remote/usr.src.lib.libc/{vax,pyr}/sys/remoteoff.c
~rfs/remote/usr.src.lib.libc/{vax,pyr}/sys/remotename.c
.DE
The contents of \fIerrlst.c.diff\fP is a patch file for
\fI/usr/src/lib/libc/gen/errlst.c\fP\*(Fi.
.FS
The \fIMakefile\fP assumes that general source is in \fI/usr/src\fP.
This can be overridden with an assignment on the command line:
.sp 1
	# make vax SRC=/usr/othersrcdir
.sp 1
.FE
The file \fI~rfs/remote/usr.src.lib.libc/Makefile\fP
knows how to build a C\-library from the sources and patch files provided.
By default, the \fIMakefile\fP will install the new libc in
\fI/lib/libc.a\fP\*(Fi so you must be root.
.FS
You can install it in \fI~rfs/remote\fP
by changing the \fIMakefile\fP variable LIBDIR,
or by assigning it on the command line
.sp 1
	# make vax LIBDIR=..
.sp 1
but then you must remember to change the \fIMakefile\fP
in ~rfs/remote
after it is generated by \fI~rfs/remote/shells/makemake\fP.
.FE
.DS
% su root
Password:
# cd ~rfs/remote/usr.src.lib.libc
# make vax	# for VAX BSD 4.2 or 4.3
	or
# make pyr	# for Pyramid 2.5
.DE
.NH 3
Building a Makefile for \fIRfs_server\fP, and \fIRmtmnt\fP (about 5 minutes)
.PP
There are a variety of makefiles for building the server, \fIrfs_server\fP,
and each is built with a command called \fIshells/makemake\fP.
Currently there is support for a makefile
on a Vax that will run the RFS kernel, for
a Pyramid that will run the RFS kernel and
for a Vax that will only be a server (running an ordinary kernel).
Each is created\*(Fi
.FS
The last command run will leave that kind of makefile in \fI./remote\fP.
.FE
with one of the following commands\*(Fi:
.FS
If none of these fit your needs,
you must design your own from the files \fImake.base*\fP.
An important part of the makefile is the byte\-ordering
for the machine on which the server will run.
If your architecture is neither Vax nor Pyramid,
you should compile the program \fI~rfs/remote/byteorder.c\fP
and run it to get its opinion.
.FE
.DS
% cd ~rfs/remote
% shells/makemake vax		# Vax with RFS kernel
	or
% shells/makemake vaxnorfs	# Vax without RFS kernel
	or
% shells/makemake pyramid	# Pyramid with RFS kernel
.DE
.NH 3
Compiling \fIRmtmnt\fP and \fIRfs_server\fP (about 30 minutes)
.PP
Now, compile the server and \fIrmtmnt\fP program\*(Fi,
installing the binaries as root:
.FS
If you put the new libc.a in ~rfs/remote, instead
of following the reccomended procedure,
you should edit the makefile at this point.
.FE
.DS
% cd ~rfs/remote
% make
% su root
Password:
# make install
.DE
.NH 2
Other Chores
.PP
At this point,
you should have an RFS kernel running on one or more machines,
call them \fIclient\fPs, and the
potential to run the RFS server on one or more machines,
call them \fIserver\fPs\*(Fi.
.FS
Any machine can be both client and server,
even with respect to itself.
.FE
Before starting up any connections,
there are a few extra items that should be tended to.
.IP \(bu .125i
Add the service \fIremotefs\fP to \fI/etc/services\fP,
on all server and client machines.
It should be a privileged port number, i.e. less than 1024,
because the server runs as root and does a \fIseteuid(2)\fP
to whatever user it thinks it should be.
I have used port 556 on the machines where I installed RFS.
.IP \(bu .125i
Add the user \fIguest\fP to \fI/etc/passwd\fP on
each server machine.
The default permissions for any access to a server machine
are mapped to guest (so it shouldn't have wide access).
The server process will refuse to run without this user id installed.
.IP \(bu .125i
If you are on a Pyramid,
there is no easy way to turn on the debugging software in the kernel,
so there is a program provided;
you should compile it now hand have it ready.
More on debugging later.
.DS
% cd ~rfs/remote
% make debug
.DE
.NH 2
Starting up the software
.PP
The server should be started as root.
For now, you start it by hand,
and then arrange for it to be started automatically.
This should be done on all machines that will be servers or
clients.
Remember that \fI/etc/rfs_server\fP is slightly different for
a machine running the RFS kernel; see section \fB1.2.2\fP.
.DS
% su root
Password:
# /etc/rfs_server
# vi /etc/rc.local
< insert at an appropriate place, the lines... >
	if [ -f /etc/rfs_server ]; then
		/etc/rfs_server & echo -n ' rfs_server'	>/dev/console
	fi
.DE
.PP
Next we should set up for loop\-back RFS access just to make
sure that everything works.
Note that the mount point is a file,
not a directory.
This is very important.
The design decisions are discussed in a separate paper
under \fI~rfs/remote/doc/paper\fP.
In a nutshell,
using a file for a mount point
allows the natural syntax for file access
.DS
% cp /foovax/etc/passwd /feevax/tmp/x
.DE
but prevents programs like
.DS
% find / -print
.DE
from finding every file on every remote host.
.PP
If your host name is foovax, then create the file /foovax and
mount your host.
The use of the file /foovax is not required,
but only suggested as a convention\*(Fi.
Foovax must be defined in \fI/etc/hosts\fP and \fIrfs_server\fP
should be running.
.FS
It has been pointed out to me that this convention
is slightly restrictive for machines whose owners choose
interesting names such as bin and tmp.
.FE
.DS
% touch /foovax
% /etc/rmtmnt foovax /foovax
.DE
In all likelyhood,
you should not have crashed by this time,
but if there are any problems with the way you installed the software,
they will be encountered now.
Try the commands
.DS
% ls -ld /foovax/
% ls -ld /foovax/etc
% ls -l /foovax/etc
% cd /foovax/etc
% pwd
% ls
.DE
If the machine crashes
or the result is not what you expected,
you should pull out the stops and start debugging.
See the section on debugging.
If all goes well,
then try mounting another host, say foo2vax:
.DS
% touch /foo2vax
% /etc/rmtmnt foo2vax /foo2vax
% ls -l /foo2vax/etc
% ....
.DE
Try a generic mount point:
.DS
% touch /net
% /etc/rmtmnt -g /net
% more /net/anotherhost/etc/passwd
.DE
.PP
Finally,
when you are satisfied that everything is working,
you should install the mount commands in \fI/etc/rc.local\fP.
Note that you don't really need the generic mount point
unless you have many workstations that you don't want to explicitly
mount.
.DS
% su root
Password:
# /etc/rfs_server
# vi /etc/rc.local
< insert at an appropriate place, the lines... >
	if [ -f /foovax ]; then
		/etc/rmtmnt foovax /foovax
	fi
	if [ -f /foo2vax ]; then
		/etc/rmtmnt foovax /foovax
	fi
	if [ -f /net ]; then
		/etc/rmtmnt -v /net
	fi
.DE
.NH 1
General Portability
.PP
I consider RFS to be ``installable'', not necessarily portable.
.PP
RFS's server, \fIrfs_server\fP,
depends very heavily on the BSD signal mechanism,
inheirited file descriptors (sockets to the client),
and the \fIrecv(2)\fP system call using the MSG_PEEK flag.
.PP
The RFS kernel depends very heavily on mbuf structures, and the BSD
socket paradigm.
.NH 1
Access Permissions over RFS
.PP
For a greater understanding of this discussion,
and the terms ``sentry server'', ``gateway server'',
I suggest that the reader familiarize himself
with the \fIrfs_server(8)\fP man page.
The terms ``client'' and ``server'' are described in section \fI1.3\fP.
.PP
A permission map is compiled by \fIrfs_server\fP
when it starts up
based on \fI/etc/hosts\fP, \fI/etc/passwd\fP and \fI.rhosts\fP files
under every user's login directory.
In addition,
whenever a client does a \fIrmtmnt\fP command,
that command sends its \fI/etc/passwd\fP file to the sentry server.
Similarly,
if a server receives a call from a client,
but has not received an \fI/etc/passwd\fP file,
the server calls the client's server,
and asks for it\*(Fi.
.FS
If this fails,
then the server will return EIO
on all \fIread(2)\fP calls where the file descriptor
is to a directory,
because it doesn't know the client architecture's byte order.
.FE
.PP
When a server receives a message from a client process
whose uid number is \fIn\fP,
it consults the client's \fI/etc/passwd\fP file.
If it finds a matching uid number,
then it checks\*(Fi to see if
that uid name is allowed login privileges in some
user's \fI.rhosts\fP file on the server.
If a user allows it,
then the server for that process sets
the effective user id
to that user's uid number (with \fIseteuid(2)\fP)
and sets the groups associated with that
user (with \fIsetgroups(2)\fP).
.FS
Most of this checking is done when \fIrfs_server\fP first starts up,
and is kept in LRU lists for fast access.
Mappings from client uid to server uid are already done
by the time a client makes a connection.
.FE
.PP
If more than one user on the server host
allows login access to that client's user,
then the last user in the server's \fI/etc/passwd\fP takes precedence.
However,
if the one of the users on the server hosts has the same uid name
as that on the client,
that mapping takes precedence over all other mappings.
Note that this means the user \fIx\fP on may have remote
login privileges for users \fIy\fP and \fIz\fP,
but his access permissions over RFS will be for one or the other,
never both.
.PP
If a user changes his \fI.rhosts\fP file on a server,
that change is not noticed until the server is restarted.
Fortunately,
restarting the server is simple:
just run \fIrfs_server\fP again.
As long as the primary log \fI/usr/tmp/rfs_log\fP
still exists which contains the process id number of the running server,
it is intelligent enough to shutdown the old server.
.PP
If the server host on which the user wanted to change his \fI.rhosts\fP file
is currently connected to the client,
that connection must be severed and a new one started.
This can be done by killing all processes
having the SREMOTE bit set\(*Fi on the client machine.
The \fI/etc/rmtmnt\fP command with no arguments will tell
you how many processes are connected to each server.
.FS
The command \fIps axl\fP will show all commands and their per\-process
flag bits.
simply examine the left\-most field of the \fIps\fP output
looking for the SREMOTE bit.
Unfortunately,
multiple remote systems may be open,
and \fIps\fP output does not show which process is using which remote hosts
or even if it is using multiple hosts.
.FE
.PP
Similarly,
if a new user is added to a client's \fI/etc/passwd\fP file,
or an existing user's uid number changes on a client,
the \fIrfs_server\fP must be started again on each of the server machines,
or the remote file systems can be unmounted and mounted again using
\fI/etc/rmtmnt\fP.
Again, however,
if the client host where the change was made is currently connected
to a server,
then that connection must be severed and a new one started
as described above.
.NH 1
Debugging
.PP
All kernel debugging is accomplished by setting the
global variable \fIremote_debug\fP.
The bit definitions are briefly explained in the
file \fI~rfs/sys/remote/remotefs.h\fP.
.PP
Debugging on the server is explained briefly in the
man page for \fIrfs_server(8)\fP.
.NH 2
Places to Look if RFS Does Not Work
.PP
If the kernel gets hung in startup or does not seem to do
some of the system calls correctly
check the code inserted into \fI~rfs/sys/machine/trap.c\fP for
one of the following problems:
.IP \(bu .125i
On the Vax,
the declarations at the top of the routine are very position
dependent.
RFS has some register declarations that must appear last
or the register allocation that the code depends on will be
messed up.
.IP \(bu .125i
On the Pyramid,
system calls are made by passing the arguments as arguments
(the Vax copies them to an array).
Check to make sure that the code inserted is doing this right,
because I only tested it for a week.
.PP
On the Pyramid,
declarations of variables inside of local blocks
sometimes causes the value of other local variables in outer blocks to
be corrupted.
For example:
.DS
	routinex(arg1, ...)
	{
		register long	a, b, c, ...;	/* lots of variables */
		...
		x = y;

		{ /* start a local block */
			register long	z;

			z = 1;
		}
	}
.DE
The declaration and use of z
will probably clobber one of the earlier variables \fIa, b, c, ...\fP.
This is a pretty bad compiler bug, but I think they are aware of it.
This kind of code was completely removed at one time from the
RFS source code, but may have crept back in;
be sure that you don't add any.
.PP
If you \fIcd\fP to a remote directory,
but it always appears as if you are still in the root directory for
that machine,
it almost certainly has to do with \fIfork(2)\fP or \fIvfork(2)\fP
system calls not being sent to the server.
Check the generated table at the end of 
\fI~rfs/sys/remote/rmt_data.c\fP to ensure that there is a \fIRFS_\fP
entry for both \fIfork(2)\fP and \fIvfork(2)\fP.
If there isn't, change it by hand or try to fix the shell file
that generates it in \fI~rfs/sys/remote/remote_mkdata\fP.
.NH 1
Making the Software Permanent
.PP
First,
read in the distribution tape again, this
time putting it in \fI/usr/src/etc\fP;
this will be the proper resting place for the software that
lives in the directory immediately below the first\-level directory,
\fIremote\fP,
because the two commands \fI/etc/rfs_server\fP and \fI/etc/rmtmnt\fP
reside there.
.PP
Second,
rerun the installation procedure,
this time instructing \fIrfs_setup\fP to make the installation permanent,
instead of in the current directory.
The only exception is that the software in \fI~rfs/remote/usr.src.lib.libc\fP
must be copied by hand to /usr/src/lib/libc and placed in
the proper directories there, modifying the \fImakefile\fP accordingly.
.NH 1
Current Bugs and Needed Extensions
.PP
The bugs listed have only proved irritating for our environment,
but worthy of note, none the less.
.IP \(bu .125i
When a client fails to connect to a server,
the internal count of how many processes there are sometimes
goes awry,
and the kernel (as seen by \fIrmtmnt\fP)
will insist that it is still closing
waiting for processes to die that aren't there.
The bug is almost certainly in \fIremote_getconnection()\fP.
.IP \(bu .125i
Whenever a connection goes down,
there may be processes still up ``using'' that connection.
However,
if there are no open files or remote \fIchdir(2)\fP's,
the kernel could just as well restart the connection
without waiting for those processes to die.
This fix would go into \fIremote_getconnection()\fP.
.IP \(bu .125i
The server has the beginnings of capability to
do an asynchronous system call (like reading 10k of data),
but it has not been fully implemented.
A much better solution would be to implement an
atomic \fIwrite(2)\fP system call where
the writer is guarenteed that no one else
will be able to write on the same file descriptor (socket,
in this case)
until he is done.
This would allow servers to perform system calls
at their own pace sending the results back at any time.
.IP \(bu .125i
Along the same lines,
implementing an atomic message reader system call
that would allow all servers to be blocked on a read
of the single socket connected to the client.
The reading server would provide a list of process id's
that he is handling as input arguments along with
file descriptor, buffer and size.
The kernel would slice up all the messages, handing
each out to the server that is serving that process.
The only reason it wasn't implemented so far, was to keep the
server standard.
.IP \(bu .125i
The server maps two user id names with the same
number on a client to one user id on a server.
The reason is that only the uid number is sent in the messages
to the server.
There is no real fix for this.
.IP \(bu .125i
The sentry server has the information about every host in
its memory and hands all this information off to each \fIgateway\fP
server and its children even though it is never used.
If this was freed when each \fIgateway\fP
server starts up, the servers would consume fewer memory
and swap resources.
.IP \(bu .125i
The server currently does not handle \fI.rhosts\fP
files that have entries uttering only a host name.
It also ignores \fI/etc/hosts.equiv\fP.
.IP \(bu .125i
This implementation ignores the possibility that the kernel COMPAT
option may have been used.
Old system calls should just fail.
.IP \(bu .125i
The \fIpwd\fP command (and library call) does not work
while your current directory is under \fI/net/host/somedirectory\fP.
Basically, the \fIpwd\fP command discovers that its current directory
is \fI/net/somedirectory\fP
because the \fIhost\fP is really only a virtual pathname component.
But when it tries to do
a \fIchdir(2)\fP back to it, it fails.
This is an irritating problem,
but the only easy solution is for a change in the implementation,
such that \fI/net\fP becomes a directory, and the nameserver
creates files and
does an explicit mount for any hosts that the kernel asks about.
So far I have resisted this because it requires another change
to the kernel routine \fInamei()\fP,
and because \fIpwd\fP works just fine if a host explicitly mounted.
.IP \(bu .125i
I think that RFS may be responsible for dropping an mbuf when a connection
fails, but this is apparently infrequent.
.IP \(bu .125i
\fIChroot(2)\fP system call is only partially implemented.
and should not be used with a remote directory as an argument.
.IP \(bu .125i
The \fIselect(2)\fP and \fIioctl(2)\fP system calls are not yet implemented.
The latter means that the \fItip\fP command won't work with remote
pointers to tty dialers.
.IP \(bu .125i
The server allows any unknown user on a client to see
its file system through the guest
account.
This is a good default.
But an appropriate extension
would be a table describing a restricted domain of access,
so that if machines \fIx\fP and \fIy\fP appear in the table,
they are allowed access through the normal permission scheme
described above.
However, any machine not in the table would be denied access in general,
so that only users on the client that have been allowed remote logins
via \fI.rhosts\fP files can have access.
.IP \(bu .125i
No fixes have been provided for \fImv(1)\fP or \fIcp(1)\fP
to distinguish the triple device/inode/host for uniqueness.
The information is available:
for a local file,
the \fIstat(2)\fP, \fIfstat(2)\fP and \fIlstat(2)\fP
system calls return a stat structure whose
\fIst_spare1\fP element is 0.
If the file is remote,
the \fIst_spare1\fP element contains the value \fIn\fP+1
where \fIn\fP is the number of the remote host mount point,
as shown by \fI/etc/rmtmnt\fP.
.IP \(bu .125i
The server, \fIrfs_server\fP, does not correctly handle situations where
a host has more than one internet address.
What happens is that all of the user access privileges
(distilled from all the .rhosts files)
gets attached to one address, and when a call is made using the
other address, everyone only has the access of the \fIguest\fP user.
.IP \(bu .125i
Some user commands,
like \fIchgrp(1)\fP and \fIrn(1)\fP
try to guess ahead of time whether the kernel will allow you write
permission on a file.
Hence, they will fail sometimes where they should not.
For example,
if your user id number on one host is 5 and on
another host it is 6,
\fIchgrp(1)\fP on one host
will tell you that you cannot change the group of a file that resides on
another host, even though RFS would allow it.
.NH 1
Conclusions and Plans for the Future
.PP
RFS is not production quality,
but is very useful,
providing reasonably fast file access:
I consider rcp unreasonable.
I have made this software publicly available in hopes that other
interested parties might contribute some of the fixes to it.
I have other responsibilities and cannot afford to spend the
many hours necessary to get that last 5 or 10% of quality,
but perhaps you can help.
.PP
The only hope for this software is that it is free and easy
to install and debug.
I am more than willing to accept bug reports and fixes for it
until this or something else becomes part of the Berkeley Software
Distribution.

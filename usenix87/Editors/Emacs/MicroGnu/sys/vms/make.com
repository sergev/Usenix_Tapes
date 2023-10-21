$	on error then goto trouble
$	on severe_error then goto trouble
$	default = f$trnlnm("SYS$DISK") + f$directory()
$!
$! Command procedure to build MicroGnuEmacs on VMS systems.
$!
$! Compile-time options you can set by appropriate assignments to
$! "ccomflags" and "linkflags".  Defining these flags asks for
$! a particular feature.
$!
$!	/DEFINE:"STARTUP"		-- look for SYS$LOGIN:.MG startup file
$!	/DEFINE:"FLOWCONTROL"		-- use ^S, ^Q for flow control
$!
$! Set compilation and linking options.  The first commented-out-line is
$! the set I use...
$	ccomflags := "/define:""STARTUP"" "
$!	ccomflags := "/define:(""STARTUP"",""XKEYS"",""PREFIXREGION"") "
$!	ccomflags := "/debug"	! if you want to debug the program
$!
$	linkflags := ""
$!	linkflags := "/debug"	! to debug the program
$!
$! To make MG,
$!
$! Set def to the top level MicroGnuEmacs directory and type
$!
$!	@[.SYS.VMS]MAKE
$!
$! to get things rolling.
$!
$!* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
$! Create the termcap library
$!
$	set def [.sys.vms.termcap]
$	@createlib.com
$	set def [---]
$!* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
$! Define a search path for source files.  This is important
$! because 
$!	1) The search path lets us to keep these files in
$!	   separate directories
$!	2) When searching for quoted #include files (e.g.
$!	   #include "def.h"), VAX C uses the default file
$!	   specification set up by the source file name.
$!
$!	   If we use mgsrc:foo.c,  the search list becomes
$!	   part of the default file spec, and the compiler
$!	   can find the system- and terminal-specific
$!	   header files.  This acts as a substitute for the
$!	   -I flag found on most Unix C compilers.
$!
$! A side effect of the search list is that the object files get
$! created in the top level directory, which I prefer.
$!
$	define mgsrc [],[.sys.vms],[.tty.termcap]
$!
$! Define alias for the compilation command.  By default, use a
$! command file that checks revision dates and only compiles when
$! it has to.  If you want to force a total recompile,  switch the
$! comments around.
$!
$	ccom := @mgsrc:ccom
$!	ccom := cc
$!
$! Compile all the basic files
$!
$	ccom mgsrc:basic	'ccomflags
$	ccom mgsrc:buffer	'ccomflags
$	ccom mgsrc:cinfo	'ccomflags
$	ccom mgsrc:display	'ccomflags
$	ccom mgsrc:echo		'ccomflags
$	ccom mgsrc:extend	'ccomflags
$	ccom mgsrc:file		'ccomflags
$	ccom mgsrc:kbd		'ccomflags
$	ccom mgsrc:line		'ccomflags
$	ccom mgsrc:main		'ccomflags
$	ccom mgsrc:match	'ccomflags
$	ccom mgsrc:paragraph	'ccomflags
$	ccom mgsrc:prefix	'ccomflags
$	ccom mgsrc:random	'ccomflags
$	ccom mgsrc:region	'ccomflags
$	ccom mgsrc:search	'ccomflags
$	ccom mgsrc:symbol	'ccomflags
$	ccom mgsrc:version	'ccomflags
$	ccom mgsrc:window	'ccomflags
$	ccom mgsrc:word		'ccomflags
$!
$! Compile the terminal interface 
$!
$	ccom mgsrc:tty		'ccomflags
$	ccom mgsrc:ttykbd	'ccomflags
$!
$! Compile the VMS-specific files
$!
$	ccom mgsrc:fileio	'ccomflags
$	ccom mgsrc:spawn	'ccomflags
$	ccom mgsrc:trnlnm	'ccomflags
$	ccom mgsrc:ttyio	'ccomflags/define:"FLOWCONTROL=0"
$	macro mgsrc:bcopy
$!
$! Link the program
$!
$	@[.sys.vms]mglink	"''linkflags'"
$!
$! We're done!
$!
$	write sys$output "MicroEmacs build completed."
$	set default 'default
$	exit
$!
$! Trouble somewhere -- go 'way
$!
$trouble:
$	write sys$output "Problem building MicroEmacs!!!!!"
$	set default 'default
$	exit

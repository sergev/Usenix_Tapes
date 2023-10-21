$!
$!	Build the GNU "C" pre-processor on VMS
$!
$ if "''p1'" .eqs. "LINK" then goto Link
$ gcc/debug cccp.c
$ bison cexp.y
$ rename cexp_tab.c cexp.c
$ gcc/debug cexp.c
$ gcc/debug version.c
$ Link:
$ link/exe=gcc-cpp sys$input:/opt
!
!	Linker options file for linking the GNU "C" pre-processor
!
cccp,cexp,version,gnu_cc:[000000]gcclib/lib,sys$share:vaxcrtl/lib
$!
$!	Done
$!
$ exit

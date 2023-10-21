$!
$! Command procedure to compile and create VMS termcap library
$!
$	ccom := @[-]ccom.com		! Checks file dates
$!	ccom := cc			! To force a recompile
$!
$	ccom fgetlr
$	ccom tgetent
$	ccom tgetflag
$	ccom tgetnum
$	ccom tgetstr
$	ccom tgoto
$	ccom tputs
$!
$ library/create/object termcap.olb fgetlr.obj,tgetent.obj,-
tgetflag.obj,tgetnum.obj,tgetstr.obj,tgoto.obj,tputs.obj
$ purge/keep=2 termcap.olb

$!
$! Command procedure to make new VMS termcap library
$!
$ Library/Create/Object Termcap.Olb Fgetlr.Obj,Tgetent.Obj,-
Tgetflag.Obj,Tgetnum.Obj,Tgetstr.Obj,Tgoto.Obj,Tputs.Obj
$ Purge/Keep=2 Termcap.Olb

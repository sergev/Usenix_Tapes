$!
$! Link MicroGnuEmacs object files into MG.EXE
$!
$! Note: This command procedure is designed to be executed
$! in the main microEmacs directory.
$!
$! Further note: If the VAX C shareable run-time library is
$! found in SYS$SHARE, the command procedure uses it.
$! If not found, it uses SYS$LIBRARY:VAXCRTL.
$
$ runtime_library := sys$library:vaxcrtl.olb/lib
$ if f$search("SYS$SHARE:VAXCRTL.EXE") .nes. "" then -
	runtime_library := [.sys.vms]emacs.opt/opt
$ link'p1'/exec=sys$disk:[]mg.exe basic.obj, buffer.obj, cinfo.obj, -
	display.obj, echo.obj, extend.obj, file.obj, kbd.obj, line.obj, -
	main.obj, match.obj, paragraph.obj, prefix.obj, random.obj, -
	region.obj, search.obj, symbol.obj, version.obj, window.obj, word.obj, -
	tty.obj, ttykbd.obj, -
	fileio.obj, spawn.obj, trnlnm.obj, ttyio.obj, bcopy.obj, -
	[.sys.vms.termcap]termcap.olb/lib,-
	'runtime_library'

$	if p1 .eqs. "\"   then  p1 := 'cd_last_dir'
$	if p1 .eqs. ""    then	p1 := 'f$logical("SYS$LOGIN")'
$	len = 'f$length(p1)' 
$	if p1 .eqs. "[-]" then goto updir
$	if 'f$locate("[",p1)' .nes. len then goto setdef
$	if "''f$logical(p1)'" .nes. "" then  goto chdir
$	if p1 .nes. ".."  then goto chkroot
$ updir:
$	curdir := 'f$directory()'
$	if curdir .eqs. "[000000]" then goto nodir
$	p1 := "[-]"
$	goto chdir
$ chkroot:
$	if p1 .nes. "000000" then goto chksub
$	p1 := "[000000]"
$	goto chdir
$ chksub:
$	if 'f$locate(".",p1)' .eqs. len then p1 := .'p1'
$	p1 := "[''p1']"
$ setdef:
$	len = 'f$length(p1)'
$	end = 'f$locate("]",p1)'
$	beg = end - 1
$ findbeg:
$	c := "''f$extract(beg,1,p1)'"
$	if c .eqs. "[" then goto dirbeg1
$	if c .eqs. "." then goto dirbeg2
$	beg = beg - 1
$	goto findbeg
$ dirbeg1:
$	opendir := 'f$extract(0, beg + 1, p1)'"000000"
$	goto testdir
$ dirbeg2:
$	opendir := "''f$extract(0, beg, p1)'"
$ testdir:
$	opendir := 'opendir''f$extract(end, len - end, p1)''f$extract(beg + 1, end - beg - 1, p1)'".dir"
$	open/error=nodir dir 'opendir'
$	close dir
$ chdir:
$	cd_last_dir :== 'f$logical("SYS$DISK")''f$directory()'
$	set default 'p1'
$	set prompt='f$directory()'>
$	if p2 .nes. "" then cd "''p2'" "''p3'" "''p4'"
$	exit
$ nodir:
$	write sys$error "invalid directory"
$	exit

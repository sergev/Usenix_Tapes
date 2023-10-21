$ ! install new version of snake2 - Don Libes
$
$! make it /notraceback so it can be installed
$ trpatch :== $eun_usr:[usr.eun]trpatch trpatch
$ trpatch snake2.
$!
$ pushp sysprv,cmkrnl,cmexe
$ copy snake2. eun_usr:[usr.local.bin]
$ mc install
eun_usr:[usr.local.bin]snake2./replace
$ popp

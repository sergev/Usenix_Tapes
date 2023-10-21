$! install snake2 with sysprv
$! sysprv is necessary to update the score file
$!
$ pushp sysprv,cmkrnl,cmexe
$ mc install
eun_usr:[usr.local.bin]snake2./shared/header/priv=sysprv
$ popp

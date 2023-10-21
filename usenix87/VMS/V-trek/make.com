$cc plot.c
$cc sub1.c
$cc sub2.c
$cc termio.c
$lib/create vtrek plot,sub1,sub2,termio
$cc main
$define lnk$library sys$library:vaxccurse
$define lnk$library_1 sys$library:vaxcrtl
$link main,vtrek/lib
$exit

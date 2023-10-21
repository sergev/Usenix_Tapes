cc86 shar.c large include (/lib/cc86/) print (shar.mp0) &
optimize (2) verbose
copy shar.mp0
link86 lqmain.obj, shar.obj, &
/lib/cc86/lclib.lib, &
/lib/rmx86/large.lib, &
/lib/cc86/lcifl.lib, /lib/rmx86/lpifl.lib, &
/lib/cc86/hcifl.lib, /lib/rmx86/hpifl.lib, &
/lib/cc86/ecifl.lib, /lib/rmx86/epifl.lib, &
/lib/cc86/icifl.lib, /lib/rmx86/ipifl.lib, &
/lib/cc86/rcifl.lib, /lib/rmx86/rpifl.lib, &
/lib/pasc86/87null.lib &
to shar &
bind map fastload segsize (stack(+400h)) &
purge mempool (50000, 0FFFF0H)
copy shar.mp0,shar.mp1 over shar.lst
delete shar.mp?,shar.obj

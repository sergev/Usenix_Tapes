#include "boot.h"

chdir ../misc

if ! -r ../save.h goto ok
echo "You interrupted the last boot"
echo "To prevent loss of the original local.h type:"
echo "	mv ../save.h ../local.h"
exit
: ok

echo "please, do not interrupt"
cp ../local.h ../save.h
cat ../rundir/boot.h >> ../local.h

: "these pc and em1 include boot.h"
: "therefore, they may be executed only in"
: "subdirectories of src"
cc -n -O pc.c
mv a.out pcboot
cc -n -O em1.c
mv a.out em1boot

rm -f ../local.h
mv ../save.h ../local.h
echo "Interrupt allowed"

: "these pc and em1 include local.h"
: "and will be put in /usr/bin eventually"
cc -n -O pc.c
mv a.out pc
cc -n -O em1.c
mv a.out em1

mv pc PCNOBOOT_PATH
mv em1 EM1NOBOOT_PATH
mv pcboot PC_PATH
mv em1boot EM1_PATH

cc -n -O makelib.c
mv a.out LIB_PATH

cc -n -O mktab.c
mv a.out mktab
cat TABLES_PATH | mktab
mv em1.h ..

chdir ../int
EM1_PATH -i

chdir ../opt
sh run
mv opt OPT_PATH

chdir ../ass
sh run
mv ass ASS_PATH

chdir ../misc
: "encode is precompiled, because it needs the libraries,"
: "while it is necessary to construct the libraries"
: "this precompiled version finds its tables in ../../etc/em1:tables"
cp encode.boot ENCI_PATH

chdir ../libpr
ln PC_PATH pc
sh erun
rm pc
mv em1:pr.a EM1PR_PATH

chdir ../libP
ln PC_PATH pc
sh erun
rm pc
mv em1:P.a EM1P_PATH

chdir ../lib2
ln PC_PATH pc
sh erun
rm pc
mv em1:2.a EM12_PATH

chdir ../pem
PC_PATH --t pem.boot.m
mv e.out PEMI_PATH

chdir ../misc
ln decode.p dec.c
cc -P dec.c
mv dec.i dec.p
PC_PATH -I -O --t dec.p EM1P_PATH EM12_PATH
mv e.out DECI_PATH
rm dec.p dec.c

ln encode.p enc.c
cc -P enc.c
mv enc.i enc.p
PC_PATH -I -O --t enc.p EM1P_PATH EM12_PATH
mv e.out ENCINOBOOT_PATH
rm enc.p enc.c

cc -n -O eprof.c
mv a.out PROF_PATH
cc -n -O ecount.c
mv a.out COUNT_PATH
cc -n -O eflow.c
mv a.out FLOW_PATH

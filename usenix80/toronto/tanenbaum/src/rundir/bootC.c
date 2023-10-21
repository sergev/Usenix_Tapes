#include "boot.h"

chdir ../pdp
sh run
mv pdp PDP_PATH

chdir ../libem
sh run
mv emlib.a LIBEM_PATH
mv pc:rt0.o RT0_PATH
mv pc:frt0.o FRT0_PATH
mv pc:bss.o BSS_PATH

chdir ../libpr
ln PC_PATH pc
sh srun
rm pc
mv pc:prlib.a LIBPR_PATH

chdir ../libP
ln PC_PATH pc
sh srun
rm pc
mv libP.a LIBP_PATH

chdir ../lib2
ln PC_PATH pc
sh srun
rm pc
mv lib2.a LIB2_PATH

chdir ../pem
ln PC_PATH pc
ln pem.p p.c
cc -P p.c
mv p.i p.p
pc -I -C -3 p.p
pc -C -i p.o
mv a.out PEM45_PATH
pc -C -n p.o
mv a.out PEM40_PATH
rm p.c p.p p.o pc

chdir ../misc
ln decode.p dec.c
cc -P dec.c
mv dec.i dec.p
PC_PATH -C dec.p LIBP_PATH LIB2_PATH
mv a.out DEC_PATH
rm dec.c dec.p

ln encode.p enc.c
cc -P enc.c
mv enc.i enc.p
PC_PATH -C enc.p LIBP_PATH LIB2_PATH
mv a.out ENC_PATH
rm enc.p enc.c

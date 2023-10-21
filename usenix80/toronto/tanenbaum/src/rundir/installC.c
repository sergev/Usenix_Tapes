#include "../local.h"

chdir ../../ovl
#ifdef SEPARATE_ID
cp	pc:pem45	PEM_PATH
#endif
#ifndef SEPARATE_ID
cp	pc:pem40	PEM_PATH
#endif
cp	pc:pdp		PDP_PATH
cp	pc:encode	ENC_PATH
cp	pc:decode	DEC_PATH

chdir ../lib
cp	pc:rt0.o	RT0_PATH
cp	pc:frt0.o	FRT0_PATH
cp	pc:bss.o	BSS_PATH
cp	pc:emlib.a	LIBEM_PATH
cp	pc:prlib.a	LIBPR_PATH
cp	libP.a		LIBP_PATH
cp	lib2.a		LIB2_PATH

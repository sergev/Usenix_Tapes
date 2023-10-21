#include	"../local.h"

chdir	../../etc
cp	pc:info		INFO_PATH
cp	pc:errors	ERR_PATH
cp	em1:tables	TABLES_PATH

chdir	../bin
cp	pc		PC_PATH
cp	em1		EM1_PATH
cp	eprof		PROF_PATH
cp	ecount		COUNT_PATH
cp	eflow		FLOW_PATH

chdir	../ovl
cp	pc:pem.out	PEMI_PATH
cp	pc:opt		OPT_PATH
cp	pc:encode.out	ENCI_PATH
cp	pc:decode.out	DECI_PATH
cp	pc:makelib	LIB_PATH
cp	pc:ass		ASS_PATH

chdir	../lib
cp	em1:pr.a	EM1PR_PATH
cp	em1:P.a		EM1P_PATH
cp	em1:2.a		EM12_PATH

chdir	../src/int
echo	"	.float	= 0"	>	r-
echo	"	.softfp	= 0"	>>	r-
echo	"	.float	= 1"	>	r+
#ifdef	HARDWARE_FP
echo	"	.softfp	= 0"	>>	r+
#endif
#ifndef	HARDWARE_FP
echo	"	.softfp	= 1"	>>	r+
#endif

mkdir	INT_DIR
cp	em.s		INT_DIR
cp	c+		INT_DIR
cp	c-		INT_DIR
cp	f+		INT_DIR
cp	f-		INT_DIR
cp	p+		INT_DIR
cp	p-		INT_DIR
cp	r+		INT_DIR
cp	r-		INT_DIR
cp	t+		INT_DIR
cp	t-		INT_DIR
cp	x+		INT_DIR
cp	x-		INT_DIR

em1	-i 
em1	-i +r
em1	-i +rx
em1	-i +t
em1	-i +rt
em1	-i +rxt

CC = CC
CCDEBUG =
I = /usr/include/CC
LIB_ID = C
CFLAGS = $(CCDEBUG)
LFLAGS = $(CCDEBUG)

# Target Directories for Installation
OOPSLIBDIR = /usr/lib		# directory for liboops.a
OOPSINCDIR = /usr/include/oops	# directory for OOPS include files
CLIBDIR = /usr/lib		# directory where libC.a resides
ERRDIR = /usr/local/bin		# directory for errgen utility

# Machine-dependent code directories
MACHDEP = libhp320 libmasscomp librt-acis librt-aix libsun3 libvax

# Other directories
CLASSES = src
TEST = test
EX = ex
OTHERDIR = ${CLASSES} ${TEST} ${EX}

all: oopsclasses oopstest oopsexamples

errorfacility:
	cd errfac; \
		make -f MAKEFILE ${MFLAGS} \
			CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}; \
		make -f MAKEFILE ${MFLAGS} install \
			CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID} \
			CLIBDIR=${CLIBDIR} ERRDIR=${ERRDIR}

oopsclasses:
	cd ${CLASSES}; make -f MAKEFILE ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

oopstest:
	cd ${TEST}; make -f MAKEFILE ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

oopsexamples:
	cd ${EX}; make -f MAKEFILE ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

hp320:
	cd libhp320; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

masscomp:
	cd libmasscomp; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

rt-acis:
	cd librt-acis; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

rt-aix:
	cd librt-aix; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

sun3:
	cd libsun3; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

vax:
	cd libvax; make ${MFLAGS} \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID}

verify:
	cd test; verify

install:
	for i in ${OTHERDIR}; do \
		(echo $$i; cd $$i; make -f MAKEFILE ${MFLAGS} install \
		CC=${CC} CCDEBUG=${CCDEBUG} I=${I} LIB_ID=${LIB_ID} \
		OOPSLIBDIR=${OOPSLIBDIR} OOPSINCDIR=${OOPSINCDIR} \
		CLIBDIR=${CLIBDIR} ERRDIR=${ERRDIR} \
		); done
clean:
	for i in errfac ${OTHERDIR} ${MACHDEP}; do \
		(echo $$i; cd $$i; make -f MAKEFILE ${MFLAGS} clean); done
	cd vectest; make clean;
	cd vector; make clean;

newdistribution:
	-mkdir distribution
	find errfac ${OTHERDIR} ${MACHDEP} paper ws vector vectest -depth -print | cpio -pdm distribution
	ls COPYRIGHT INSTALL Makefile README bugs* | cpio -pdm distribution
	(cd distribution; make clean; cd ..)
	(cd distribution/src; chmod u+w *; rm *,v; cd ..)
	(cd distribution/test; chmod u+w *; rm *,v; cd ..)
	(cd distribution/vector; chmod u+w *; rm *,v; cd ..)
	(cd distribution/vectest; chmod u+w *; rm *,v; cd ..)

tape:
	tar cvf /dev/rmt0 -C distribution .

cpiotape:
	find distribution vector vectest -depth -print | cpio -ocv >/dev/rmt0

cartridge:
	tar cvf /dev/rst0 -C distribution .

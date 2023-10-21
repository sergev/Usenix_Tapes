#
# This makefile is present only to build a temporary libc... until
# you decide that the files compiled here should be installed in the
# source directory.  This makefile should not be installed anywhere.
#
OBJS=	remotename.o remoteon.o remoteoff.o errlst.o
SRC=/usr/src
LIBSRC=${SRC}/lib/libc
USR_INCLUDE=../../usr.include
LIBDIR=/lib

nothing:
	echo "Nothing made!"
	echo "You must type 'make vax' or 'make pyr'"

vax: /tmp
	make	INCLUDE="-I${LIBSRC}/vax/sys -I${USR_INCLUDE}" \
		MACHTYPE=vax \
		SUFFIX=c \
		LIBDIR=${LIBDIR} \
		SRC=${SRC} \
		install

pyr: /tmp
	make	INCLUDE="-I${USR_INCLUDE}" \
		MACHTYPE=pyr \
		SUFFIX=s \
		LIBDIR=${LIBDIR} \
		SRC=${SRC} \
		install

newlibc.a: ${OBJS} libc.a
	cp libc.a newlibc.a
	ar cru newlibc.a ${OBJS}

install: newlibc.a libc.a
	-cp ${LIBDIR}/libc.a ${LIBDIR}/libc.a.bak
	cp newlibc.a ${LIBDIR}/libc.a
	ranlib ${LIBDIR}/libc.a

libc.a:	/lib/libc.a
	cp /lib/libc.a .

errlst.o: errlst.c
remotename.o remoteon.o remoteoff.o:
remotename.o: ${MACHTYPE}/sys/remotename.${SUFFIX}
	/lib/cpp ${INCLUDE} -E $? | ${AS} -o $@
	-ld -x -r $@
	mv a.out $@
remoteon.o: ${MACHTYPE}/sys/remoteon.${SUFFIX}
	/lib/cpp ${INCLUDE} -E $? | ${AS} -o $@
	-ld -x -r $@
	mv a.out $@
remoteoff.o: ${MACHTYPE}/sys/remoteoff.${SUFFIX}
	/lib/cpp ${INCLUDE} -E $? | ${AS} -o $@
	-ld -x -r $@
	mv a.out $@

errlst.c: ${LIBSRC}/gen/errlst.c
	cp ${LIBSRC}/gen/errlst.c nerrlst.c
	patch nerrlst.c gen/${MACHTYPE}.errlst.c.diff
	mv nerrlst.c errlst.c

CFLAGS = -g

OBJS = dca2troff.o do_sfe1.o do_sfe2.o do_sfe3.o do_sfe4.o do_sfe5.o \
	do_sfe6.o do_sfe8.o do_sfe9.o do_text.o do_single.o \
	do_multi.o do_mbd1.o do_mbd2.o do_mbd4.o do_mbd8.o do_mbd9.o \
	misc.o externs.o outachar.o do_accent.o do_spchar.o

dca2troff:	${OBJS}
	${CC} -o dca2troff ${OBJS}

${OBJS}: dca2troff.h

do_text.o: ebtab.h

clean:
	/bin/rm -f ${OBJS} dca2troff

install: dca2troff
	install -s dca2troff /usr/local/bin

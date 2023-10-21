#
# InterViews workstation statistics display
#

I = ../../../..
T = ../../..
M = VAX
X = X10
D = ${M}.${X}

AOUT = a.out
DEST = ${I}/bin/${D}/istat

CC = CC
CFLAGS = -g -I..
LIBS = ${T}/lib/${D}/libInterViews.a
EXTLIBS = -l${X} -lm

LD = CC
LDFLAGS =

OBJS = istat.o stats.o log.o indicator.o

${AOUT}: ${OBJS} ${LIBS}
	${LD} ${LDFLAGS} ${OBJS} ${LIBS} ${EXTLIBS} -o ${AOUT}

install: ${AOUT}
	install -s ${AOUT} ${DEST}

istat.o: ../istat.c
	${CC} ${CFLAGS} -c ../istat.c

stats.o: ../stats.c
	${CC} ${CFLAGS} -c ../stats.c

log.o: ../log.c
	${CC} ${CFLAGS} -c ../log.c

indicator.o: ../indicator.c
	${CC} ${CFLAGS} -c ../indicator.c

clean:
	rm -f a.out core make.out ${OBJS}

depend:
	mkdepend ${CFLAGS} ${OBJS}
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
stats.o: ../InterViews/border.h
stats.o: ../InterViews/box.h
indicator.o istat.o log.o stats.o: ../InterViews/defs.h
istat.o stats.o: ../InterViews/frame.h
stats.o: ../InterViews/glue.h
indicator.o istat.o log.o stats.o: ../InterViews/interactor.h
indicator.o stats.o: ../InterViews/paint.h
indicator.o stats.o: ../InterViews/painter.h
indicator.o istat.o stats.o: ../InterViews/resource.h
istat.o stats.o: ../InterViews/scene.h
istat.o stats.o: ../InterViews/sensor.h
indicator.o stats.o: ../InterViews/shape.h
indicator.o stats.o: ../InterViews/transformer.h
istat.o: ../InterViews/world.h
indicator.o stats.o: ../indicator.h
indicator.o log.o stats.o: ../log.h
stats.o: ../rstat.h
istat.o stats.o: ../stats.h
istat.o: /usr/include/CC/stdio.h
stats.o: /usr/include/netinet/in.h
stats.o: /usr/include/rpc/auth.h
stats.o: /usr/include/rpc/auth_unix.h
stats.o: /usr/include/rpc/clnt.h
stats.o: /usr/include/rpc/rpc.h
stats.o: /usr/include/rpc/rpc_msg.h
stats.o: /usr/include/rpc/svc.h
stats.o: /usr/include/rpc/svc_auth.h
stats.o: /usr/include/rpc/types.h
stats.o: /usr/include/rpc/xdr.h
stats.o: /usr/include/sys/dk.h
stats.o: /usr/include/sys/kmalloc.h
stats.o: /usr/include/sys/time.h
stats.o: /usr/include/sys/types.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

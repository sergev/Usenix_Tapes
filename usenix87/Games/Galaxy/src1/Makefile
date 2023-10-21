# This makefile belongs to ~galaxy/src1.
# %W% (mrdch&amnnon) %E%
#

LIBDIR  = /usr/games/lib/galaxy
GAMES   = /usr/games
ECHO    = /bin/echo
CFLAGS  = -O -s -Dindex=strchr

all:    okgalaxy pager local edscore
	@${ECHO} Utility files are up to date.

okgalaxy: okgalaxy.c
	cc ${CFLAGS} -o okgalaxy okgalaxy.c

pager:  pager.c
	cc ${CFLAGS} -o pager pager.c

local: local.c
	cc ${CFLAGS} -o local local.c

edscore: edscore.c constants.h score.h
	cc $(CFLAGS) -o edscore edscore.c

.DEFAULT:
	sccs get $@

clean:
	/bin/rm okgalaxy pager local edscore

install: okgalaxy pager local edscore
	cp pager local edscore ${LIBDIR}
	chown galaxy ${LIBDIR}/pager
	chown galaxy ${LIBDIR}/local
	chown galaxy ${LIBDIR}/edscore
	chmod 755 ${LIBDIR}/pager
	chmod 755 ${LIBDIR}/local
	chmod 755 ${LIBDIR}/edscore
	cp galaxy.csh ${GAMES}/galaxy
	chown galaxy ${GAMES}/galaxy
	chmod 755 galaxy.csh
	cp okgalaxy ${GAMES}/okgalaxy
	chown galaxy ${GAMES}/okgalaxy
	chmod 4111 ${GAMES}/okgalaxy
	cp galaxy.motd ${LIBDIR}
	chown galaxy ${LIBDIR}/galaxy.motd
	chmod 600 ${LIBDIR}/galaxy.motd

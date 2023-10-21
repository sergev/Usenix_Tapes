#
# Makefile for UNaXcess
#

SHELL = /bin/sh
# make on sys3 is not very nice when it comes to standard shell interfaces...

# ``make SYSTEM=(SYS3|SYS5|V7|BSD|XENIX|XENIX3|XENIX5) all''
# Use V7     for V7, XENIX 2.x, and BSD pre-4.2 (+ bastardizations of same)
#     BSD    for BSD 4.2 and 4.3
#     SYS3   for real AT&T System III
#     SYS5   for real AT&T System V revisions 1 and 2
#     XENIX3 for Xenix v3.x, Unisoft System III, and other bastardized V7's
#     XENIX5 for Xenix V5.x and other bastardized V7's with System V libraries
# You're on your own if it's a really odd combo.
# Pyramid 90x:  use either universe.  SYS5 is best tested; BSD has bigger
# conference names and many performance pluses (extra-fast conference
# creation; I don't know if the 4.2 directory stuff is implemented in kernel
# or not, so I don't know if you gain -- but libndir is SLOW!).

CFLAGS = -D${SYSTEM} -O

OBJS = ua.o msg.o conf.o date.o bull.o sys.o user.o dir.o param.o udl.o
SRCS = ua.c msg.c conf.c date.c bull.c sys.c user.c dir.c param.c udl.c \
       ua.h user.h dir.h sys.h README \
       Utilities/install.sh Utilities/mkconf.c Utilities/uanews.c \
       Utilities/udate.c Utilities/umotd.sh Utilities/upost.sh \
       Utilities/uwho.c
SRC  = ua.c msg.c conf.c date.c bull.c sys.c user.c dir.c param.c udl.c \
       ua.h user.h dir.h sys.h README
UTIL = install.sh mkconf.c uanews.c udate.c umotd.sh upost.sh uwho.c

all: ua uwho udate mkconf mvmsg uanews

uwho: Utilities/uwho
udate: Utilities/udate
mkconf: Utilities/mkconf
mvmsg: Utilities/mvmsg
uanews: Utilities/uanews

ua: $(OBJS)
	${CC} -o ua $(OBJS)

$(OBJS): ua.h user.h dir.h sys.h

Utilities/mkconf: Utilities/mkconf.o
	${CC} Utilities/mkconf.o -o Utilities/mkconf

Utilities/mvmsg: Utilities/mvmsg.o
	${CC} Utilities/mvmsg.o -o Utilities/mvmsg

Utilities/uanews: Utilities/uanews.o
	${CC} Utilities/uanews.o -o Utilities/uanews

Utilities/udate: Utilities/udate.o date.o
	${CC} Utilities/udate.o date.o -o Utilities/udate

Utilities/uwho: Utilities/uwho.o
	${CC} -o Utilities/uwho Utilities/uwho.o

Utilities/mkconf.o: Utilities/mkconf.c
	${CC} ${CFLAGS} -c Utilities/mkconf.c
	mv mkconf.o Utilities/mkconf.o

Utilities/udate.o: Utilities/udate.c
	${CC} ${CFLAGS} -c Utilities/udate.c
	mv udate.o Utilities/udate.o

Utilities/uwho.o: Utilities/uwho.c
	${CC} ${CFLAGS} -c Utilities/uwho.c
	mv uwho.o Utilities/uwho.o

Utilities/mvmsg.o: Utilities/mvmsg.c
	${CC} ${CFLAGS} -c Utilities/mvmsg.c
	mv mvmsg.o Utilities/mvmsg.o

Utilities/uanews.o: Utilities/uanews.c
	${CC} ${CFLAGS} -c Utilities/uanews.c
	mv uanews.o Utilities/uanews.o

clean:
	rm -f ua Utilities/uwho Utilities/udate Utilities/upost \
		Utilities/mkconf Utilities/umotd *.o Utilities/*.o tags core \
		shar? Utilities/mvmsg Utilities/uanews

tags:
	ctags *.[ch] Utilities/*.c

shar:
	for file in $(SRC); do release $$file; done
	cd Utilities; for file in $(UTIL); do release $$file; done
	shar -dv ua.c date.c bull.c sys.c user.c param.c dir.c > shar1
	shar -dv msg.c conf.c ua.h user.h dir.h sys.h README > shar2
	shar -dv udl.c Makefile Utilities/*.c Utilities/*.sh > shar3
	rm $(SRC); cd Utilities; rm $(UTIL)

tar:
	tar cvf unaxcess.tar README *.[ch] Makefile Utilities/*.c \
		Utilities/*.sh

edall: %edall
	for file in *.[ch] Makefile Utilities/*.c Utilities/*.sh; do\
		sh %edall $$file;\
	done
	
update:
	if test -z "$(VERSION)"; then exit 1; fi
	for file in $(SRCS); do \
		admin -fq"$(VERSION)" s.$file \
	done

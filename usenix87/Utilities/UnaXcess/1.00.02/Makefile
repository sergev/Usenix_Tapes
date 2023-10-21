#
# Makefile for UNaXcess Conferencing, version 1.00.02
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
#            (If Xenix 5.x has /etc/inittab, then I messed up; use SYS5.)
# You're on your own if it's a really odd combo.
# Pyramid 90x:  use either universe.  SYS5 is best tested; BSD has bigger
# conference names and many performance pluses (extra-fast conference
# creation; I don't know if the 4.2 directory stuff is implemented in kernel
# or not, so I don't know if you gain -- but libndir is SLOW!).

CFLAGS = -D$(SYSTEM) -O
LINT = 
# the above line should be modified on systems other than USG Unix(R), as
# USG Unix(R) defaults to the major checks.  Don't bother with -p, as UNaXcess
# is definitely not designed to work anywhere other than Unix(R).

OBJS = ua.o message.o conference.o date.o bulletin.o system.o user.o dir.o \
       fileudl.o io.o
SRC  = ua.c message.c conference.c date.c bulletin.c system.c user.c dir.c \
       fileudl.c io.c
HDR  = ua.h user.h system.h dir.h
UTIL = install.sh mkconf.c udate.c uwho.c
SRCS = $(SRC) $(UTIL)

all: system ua uwho udate mkconf mvmsg guide
lint: system lintcheck

install: all
	-sh install.sh

ua: $(OBJS)
	$(CC) -o ua $(OBJS)

udate: udate.o date.o
	cc -O -o udate udate.o date.o

$(OBJS): $(HDR)

clean:
	rm -f ua uwho udate upost mkconf umotd *.o tags core ua.shar* mvmsg unaxcess.tar

tags: $(SRC) $(HDR)
	ctags $(SRC) $(HDR)

shar:
	mkshar -v -s 65000 -h ua.shar README *.[chl] Makefile *.sh *.help *.man

tar:
	tar cvf unaxcess.tar README *.[chl] Makefile *.sh *.help *.man

edall: %edall
	for file in *.[chl] Makefile *.sh; do\
		sh %edall $$file;\
	done

system:
	expr	"$(SYSTEM)" = V7 \| \
		"$(SYSTEM)" = SYS3 \| \
		"$(SYSTEM)" = SYS5 \| \
		"$(SYSTEM)" = BSD \| \
		"$(SYSTEM)" = XENIX \| \
		"$(SYSTEM)" = XENIX3 \| \
		"$(SYSTEM)" = XENIX5

lintcheck:
	lint $(LINT) -D$(SYSTEM) $(SRC)

guide: guide.man
	nroff -man guide.man > guide

uwho: uwho.o
	$(CC) -o uwho uwho.o

mkconf: mkconf.o
	$(CC) -o mkconf mkconf.o

mvmsg: mvmsg.o
	$(CC) -o mvmsg mvmsg.o

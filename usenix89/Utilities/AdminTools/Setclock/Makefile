#                  Makefile for setclock Program 
#
#      General Files

MAIN    =	setclock
SYS     =   XENIX
MANP    =   $(MAIN).7t
DOCS    =	README $(MANP) syncclock.mdl

#
#      General Flags

DEFS    =   -D$(SYS) -DBADDST
CFLAGS  =	-O -i $(DEFS)
LDFLAGS = 	-i -lx
TEXT    =	$(HDRS) $(MAIN).c $(SRCS)
OFF     =   8.0

#
#      General Programs

LINT    =	/usr/bin/lint -hp
PR      =	@print1
SED     =	sed
SPELL   =	spell
TBL     =	tbl
NROFF   =	nroff
SHAR    =	cshar -a
CC      =	/bin/cc

$(MAIN): $(MAIN).o $(OBJS)
		$(CC) $(LDFLAGS) -o $(MAIN) $(MAIN).o $(OBJS) $(LIBS)

$(OBJS) $(MAIN).o: $(HDRS)

save:
		mv /etc/setclock          /etc/setclock.old
		cp /etc/setclock.old      /etc/setclock
		mv /usr/lib/syncclock     /usr/lib/syncclock.old
		cp /usr/lib/syncclock.old /usr/lib/syncclock

install: setclock
		cp setclock  /etc/setclock
		chmod 700    /etc/setclock
		chown bin    /etc/setclock
		chgrp bin    /etc/setclock
		$(SED) "s/OFF/$(OFF)/" syncclock.mdl >syncclock
		cp syncclock /usr/lib/syncclock
		chmod 700    /usr/lib/syncclock
		chown bin    /usr/lib/syncclock
		chgrp bin    /usr/lib/syncclock
		if [ -d /usr/man/man7 ]  ; \
		then \
			cp $(MANP)   /usr/man/man7  ; \
			chmod 644    /usr/man/man7/$(MANP)  ; \
			chown bin    /usr/man/man7/$(MANP)  ; \
			chgrp bin    /usr/man/man7/$(MANP)  ; \
			rm -f /usr/man/man7/syncclock.7t ; \
			ln /usr/man/man7/$(MANP) /usr/man/man7/syncclock.7t  ; \
			fi

print:
		$(PR) $(TEXT)

lint:
		$(LINT) $(MAIN).c $(SRC)

man:
		$(TBL) $(MANP) | $(NROFF) -man | col >:$(MAIN).manpg

spell:
		$(SPELL) $(DOCS)

archive:
		$(SHAR) $(DOCS) :setclock.manpg Makefile $(TEXT) >setclock.shar

clean:
		rm -f *.o core *.out :*

.PRECIOUS: $(TEXT) $(DOCS)

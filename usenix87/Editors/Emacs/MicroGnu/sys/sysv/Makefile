# Makefile for MicroEMACS.
# Is there a better way to do the rebuilds, other than using
# the links?

SYS	= sysv
TTY	= termcap
# you may not need (or have) termlib...
LIBS	= -lcurses -ltermlib
# CDEFS gets defines, and gets passed to lint. CFLAGS gets flags, and doesn't
# get passed to lint.
CDEFS	= -DDO_METAKEY -DSTARTUP
CFLAGS	= -O $(CDEFS)

OBJ =	basic.o buffer.o cinfo.o display.o echo.o extend.o file.o kbd.o \
	line.o main.o match.o random.o region.o search.o symbol.o version.o \
	window.o paragraph.o prefix.o \
	word.o fileio.o spawn.o ttyio.o tty.o ttykbd.o
OSRCS = fileio.c spawn.c ttyio.c tty.c ttykbd.c
SRCS =	basic.c buffer.c cinfo.c display.c echo.c extend.c file.c kbd.c \
	line.c main.c match.o random.c region.c search.c symbol.c version.c \
	window.c word.c paragraph.c prefix.c
OINCS =	ttydef.h sysdef.h
INCS =	def.h

mg:		$(OBJ)
		cc $(CFLAGS) -o mg $(OBJ) $(LIBS)

# for AT&T UNIX-PC
mg.7300:	$(OBJ)
		ld /lib/crt0s.o /lib/shlib.ifile $(OBJ) -o mg.7300

lint: $(SRCS) $(OSRCS)
		lint $(CDEFS) $(SRCS) $(OSRCS)

$(OBJ):		def.h sysdef.h ttydef.h

sysdef.h:	sys/$(SYS)/sysdef.h	# Update links, if needed.
		rm -f sysdef.h
		ln sys/$(SYS)/sysdef.h .

ttydef.h:	tty/$(TTY)/ttydef.h
		rm -f ttydef.h
		ln tty/$(TTY)/ttydef.h .

fileio.c:	sys/$(SYS)/fileio.c
		rm -f fileio.c
		ln sys/$(SYS)/fileio.c .

spawn.c:	sys/$(SYS)/spawn.c
		rm -f spawn.c
		ln sys/$(SYS)/spawn.c .

tty.c:		tty/$(TTY)/tty.c
		rm -f tty.c
		ln tty/$(TTY)/tty.c .

ttyio.c:	sys/$(SYS)/ttyio.c
		rm -f ttyio.c
		ln sys/$(SYS)/ttyio.c .

ttykbd.c:	tty/$(TTY)/ttykbd.c
		rm -f ttykbd.c
		ln tty/$(TTY)/ttykbd.c .

clean:;		rm -f $(OBJ) $(OSRCS) $(OINCS)

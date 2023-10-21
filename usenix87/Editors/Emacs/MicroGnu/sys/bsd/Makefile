# Makefile for MicroEMACS.
# Is there a better way to do the rebuilds, other than using
# the links?

SYS	= bsd
TTY	= termcap
LIBS	= -ltermcap
# CDEFS gets defines, and gets passed to lint. CFLAGS gets flags, and doesn't
# get passed to lint.
CDEFS	=  -DDO_METAKEY -DSTARTUP
CFLAGS	= -g $(CDEFS)

OBJ =	basic.o buffer.o cinfo.o display.o echo.o extend.o file.o kbd.o \
	line.o main.o match.o random.o region.o search.o symbol.o version.o \
	window.o paragraph.o prefix.o \
	word.o fileio.o spawn.o ttyio.o tty.o ttykbd.o
OSRCS = fileio.c spawn.c ttyio.c tty.c ttykbd.c
SRCS =	basic.c buffer.c cinfo.c display.c echo.c extend.c file.c kbd.c \
	line.c main.c match.c random.c region.c search.c symbol.c version.c \
	window.c word.c paragraph.c prefix.c
OINCS =	ttydef.h sysdef.h
INCS =	def.h

mg:		$(OBJ)
	cc $(CFLAGS) -o mg $(OBJ) $(LIBS)

# the v arg to lint turns off all the complaints about args f, n & k.
# It's a good idea to take that out and rerun make lint after getting
# a clean lint, just to verify that f, n & k are the ONLY unused args.
lint: $(SRCS) $(OSRCS)
	lint -ahb $(CDEFS) $(SRCS) $(OSRCS)

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

port: $(SRCS) $(INCS)
	rm -f port
	tar cfb port 1 $?

clean:;	rm -f $(OBJ) $(OSRCS) $(OINCS)


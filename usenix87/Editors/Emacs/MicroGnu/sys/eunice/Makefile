# Makefile for MicroEMACS, under Eunice

SYS	=eunice
TTY	=termcap

LIBS	= -ltermcap
# CDEFS gets defines, and gets passed to lint. CFLAGS gets flags, and doesn't
# get passed to lint.
CDEFS	=  -Isys/$(SYS)/ -Itty/$(TTY)/ -DDO_METAKEY -DSTARTUP
CFLAGS	= -g $(CDEFS)

OBJ =	basic.o buffer.o cinfo.o display.o echo.o extend.o file.o kbd.o \
	line.o main.o match.o random.o region.o search.o symbol.o version.o \
	window.o paragraph.o prefix.o word.o \
	fileio.o spawn.o ttyio.o tty.o ttykbd.o

OSRCS = fileio.c spawn.c ttyio.c tty.c ttykbd.c
SRCS =	basic.c buffer.c cinfo.c display.c echo.c extend.c file.c kbd.c \
	line.c main.c match.c random.c region.c search.c symbol.c version.c \
	window.c word.c paragraph.c prefix.c
INCS =	def.h

mg:		$(OBJ) $(XOBJ)
	cc $(CFLAGS) -o mg $(OBJ) $(LIBS)
	rm -f $(OSRCS)

# the v arg to lint turns off all the complaints about args f, n & k.
# It's a good idea to take that out and rerun make lint after getting
# a clean lint, just to verify that f, n & k are the ONLY unused args.
lint: $(SRCS) $(OSRCS)
	lint -ahb $(CDEFS) $(SRCS) $(OSRCS)

clean:	rm -f *.o $(OSRCS)

$(OBJ):		def.h sys/$(SYS)/sysdef.h tty/$(TTY)/ttydef.h

fileio.c:	sys/$(SYS)/fileio.c
	cp sys/$(SYS)/fileio.c .

spawn.c:	sys/$(SYS)/spawn.c
	cp sys/$(SYS)/spawn.c .

tty.c:		tty/$(TTY)/tty.c
	cp tty/$(TTY)/tty.c .

ttyio.c:	sys/$(SYS)/ttyio.c
	cp sys/$(SYS)/ttyio.c .

ttykbd.c:	tty/$(TTY)/ttykbd.c
	cp tty/$(TTY)/ttykbd.c .

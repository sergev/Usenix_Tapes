#
# revgrep: search files for a pattern, backwards
#
#     Revgrep used to be composed of three source files, 
#   revgrep.c, revio.c, and revgrep.h.  Since none of the
#   three were particularly big, they were combined into 
#   one source file, revgrep.c.
#
SRCS= revgrep.c
OBJS= revgrep.o 
CFLAGS= -O
LIBES=

DESTDIR= /usr/local/bin

all: revgrep

install: revgrep
	install -s -m 755 revgrep ${DESTDIR}

revgrep: ${OBJS}
	cc ${CFLAGS} -o revgrep ${OBJS} ${LIBES}

clean: 
	rm -f revgrep.o a.out

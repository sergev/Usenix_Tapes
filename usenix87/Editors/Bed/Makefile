DESTBIN=/usr/local
DESTMAN=/usr/man/mann/bed.n
CFLAGS=-O -Dindex=strchr -Drindex=strrchr
OBJECTS=bed.o getcom.o quest.o
# The type whose width is 1 byte.
W8=char
# The type whose width is 2 bytes.
W16=short
# The type whose width is 4 bytes.
W32=int
# The default editor.
EDITOR=/usr/bin/vi

bed:	${OBJECTS}
	cc ${OBJECTS} -o $@

bed.o:	bed.c
	cc -c ${CFLAGS} -DW8=${W8} -DW16=${W16} -DW32=${W32} \
	-DEDITOR=\"${EDITOR}\" bed.c

install:bed
	install -s bed ${DESTBIN}
	cp bed.n ${DESTMAN}

clean:
	rm -f bed *.o

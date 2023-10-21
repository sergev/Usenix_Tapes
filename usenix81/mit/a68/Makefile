FILES = mical.h inst.h scan.c error.c init.c ins.c\
	ps.c rel.c sdi.c sym.c /usr/nu/include/b.out.h

OBJECTS = error.o init.o ins.o ps.o rel.o sdi.o sym.o scan.o

CFLAGS = -O

a68: $(OBJECTS)
	cc -o a68 $(OBJECTS)

$(OBJECTS): mical.h

init.o ins.o: mical.h inst.h

rel.c:	mical.h /usr/nu/include/b.out.h

sym.c:	mical.h /usr/nu/include/b.out.h

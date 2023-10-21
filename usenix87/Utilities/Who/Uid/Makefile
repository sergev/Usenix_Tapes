OBJS= all.o main.o one.o uid.o me.o
SRC= all.c main.c one.c uid.c me.c
FLAGS= -O 

.c.o:
	${CC} -c ${FLAGS} $*.c

all: uid

uid: ${OBJS}
	${CC} -o uid ${OBJS}

install:
	cp uid /u0/warren/bin
	cp uid /usr/local/bin
	cp uid.local /usr/man/manl/uid.l

lint:
	lint $(SRC)

print:	
	imprint ${SRC}

# DO NOT REMOVE THIS LINE

all.c: more.h

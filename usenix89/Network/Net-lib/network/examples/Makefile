CFLAGS =	-g

LIBS	=	../libnet.a

all: server client login2 test1 test2 ptest

server: server.o getopt.o
	${CC} ${CFLAGS} -o server server.o getopt.o ${LIBS}

client: client.o getopt.o
	${CC} ${CFLAGS} -o client client.o getopt.o ${LIBS}

login2:login.o
	${CC} ${CFLAGS} -o login2 login.o ${LIBS}

test1:test1.o
	${CC} ${CFLAGS} -o test1 test1.o ${LIBS}

test2:test2.o
	${CC} ${CFLAGS} -o test2 test2.o ${LIBS}

ptest:ptest.o
	${CC} ${CFLAGS} -o ptest ptest.o ${LIBS}

getopt.o:getopt.c
	${CC} ${CFLAGS} -DVMUNIX -c getopt.c

clean:
	@rm -f *.o server client login2 test1 test2 ptest

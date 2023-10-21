CFLAGS=

all: sockets

sockets: findserv serv

findserv: findserv.o client.o
	$(CC) -o findserv findserv.o client.o

serv: serv.o server.o
	$(CC) -o serv serv.o server.o

client.o: client.h
server.o: server.h

findserv.o: service.h client.h
serv.o:     service.h server.h

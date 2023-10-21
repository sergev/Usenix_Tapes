CFLAGS=-DMASSCOMP
UNIVERSE= ucb

all: client server finger

client:client.c
	$(UNIVERSE) $(CC) $(CFLAGS) -o client client.c

server:server.c
	$(UNIVERSE) $(CC) $(CFLAGS) -o server server.c

finger:finger.c
	$(UNIVERSE) $(CC) $(CFLAGS) -o finger finger.c

dist: /tmp
	shar -v README Makefile client.c server.c client.1 server.8 server.cmds phone.1 phone.local.sh phone.sh > shar.out

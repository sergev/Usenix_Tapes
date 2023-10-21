all: recvnews sendnews qnews

recvnews: recvnews.o dir.o
	$(CC) -o $@ recvnews.o dir.o

sendnews: sendnews.o dir.o
	$(CC) -o $@ sendnews.o dir.o

qnews: qnews.c common.h
	$(CC) -o $@ $(CFLAGS) qnews.c

sendnews.o recvnews.o: common.h

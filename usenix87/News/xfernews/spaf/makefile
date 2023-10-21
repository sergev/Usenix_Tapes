all: recvnews xsendnews qnews

recvnews: recvnews.o dir.o
	$(CC) -o $@ recvnews.o dir.o

xsendnews: sendnews.o dir.o
	$(CC) -o $@ sendnews.o dir.o

qnews: qnews.c common.h
	$(CC) -o $@ $(CFLAGS) qnews.c

sendnews.o recvnews.o: common.h

install: all
	cp mkspool xsendnews recvnews qnews /usr/lib/news
	cd /usr/lib/news; chown news xsendnews recvnews qnews
	cd /usr/lib/news; chgrp news xsendnews recvnews qnews
	chmod ug+s /usr/lib/news/xsendnews /usr/lib/news/recvnews /usr/lib/news/qnews

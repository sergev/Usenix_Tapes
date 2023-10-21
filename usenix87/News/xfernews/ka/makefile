YFLAGS=-d

all: brecvnews bsendnewsa bsendnewsb qnews unbat_a unbat_b

brecvnews: brecvnews.o dir.o
	$(CC) -o $@ brecvnews.o dir.o

sendnews: sendnews.o dir.o
	$(CC) -o $@ sendnews.o dir.o

bsendnewsa: bsendnews.o mkbat_a.o dir.o
	$(CC) -o $@ bsendnews.o mkbat_a.o dir.o

bsendnewsb: bsendnews.o mkbat_b.o dir.o
	$(CC) -o $@ bsendnews.o mkbat_b.o dir.o

qnews: qnews.c common.h
	$(CC) -o $@ $(CFLAGS) qnews.c

unbat_a: unbat_a.c common.h
	$(CC) -o $@ $(CFLAGS) unbat_a.c

unbat_b: unbat_b.c common.h pack.h
	$(CC) -o $@ $(CFLAGS) unbat_b.c

sendnews.o recvnews.o mkbat_a.o: common.h
bsendnews.o brecvnews.o: common.h common.c
mkbat_b.o: common.h pack.h

mkpack: mkpackl.o
	cc -o mkpack mkpacky.o mkpackl.o -ly -ll

mkpackl.o: mkpacky.o

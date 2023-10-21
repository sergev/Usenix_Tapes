CFLAGS = -O
BATCHDIR = /usr/spool/batch

all: batch batchd

batchd: batchd.o lex.o
	cc $(CFLAGS) batchd.o lex.o -o batchd

batch: batch.o
	cc $(CFLAGS) batch.o -o batch

lex.c: lex.l
	lex lex.l
	mv lex.yy.c lex.c

# Batch must be setuid root to signal batchd
install: all
	install -m 740 -o root batchd $(DESTDIR)/usr/lib
	install -m 4755 -o root $(DESTDIR)/usr/bin

clean:
	rm -f *.o lex.c batch batchd

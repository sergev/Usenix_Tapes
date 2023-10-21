#
#	Makefile for nbatcher Version 1.4
#
#	R.J. Esposito
#	Bell of Penna.
#	June 1986
#

#	You MUST define BATCHDIR as the place where USENET
#	puts the articles to be batched.
#
#	LIBDIR is where you USENET library is and also
#	MUST be defined.
#
#	Define PCODE for parse.c if you don't have/can't use
#	_doprnt.

BATCHDIR = /usr/spool/batchnews
LIBDIR = /misc/lib/usenet

CFLAGS = -O -c
LFLAGS = -s
DFLAGS = -DBATCHDIR='"$(BATCHDIR)"' -DLIBDIR='"$(LIBDIR)"'
# PCODE = -DUSE_PORT_CODE

OBJS = define.o main.o parse.o checkwork.o nbatcher.o logger.o

SRC = define.c main.c parse.c checkwork.c nbatcher.c logger.c nbatcher.h bst.c
DOCS = README nbatcher.ctl nbatcher.doc nbatcher.1

all: $(OBJS)
	$(CC) $(DFLAGS) $(LFLAGS) -o nbatcher $(OBJS)
	chmod 0755 nbatcher

install: all
	cp nbatcher $(LIBDIR)
	chmod 0755 $(LIBDIR)/nbatcher
	cp nbatcher.ctl $(LIBDIR)/nbatcher.ctl
	chmod 0644 $(LIBDIR)/nbatcher.ctl

define.o: nbatcher.h define.c
	$(CC) $(CFLAGS) $(DFLAGS) define.c

main.o: nbatcher.h main.c
	$(CC) $(CFLAGS) $(DFLAGS) main.c

parse.o: nbatcher.h parse.c
	$(CC) $(CFLAGS) $(DFLAGS) $(PCODE) parse.c

checkwork.o: nbatcher.h checkwork.c
	$(CC) $(CFLAGS) $(DFLAGS) checkwork.c

nbatcher.o: nbatcher.h nbatcher.c
	$(CC) $(CFLAGS) $(DFLAGS) nbatcher.c

logger.o: nbatcher.h logger.c
	$(CC) $(CFLAGS) $(DFLAGS) logger.c

bst: bst.c
	$(CC) $(CFLAGS) $(DFLAGS) bst.c
	$(CC) $(LFLAGS) -o bst bst.o

clean:
	rm -f *.o *.shar

clobber: clean
	rm -f nbatcher


shar:
	shar -v $(SRC) $(DOCS) Makefile > nbatcher.shar

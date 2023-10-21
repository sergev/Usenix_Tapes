CFLAGS=	-O -Dregfree=free
REGLIB=
LIBS= -ltermcap

OBJS=	hash.o groupdir.o envir_set.o newsrc.o pagefile.o reader.o storage.o sig_set.o term_set.o tty_set.o userlist.o vn.o vnglob.o digest.o strings.o ucb.o

vn:	$(OBJS)
	cc -o vn $(OBJS) $(REGLIB) $(LIBS)

CFLAGS=	-O -Dregfree=free -DJOBCONTROL
REGLIB=
LIBS= -ltermcap

OBJS=	hash.o groupdir.o envir_set.o newsrc.o pagefile.o reader.o storage.o sig_set.o term_set.o tty_set.o userlist.o vn.o vnglob.o digest.o strings.o tmpnam.o

vn:	$(OBJS)
	cc -o vn $(OBJS) $(LIBS) $(REGLIB)

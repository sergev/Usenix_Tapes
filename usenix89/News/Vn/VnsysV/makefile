CFLAGS=	-O -Dregfree=free -Dindex=strchr -Drindex=strrchr
REGLIB=
LIBS= -ltermcap -lPW

OBJS=	hash.o groupdir.o envir_set.o newsrc.o pagefile.o reader.o  \
storage.o sig_set.o term_set.o tty_set.o userlist.o vn.o vnglob.o \
digest.o strings.o

vn:	$(OBJS)
	cc -z -g -o vn $(OBJS) $(REGLIB) $(LIBS)

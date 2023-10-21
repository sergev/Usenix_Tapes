CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../_unix.O
LIB=$(INSDIR)/empcom.a
OBJS=\
	$(LIB)(acce.o) \
	$(LIB)(add.o) \
	$(LIB)(assa.o) \
	$(LIB)(atta.o) \
	$(LIB)(boar.o) \
	$(LIB)(buil.o) \
	$(LIB)(cens.o) \
	$(LIB)(chan.o) \
	$(LIB)(chec.o) \
	$(LIB)(coll.o) \
	$(LIB)(cont.o) \
	$(LIB)(coun.o) \
	$(LIB)(decl.o) \
	$(LIB)(defe.o) \
	$(LIB)(deli.o) \
	$(LIB)(desi.o) \
	$(LIB)(diss.o) \
	$(LIB)(enli.o) \
	$(LIB)(fire.o) \
	$(LIB)(flee.o) \
	$(LIB)(fly.o) \
	$(LIB)(fore.o) \
	$(LIB)(gran.o) \
	$(LIB)(info.o) \
	$(LIB)(ledg.o) \
	$(LIB)(lend.o) \
	$(LIB)(load.o) \
	$(LIB)(look.o) \
	$(LIB)(map.o) \
	$(LIB)(mine.o) \
	$(LIB)(move.o) \
	$(LIB)(nati.o) \
	$(LIB)(navi.o) \
	$(LIB)(news.o) \
	$(LIB)(offe.o) \
	$(LIB)(powe.o) \
	$(LIB)(rada.o) \
	$(LIB)(rea.o) \
	$(LIB)(real.o) \
	$(LIB)(repa.o) \
	$(LIB)(rout.o) \
	$(LIB)(set.o) \
	$(LIB)(shi.o) \
	$(LIB)(spy.o) \
	$(LIB)(tele.o) \
	$(LIB)(tend.o) \
	$(LIB)(torp.o) \
	$(LIB)(trad.o) \
	$(LIB)(trea.o) \
	$(LIB)(turn.o) \
	$(LIB)(upda.o) \
	$(LIB)(vers.o) \
	$(LIB)(vote.o) \
	$(LIB)(weat.o)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
	@if [ -s $(LIB) ];\
	then\
		ar r $(LIB) $?;\
	else\
		echo Creating $(LIB);\
		rm -f $(LIB);\
		ar cq $(LIB) $?;\
	fi
	@rm -f $?
	@echo $(LIB) is now up to date
.c.a:;

$(OBJS):	$(HDRDIR)/empdef.h

clobber:
	rm -f $(LIB)

.PRECIOUS:	$(LIB)

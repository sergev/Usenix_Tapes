CFLAGS = -O -q

SUBS =  equal.o getboard.o getchain.o getplot.o getchrs.o \
	holders.o locked.o putboard.o putchain.o putmisc.o putplyr.o \
	price.o \
	getmisc.o getplyr.o done.o \
	lock.o unlock.o

subs: $(SUBS)
	ar r ../fflib.a $(SUBS)

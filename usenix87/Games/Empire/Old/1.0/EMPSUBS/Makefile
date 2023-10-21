CFLAGS=-c
HDRDIR=../hdrs
INSDIR=../_unix.O
LIB=$(INSDIR)/empsub.a
HDRDEP=\
	$(LIB)(chkok.o) \
	$(LIB)(cleared.o) \
	$(LIB)(cname.o) \
	$(LIB)(cnumb.o) \
	$(LIB)(damage.o) \
	$(LIB)(erlog.o) \
	$(LIB)(getloan.o) \
	$(LIB)(getmysect.o) \
	$(LIB)(getmyship.o) \
	$(LIB)(getnat.o) \
	$(LIB)(getrel.o) \
	$(LIB)(getsect.o) \
	$(LIB)(getship.o) \
	$(LIB)(getsno.o) \
	$(LIB)(gettre.o) \
	$(LIB)(landgun.o) \
	$(LIB)(landorsea.o) \
	$(LIB)(mailbox.o) \
	$(LIB)(mtch.o) \
	$(LIB)(natarg.o) \
	$(LIB)(neigh.o) \
	$(LIB)(nreport.o) \
	$(LIB)(nxtsct.o) \
	$(LIB)(nxtshp.o) \
	$(LIB)(pr.o) \
	$(LIB)(prdate.o) \
	$(LIB)(sargs.o) \
	$(LIB)(sread.o) \
	$(LIB)(tfact.o) \
	$(LIB)(trechk.o) \
	$(LIB)(update.o) \
	$(LIB)(wethr.o) \
	$(LIB)(wu.o)

$(LIB): \
	$(LIB)(atoip.o) \
	$(LIB)(atopi.o) \
	$(LIB)(chkok.o) \
	$(LIB)(cleared.o) \
	$(LIB)(cname.o) \
	$(LIB)(cnumb.o) \
	$(LIB)(copy.o) \
	$(LIB)(damage.o) \
	$(LIB)(erlog.o) \
	$(LIB)(getbit.o) \
	$(LIB)(getele.o) \
	$(LIB)(getloan.o) \
	$(LIB)(getlogn.o) \
	$(LIB)(getmysect.o) \
	$(LIB)(getmyship.o) \
	$(LIB)(getnat.o) \
	$(LIB)(getrel.o) \
	$(LIB)(getsect.o) \
	$(LIB)(getship.o) \
	$(LIB)(getshno.o) \
	$(LIB)(getsno.o) \
	$(LIB)(getstar.o) \
	$(LIB)(getstri.o) \
	$(LIB)(gettre.o) \
	$(LIB)(idist.o) \
	$(LIB)(landgun.o) \
	$(LIB)(landorsea.o) \
	$(LIB)(mailbox.o) \
	$(LIB)(max127.o) \
	$(LIB)(mtch.o) \
	$(LIB)(myeuid.o) \
	$(LIB)(myruid.o) \
	$(LIB)(natarg.o) \
	$(LIB)(neigh.o) \
	$(LIB)(nreport.o) \
	$(LIB)(nxtsct.o) \
	$(LIB)(nxtshp.o) \
	$(LIB)(onearg.o) \
	$(LIB)(plur.o) \
	$(LIB)(pr.o) \
	$(LIB)(prdate.o) \
	$(LIB)(prmptrd.o) \
	$(LIB)(pslsin.o) \
	$(LIB)(resetuid.o) \
	$(LIB)(round.o) \
	$(LIB)(same.o) \
	$(LIB)(sargs.o) \
	$(LIB)(sigsave.o) \
	$(LIB)(sread.o) \
	$(LIB)(tfact.o) \
	$(LIB)(trechk.o) \
	$(LIB)(ttyn.o) \
	$(LIB)(update.o) \
	$(LIB)(wethr.o) \
	$(LIB)(wu.o) \
	$(LIB)(xwrap.o) \
	$(LIB)(xytoa.o)
	$(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
	@if [ -s $(LIB) ];\
	then\
		ar r $(LIB) $?;\
	else\
		echo Creating $(LIB);\
		rm -f $(LIB);\
		ar cq $(LIB) `lorder $? | tsort`;\
	fi
	@rm -f $?
	@echo $(LIB) is now up to date
.c.a:;

$(HDRDEP):	$(HDRDIR)/empdef.h

clobber:
	rm -f $(LIB)

.PRECIOUS:	$(LIB)

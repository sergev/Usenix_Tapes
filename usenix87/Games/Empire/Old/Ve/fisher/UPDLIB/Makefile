CFLAGS= -c -O
HDRDIR =../hdrs
INSDIR=../_unix.O
LIB=$(INSDIR)/updlib.a
HDRDEP= \
	$(LIB)(docen.o) \
	$(LIB)(domap.o) \
	$(LIB)(dorad.o) \
	$(LIB)(dospy.o)

$(LIB): \
	$(LIB)(docen.o) \
	$(LIB)(domap.o) \
	$(LIB)(dorad.o) \
	$(LIB)(dospy.o)
	$(CC) $(CFLAGS) -I$(HDRDIR) $(?:.o=.c)
	@ar r $(LIB) *.o
	@rm -f *.o
	@echo $@ is now up to date
.c.a:;

$(HDRDEP):	$(HDRDIR)/emp.h

clean:

clobber:
	rm -f $(LIB)

.PRECIOUS:	$(LIB)


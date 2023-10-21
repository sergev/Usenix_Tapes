CFLAGS = -O
DFLAGS =
LFLAGS =
BINDIR = $$HOME/bin
PROGRM = w
OBJS   = ch_time.o w.o

.c.o:
	cc $(CFLAGS) $(DFLAGS) -c $*.c

$(PROGRM): $(OBJS)
	cc $(CFLAGS) $(DFLAGS) -o $(PROGRM) $(OBJS) $(LFLAGS);
#	@rm -f $(BINDIR)/$(PROGRM)
#	@ln $(PROGRM) $(BINDIR)
#	@chmod 755 $(PROGRM)

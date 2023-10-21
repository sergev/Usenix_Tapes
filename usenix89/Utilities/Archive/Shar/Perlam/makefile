CFLAGS=-O

OBJS=shar.o traverse.o getopt.o
SRCS=shar.c traverse.c getopt.c
DOCS=shar.1 traverse.3 getopt.3

PR=/usr/ucb/print
LINT=lint -h

shar: $(OBJS) makefile
	@cc $(CFLAGS) -o shar $(OBJS)

traverse:
	@cc -o traverse -DSTANDALONE traverse.c

archive: makefile $(SRCS) $(DOCS)
	@shar -a makefile $(SRCS) $(DOCS) > archive

print:
	@$(PR) makefile shar.c

lint:
	@$(LINT) $(SRCS)

clean:
	@-rm -f core a.out *.o

BINDEST	      = /usr/local/bin
DOCDEST	      = /usr/man/manl

CC	      = cc
CFLAGS	      = -c -O

LD	      = cc
LDFLAGS	      =
LIBS	      =

OBJS	      = rename.o
SRCS	      = rename.c
HDRS	      =

MAKEFILE      = Makefile
DOC	      = rename.l
PROGRAM	      = rename

SHAR	      = shar
SHARFLAGS     = -v
SHARFILES     = $(SRCS) $(HDRS) $(MAKEFILE) $(DOC) README
SHARFILE      = rename.shar

build:		$(PROGRAM)

$(PROGRAM):     $(OBJS)
		$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)

install:	$(PROGRAM)
		rm -f $(BINDEST)/$(PROGRAM)
		cp $(PROGRAM) $(BINDEST)
		chmod 755 $(BINDEST)/$(PROGRAM)
		rm -f $(DOCDEST)/$(DOC)
		cp $(DOC) $(DOCDEST)
		chmod 644 $(DOCDEST)/$(DOC)

clean:;		rm -f $(OBJS) $(SHARFILE)

clobber: clean
		rm -f $(PROGRAM)

shar:;		$(SHAR) $(SHARFLAGS) $(SHARFILES) > $(SHARFILE)

.c.o:; $(CC) ${CFLAGS} $<
###
rename.o: /usr/include/stdio.h

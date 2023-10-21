# Change the following defaults to suit your taste
TABS          = 4
RCS           = true
SCCS          = false

CFLAGS        = -g -DTABS=$(TABS) -DRCS=$(RCS) -DSCCS=$(SCCS)

DEST	      = /usr/local

EXTHDRS	      = /usr/include/ctype.h \
		/usr/include/stdio.h

HDRS	      = mkprog.h

LINKER	      = cc -g

MAKEFILE      = Makefile

GETOPT        =
# if you do not have getopt(3) in your library uncomment the following
# statement:
# GETOPT      = getopt.o
OBJS	      = constant.o \
		do_mkprog.o \
		efopen.o \
		$(GETOPT) \
		mkprog.o

PRINT	      = pr

PROGRAM	      = mkprog

SRCS	      = constant.c \
		do_mkprog.c \
		efopen.c \
		getopt.c \
		mkprog.c

MANPAGE       =	mkprog.1 efopen.3s

SHAR_OBJECTS  =	README TODO EXAMPLE Makefile $(HDRS) $(SRCS) $(MANPAGE)

SHARFILE      = mkprog.shar

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

lint:;		lint $(SRCS)

clean:;		@rm -f $(OBJS) $(PROGRAM) $(SHARFILE) prog prog.? a.out core

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install

shar:;		@shar -a $(SHAR_OBJECTS) > $(SHARFILE)
###
constant.o: mkprog.h /usr/include/stdio.h /usr/include/ctype.h
do_mkprog.o: mkprog.h /usr/include/stdio.h /usr/include/ctype.h
efopen.o: /usr/include/stdio.h
getopt.o: /usr/include/stdio.h
mkprog.o: mkprog.h /usr/include/stdio.h /usr/include/ctype.h

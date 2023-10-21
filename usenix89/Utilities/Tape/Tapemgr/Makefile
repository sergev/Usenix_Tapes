DEST	      = /usr/local

CFLAGS        = -O

EXTHDRS	      = /usr/include/ctype.h \
		/usr/include/machine/param.h \
		/usr/include/pwd.h \
		/usr/include/sgtty.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/strings.h \
		/usr/include/sys/ioctl.h \
		/usr/include/sys/mtio.h \
		/usr/include/sys/param.h \
		/usr/include/sys/stat.h \
		/usr/include/sys/time.h \
		/usr/include/sys/ttychars.h \
		/usr/include/sys/ttydev.h \
		/usr/include/sys/types.h \
		/usr/include/sys/wait.h

HDRS	      =

INSFLAGS      = -m 04555 -o root -g tapemgr

LDFLAGS	      = -O

LIBS	      =

LINKER	      = cc

LINTFLAGS     =

LINTLIST      =

MAKEFILE      = Makefile

OBJS	      = tapemgr.o

PRINT	      = lpr -p -i

PROGRAM	      = tapemgr

SRCS	      = tapemgr.c

VERSION	      =

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

co:;		@co -r$(VERSION) $(HDRS) $(SRCS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

diff:;		@rcsdiff -r $(VERSION) $(HDRS) $(SRCS)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(INSFLAGS) $(PROGRAM) $(DEST)

lint:;		@lint $(LINTFLAGS) -Dlint $(SRCS) $(LINTLIST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install

.DEFAULT:;	@co -r$(VERSION) $@
###
tapemgr.o: /usr/include/ctype.h /usr/include/pwd.h /usr/include/stdio.h \
	/usr/include/strings.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sgtty.h /usr/include/sys/param.h \
	/usr/include/machine/param.h /usr/include/signal.h \
	/usr/include/sys/types.h /usr/include/sys/mtio.h \
	/usr/include/sys/stat.h /usr/include/sys/time.h \
	/usr/include/sys/wait.h

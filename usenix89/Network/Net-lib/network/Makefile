CFLAGS        = -O

DEST	      = /usr/local/lib

MDEST	      = /usr/man/man3

EXTHDRS	      = /usr/include/fcntl.h 

HDRS	      = net.h \
		netw.h \
		openpty.h

LIBRARY	      = libnet.a

MAKEFILE      = Makefile

OBJS	      = basic.o \
		ident.o \
		isignal.o \
		openpty.o \
		perror.o \
		proutines.o \
		tty.o \
		utmp.o

PRINT	      = imprint -O -2

SRCS	      = basic.c \
		ident.c \
		isignal.c \
		openpty.c \
		perror.c \
		proutines.c \
		tty.c \
		utmp.c


.c.o:
	$(CC) $(CFLAGS) -c $<

all:		$(LIBRARY)

$(LIBRARY):	$(OBJS)
		@echo -n "Loading $(LIBRARY) ... "
		@ar cru $(LIBRARY) $(OBJS)
		@ranlib $(LIBRARY)
		@echo "done"

clean:		
		@rm -f $(OBJS) libnet.a

depend:;	@mkmf -f $(MAKEFILE) LIBRARY=$(LIBRARY) DEST=$(DEST)

extract:;	@ar xo $(DEST)/$(LIBRARY)
		@rm -f __.SYMDEF

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(LIBRARY)
		@echo Installing $(LIBRARY) in $(DEST)
		install $(LIBRARY) $(DEST)
		ranlib $(DEST)/$(LIBRARY)
		@echo Copying the manual page to ${MDEST}
		cp netw.3 ${MDEST}
		@echo Copying the header file
		cp netw.h /usr/include

library:        $(LIBRARY)

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:         $(DEST)/$(LIBRARY)

$(DEST)/$(LIBRARY): $(SRCS) $(HDRS) $(EXTHDRS)
		@-ar xo $(DEST)/$(LIBRARY)
		@make -f $(MAKEFILE) DEST=$(DEST) install clean
###

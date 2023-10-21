DEST	      = .

EXTHDRS	      = /usr/include/ctype.h \
		/usr/include/curses.h \
		/usr/include/math.h \
		/usr/include/pwd.h \
		/usr/include/sgtty.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/strings.h \
		/usr/include/sys/file.h \
		/usr/include/sys/ioctl.h \
		/usr/include/sys/time.h \
		/usr/include/sys/ttychars.h \
		/usr/include/sys/ttydev.h \
		/usr/include/time.h

HDRS	      = def.h \
		extern.h \
		include.h \
		struct.h \
		tunable.h

LDFLAGS	      =
CFLAGS        = -g
YFLAGS        =	-d

LIBS	      = -ll -lcurses -ltermcap

LINKER	      = ld

MAKEFILE      = Makefile

OBJS	      = extern.o \
		grammar.o \
		input.o \
		lex.o \
		list.o \
		log.o \
		main.o \
		update.o

CURSES_OBJ    = graphics_curses.o

LOBJS	      = tunable.o

PRINT	      = pr

CURSESLIBR    = libatc_curses.o

SRCS	      = extern.c \
		grammar.y \
		graphics_curses.c \
		input.c \
		lex.l \
		list.c \
		log.c \
		main.c \
		update.c

LSRCS	      =	tunable.c

MAKEFILES     = Makefile.dist Makefile.parent

ASRCS	      = $(SRCS) $(LSRCS) $(HDRS) $(MAKEFILES)

all:		$(XLIBR) $(CURSESLIBR)

co:
		co $(ASRCS)

$(CURSESLIBR):	$(OBJS) $(CURSES_OBJ)
		$(LINKER) -r $(LDFLAGS) $(OBJS) $(CURSES_OBJ) -o $(CURSESLIBR)

clean:;		@rm -f $(OBJS) $(CURSES_OBJ)

depend:;	@mkmf -f $(MAKEFILE) LIBR=$(LIBR) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(ASRCS)

install:	$(LIBR)
		@echo Installing $(LIBR) in $(DEST)
		@install -s $(LIBR) $(DEST)

print:;		@$(PRINT) $(HDRS) $(ASRCS)

program:        $(LIBR)

tags:           $(HDRS) $(ASRCS); @ctags $(HDRS) $(ASRCS)

update:		$(DEST)/$(LIBR)

$(DEST)/$(LIBR): $(ASRCS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install

lex.o: grammar.o
###
extern.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
grammar.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
graphics_curses.o: /usr/include/curses.h /usr/include/stdio.h \
	/usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h include.h \
	/usr/include/ctype.h /usr/include/pwd.h /usr/include/sys/time.h \
	/usr/include/time.h /usr/include/sys/file.h /usr/include/signal.h \
	/usr/include/math.h /usr/include/strings.h def.h struct.h extern.h \
	tunable.h
input.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
list.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
log.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
main.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h
update.o: include.h /usr/include/stdio.h /usr/include/ctype.h \
	/usr/include/pwd.h /usr/include/sgtty.h /usr/include/sys/ioctl.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/ttychars.h /usr/include/sys/ttydev.h \
	/usr/include/sys/time.h /usr/include/time.h /usr/include/sys/file.h \
	/usr/include/signal.h /usr/include/math.h /usr/include/strings.h \
	def.h struct.h extern.h tunable.h

CFLAGS= -O

eobjs=address.o buffer.o commands.o debug.o dispatch.o execute.o \
	flsbuf.o getchr.o help.o insert.o interrupt.o \
	io.o macro.o map.o match.o options.o process.o re.o scan.o \
	screen.o shell.o setup.o spell.o split.o tempfile.o misc.o \
	patch.o probe.o  
esrcs=address.c buffer.c commands.c debug.c dispatch.c execute.c \
	flsbuf.c getchr.c help.c insert.c interrupt.c \
	io.c macro.c map.c match.c options.c process.c re.c scan.c \
	screen.c shell.c setup.c spell.c split.c tempfile.c misc.c \
	patch.c probe.c
wobjs=enscreen.o make.o window.o terminal.o subs.o
wsrcs=enscreen.c make.c window.c terminal.c subs.c
editobjects = edit0.o edit1.o edit3.o edit4.o edit5.o edit9.o
heads=conf.h enscreen.h make.h process.h shell.h spell.h term.h terminal.h \
	window.h
editheads=ed.h edit.h file.h address.h options.h scan.h

terminal.o:	terminal.h term.h
enscreen.o insert.o make.o patch.o screen.o subs.o terminal.o window.o:	window.h
address.o commands.o main.o enscreen.o io.o misc.o patch.o setup.o shell.o split.o tempfile.o: file.h
address.o: address.h
options.h:	options.c makeoptions
	makeoptions

edit:	main.o $(eobjs) $(wobjs)
	cc -i -o edit main.o $(eobjs) $(wobjs)
	(date;size edit)>>histfile

install:	edit
	cp edit /bin
	chown root /bin/edit
	chmod 4755 /bin/edit
	cp edit.help /usr/lib/edit/help
	cd sh; make install
	cd spell; make install


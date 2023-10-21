SRCDIR=/usr/src/local/cmd/calend
RDIR=/usr/src/local/cmd
SRCS=calend.c process.c util.c parser scanner globals.h process.h pars.h 
MISC=calend.1l Makefile remind.c remind.1l README addcal addcal.1l
LOCDIR=/usr/local/bin
CFLAGS = -O
ScFile = calend.sc

calend: calend.o process.o util.o y.tab.o
	cc $(CFLAGS) -o calend calend.o process.o util.o y.tab.o

calend.c: globals.h process.h

process.c: globals.h process.h

util.c: globals.h

y.tab.c: parser lex.yy.c
	yacc -d parser

lex.yy.c: scanner
	lex scanner

parser: globals.h pars.h

remind: remind.c
	cc $(CFLAGS) -o remind remind.c

.DEFAULT:
	co $<

clean:
	rm -f y.* lex.* *.o calend remind

print:
	cat $(SRCS) | calls >'Function Calls'
	cpr -r 'Function Calls' $(SRCS) Makefile | sprint
	rm 'Function Calls'

inst-src:
	rm -f $(SRCDIR)/*
	cp $(SRCS) $(MISC) $(SRCDIR)
	cp Makefile.sys $(SRCDIR)/Makefile
	rm -f $(RDIR)/remind.c
	ln $(SRCDIR)/remind.c $(RDIR)/remind.c

$(LOCDIR)/calend: calend
	rm -f $(LOCDIR)/calend
	cp calend $(LOCDIR)/calend

inst-calend: $(LOCDIR)/calend

$(LOCDIR)/addcal: addcal
	rm -f $(LOCDIR)/addcal
	cp addcal $(LOCDIR)/addcal

inst-addcal: $(LOCDIR)/addcal

$(LOCDIR)/remind: remind
	rm -f $(LOCDIR)/remind
	cp remind $(LOCDIR)/remind

inst-remind: $(LOCDIR)/remind

inst-man: inst-manc inst-manr inst-mana

inst-manc: /usr/man/local/man1/calend.1l
inst-manr: /usr/man/local/man1/remind.1l
inst-mana: /usr/man/local/man1/addcal.1l

/usr/man/local/man1/calend.1l: calend.1l
	cp calend.1l /usr/man/local/man1/calend.1l

/usr/man/local/man1/addcal.1l: addcal.1l
	cp addcal.1l /usr/man/local/man1/addcal.1l

/usr/man/local/man1/remind.1l: remind.1l
	cp remind.1l /usr/man/local/man1/remind.1l

all: calend remind

inst-all: inst-calend inst-addcal inst-remind inst-man
	@echo Installed

script: 
	makescript -o ${ScFile} $(SRCS) $(MISC)

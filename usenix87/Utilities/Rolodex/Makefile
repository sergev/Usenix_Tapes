# Makefile for rolo

# Configure for your type of terminal capability database.  
#  if you have termcap,  set TERMSTUFF= -DTERMCAP,  and LIBS= -ltermcap
#  if you have terminfo, set TERMSTUFF= -DTERMINFO, and LIBS= -lterminfo
#  if you dont have either, leave them both blank (only "clear_screen" is used)
TERMSTUFF=
LIBS=
#
CC = cc
ROLOLIB = \"./helplib\"
TOOLDIR = ./toolsdir
CFLAGS = -DBSD42 -DROLOLIB=$(ROLOLIB) $(TERMSTUFF) -I$(TOOLDIR) -O
DBGFLAG = 
SOURCES = clear.c io.c menuaux.c operations.c options.c \
	rlist.c rolo.c search.c update.c
OBJECTS = clear.o io.o menuaux.o operations.o options.o \
	rlist.o rolo.o search.o update.o
TOOLOBJ = $(TOOLDIR)/mem.o $(TOOLDIR)/args.o $(TOOLDIR)/ctools.o \
	$(TOOLDIR)/menu.o
BIN = /usr/local
MS = 1
MAN = /usr/man/man$(MS)

rolo: $(OBJECTS)
	cd $(TOOLDIR) ; make tools ; cd ..
	$(CC) -o rolo $(DBGFLAG) $(CFLAGS) $(OBJECTS) $(TOOLOBJ) $(LIBS)
install: rolo
	rm -f $(BIN)/rolo
	mv rolo $(BIN)/rolo
man: 
	rm -f $(MAN)/rolo.1
	cp rolo.1 $(MAN)/rolo.$(MS)
	man rolo
clean:
	rm -f $(OBJECTS) rolo *~ foo $(TOOLDIR)/*.o

shar:
	cshar README rolo.1 Makefile helplib > roloshar1
	cshar *.[ch] > roloshar2
	cshar toolsdir > roloshar3

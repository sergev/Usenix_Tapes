# PRINTSERVER makefile

#COPTS  = 
COPTS  = -O
#CDEFS  = -DDEBUG 
CDEFS  =
INCDIR =
CFLAGS = $(COPTS) $(CDEFS) $(INCDIR)

LFLAGS = 
LIBS   = 
BIN_DIR = 
PROG = year

CFILES = \
	main.c \
	days.c \
	early.c 


OFILES = $(CFILES:.c=.o)
LFILES = $(CFILES:.c=.ln)

.SUFFIXES: .ln
.c.ln:
	lint -abchu $(INCDIR) $*.c

build	: $(PROG)

pbuild	: $(OFILES)
	cc -p -o $(PROG) $(LFLAGS)  $(OFILES)  $(LIBS)

all	: $(PROG) lint

$(PROG): $(OFILES)
	cc -o $(PROG) $(LFLAGS)  $(OFILES)  $(LIBS)

install	: $(PROG) 
	mv $(PROG) $(BIN_DIR)

lint	: $(LFILES)

clean	:
	-rm $(PROG)
	-rm *.o

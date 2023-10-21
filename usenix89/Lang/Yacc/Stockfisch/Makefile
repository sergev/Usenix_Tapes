YFLAGS	= -dv
CFLAGS	= -O
LFLAGS	=

SRC	= gram.y scan.l main.c
OBJ	= gram.o scan.o main.o

$(BIN)/ansi_c :	$(OBJ)
	cc $(CFLAGS) $(OBJ)/ansi_c

scan.o	: y.tab.h

clean	:
	rm -f y.tab.h y.output *.o

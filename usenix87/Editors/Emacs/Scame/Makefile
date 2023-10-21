CFLAGS= -O
SRC= main.c screen.c scame0.c scame1.c scame2.c io.c files.c strings.c \
	macro.c k_funcs.c x_funcs.c disptab.c help.c search.c vars.c
INC= config.h scame.h k_funcs.h
OBJ= main.o screen.o scame0.o scame1.o scame2.o io.o files.o strings.o \
	macro.o k_funcs.o x_funcs.o disptab.o help.o search.o vars.o
LIB= lib

scame:	$(OBJ)
	$(CC) $(CFLAGS) -o scame $(OBJ) -lcurses
#	$(CC) $(CFLAGS) -o scame $(OBJ) -ltermlib

$(OBJ):	$(INC)

.c.o:	; $(CC) $(CFLAGS) -c $*.c

scame-reaper:	scame-reaper.c
	$(CC) $(CFLAGS) -o scame-reaper scame-reaper.c

shar:	
	shar -c -v README Makefile scame.1 config.sample BUGS scame.h \
		k_funcs.h > scame.shar.1
	shar -c -v `find $(LIB) -print` tmp > scame.shar.2
	shar -c -v main.c screen.c scame0.c scame1.c > scame.shar.3
	shar -c -v scame2.c strings.c io.c files.c macro.c k_funcs.c \
		> scame.shar.4
	shar -c -v x_funcs.c disptab.c help.c search.c vars.c scame-reaper.c \
		> scame.shar.5

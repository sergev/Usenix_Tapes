# MakeFile for Unix Windows Amiga Client

INCLUDE = -iLC:include/ -iLC:include/lattice/
CC     = LC:lc $(INCLUDE)
LIB    = LC:
LIBS   = $(LIB)amiga.lib $(LIB)lc.lib 
# LIBS   = $(LIB)lc.lib $(LIB)amiga.lib
START  = $(LIB)Astartup.obj
# START  = $(LIB)Lstartup.obj
LINK   = LC:blink

H       = term.h

FILES   = $H terminal.c windows.c menus.c serial.c console.c intui.c uw.c

OBJS    = terminal.o windows.o menus.o serial.o console.o intui.o uw.o
LOBJS  = terminal.o+windows.o+menus.o+serial.o+console.o+intui.o+uw.o

uw : $(OBJS)
   $(LINK) $(START)+$(LOBJS) TO uw LIBRARY $(LIBS)

terminal.o : terminal.c $H
   $(CC) $(CFLAGS)  terminal.c

windows.o : windows.c
   $(CC) $(CFLAGS)  windows.c

menus.o : menus.c
   $(CC) $(CFLAGS)  menus.c

serial.o : serial.c
   $(CC) $(CFLAGS)  serial.c

console.o : console.c
   $(CC) $(CFLAGS)  console.c

intui.o : intui.c
   $(CC) $(CFLAGS)  intui.c

uw.o : $H uw.c
   $(CC) $(CFLAGS) uw.c

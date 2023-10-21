CFLAGS	= -c -O
PROF	= 		# -pg for gprof, -g for dbx
LIBS	= 
OBJS	= prep.o flow.o vec.o misc.o str.o macro.o ifdef.o

.SUFFIXES : 
.SUFFIXES : .o .c

prep :: $(OBJS)
	cc $(PROF) -o prep $(OBJS) $(LIBS)

.c.o :
	cc $(CFLAGS) $(PROF) $*.c

$(OBJS) : prep.h


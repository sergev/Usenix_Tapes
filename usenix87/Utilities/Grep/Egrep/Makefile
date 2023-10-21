# optional items for ENV:
# -DBSD			make -i work as in System V
# -I.			use regexp.h in current directory, not /usr/include
# -DEGREP=path		default /usr/bin/egrep
# -DV7			invoke xread() for system time quirk

ENV= -I.

# optional items for OBJ:
# compat-sys5.o		for V7 or BSD 4.2 systems w/no getopt(3) or string(3)
# regexp.o		if Henry Spencer's regexp(3) is not installed
# regerror.o			"
#			V8 people -- your regexp.h won't do

OBJ= regexp.o regerror.o

CFLAGS= -O -i $(ENV) 

egrep:	egrep.o $(OBJ)
	cc $(CFLAGS) $(OBJ) egrep.o -o egrep

install:
	mv egrep /usr/local

SOURCES =	main.c dont.c etc.c help.c makeb.c pass.c pay.c prints.c \
		props.c subs.c final.c random.c
OBJECTS =	main.o dont.o etc.o help.o makeb.o pass.o pay.o prints.o \
		props.o subs.o final.o random.o
#
# Use the following defines:
#
#	-DBSD42		for Berkeley Unix 4.2
#	-DBSD29		for Berkeley Unix 2.9
#
CFLAGS  =	-O -DBSD42

#
# if you are using BSD29, then add the -i option to the cc line
#
craps:	${OBJECTS}
	cc ${CFLAGS} -o craps ${OBJECTS} -lcurses -ltermlib -lm

main.o:		main.c types.h
dont.o:		dont.c types.h ext.h
etc.o:		etc.c types.h ext.h
help.o:		help.c types.h ext.h
makeb.o:	makeb.c types.h ext.h
pass.o:		pass.c types.h ext.h
pay.o:		pay.c types.h ext.h
prints.o:	prints.c types.h ext.h
props.o:	props.c types.h ext.h
subs.o:		subs.c types.h ext.h
final.o:	final.c types.h ext.h
random.o:	random.c types.h ext.h

clean:
	rm -f craps *.o

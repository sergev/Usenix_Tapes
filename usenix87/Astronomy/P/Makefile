SRC=main.c time.c conv.c wand.c sm.c misc.c glob.c io.c

p:	main.o time.o conv.o wand.o sm.o misc.o glob.o io.o Makefile
	cc main.o time.o conv.o wand.o sm.o misc.o glob.o io.o \
		-o p -lm -lcurses

main.o time.o conv.o wand.o sm.o misc.o glob.o io.o : p.h

print:
	@prdef -args -c82 p.h $(SRC)

cf:
	@cf -n -B $(SRC) | pr -l88 -h p

lint:
	lint $(SRC) -lm

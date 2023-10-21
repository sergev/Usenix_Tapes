CFLAGS= -O -c -DDEBUG
# CFLAGS= -O -c

tu58:   main.o tu58.o io.o checksum.o file.o tu58.h debug.o
	cc -o tu58 main.o tu58.o io.o checksum.o file.o  debug.o
	chgrp micro tu58
	chmod 4755 tu58
	rm err
checksum.o: tu58.h
	cc $(CFLAGS) checksum.c 2>err
	echo 
io.o:	tu58.h  debug.h
	cc $(CFLAGS) io.c 2>err
	echo 
main.o: tu58.h debug.h
	cc $(CFLAGS) main.c 2>err
	echo 
tu58.o: tu58.h debug.h
	cc $(CFLAGS) tu58.c 2>err
	echo 
file.o:	file.c tu58.h debug.h rtdir.h
	cc $(CFLAGS) file.c 2>err
	echo 
debug.o: debug.c
	cc $(CFLAGS) debug.c 2>err
	echo 

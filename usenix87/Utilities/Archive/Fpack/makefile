C=$(CC) $(CFLAGS) -DVERBOSE

fpack: fpack.c
	$C -o fpack fpack.c

test: fpack
	fpack.test

lint:
	lint -hp fpack.c

archive: fpack.1 makefile fpack.c fpack.test
	shar fpack.1 makefile fpack.c fpack.test > archive

fpack.1: fpack.c
	seec -t MANUAL fpack.c > fpack.1

gprof:
	$C -pg -o fpack fpack.c

.PRECIOUS: fpack.c

CFLAGS = -O

tptest: tptest.o
	cc -o tptest tptest.o

install: tptest
	cp tptest /usr/lbin

clean:
	rm *.o tptest

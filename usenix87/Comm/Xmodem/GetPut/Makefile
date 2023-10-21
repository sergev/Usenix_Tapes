all: xmget xmput

clean:
	rm -fr	xmget xmput
sterile:
	make clean
	rm -fr ttystuff.o xmget.o xmput.o

xmget: xmget.o ttystuff.o
	cc -o xmget xmget.o ttystuff.o

xmput: xmput.o ttystuff.o
	cc -o xmput xmput.o ttystuff.o

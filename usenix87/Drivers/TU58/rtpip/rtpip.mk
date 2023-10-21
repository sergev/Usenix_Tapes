OPTIONS= -DVAX
CFLAGS= -c ${OPTIONS}
rtpip: rtpip.o rtun.o
	cc -o rtpip rtpip.o rtun.o
	rm err
rtpip.o:	rtpip.c
	cc ${CFLAGS} rtpip.c 2>err
	echo 
rtun.o:	rtun.c
	cc ${CFLAGS} rtun.c 2>err
	echo 

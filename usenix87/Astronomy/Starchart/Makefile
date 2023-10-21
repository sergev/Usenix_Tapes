CFLAGS = -O

moonphase: moonphase.c
	cc $(CFLAGS) moonphase.c -lm -o /u/awpaeth/bin/moonphase

planet:	planet.c
	cc $(CFLAGS) planet.c -lm -o /u/awpaeth/bin/planet

startek: starchart.c starcharttek.c
	cc $(CFLAGS) starchart.c starcharttek.c -lm -o /u/awpaeth/bin/startek

starpic: starchart.c starchartpic.c
	cc $(CFLAGS) starchart.c starchartpic.c -lm -o /u/awpaeth/bin/starpic

starpost: starchart.c starchartpost.c
	cc $(CFLAGS) starchart.c starchartpost.c -lm -o /u/awpaeth/bin/starpost

stardsp: starchart.c starchartdsp.c
	cc $(CFLAGS) starchart.c starchartdsp.c -lm -o /u/awpaeth/bin/stardsp

all:	moonphase\
	planet\
	startek\
	starpic\
	starpost\
	stardsp

CFLAGS = -O

moonphase: moonphase.c
	cc $(CFLAGS) moonphase.c -lm -o ./moonphase

planet:	planet.c
	cc $(CFLAGS) planet.c -lm -o ./planet

startek: starchart.c starcharttek.c
	cc $(CFLAGS) starchart.c starcharttek.c -lm -o ./startek

starpic: starchart.c starchartpic.c
	cc $(CFLAGS) starchart.c starchartpic.c -lm -o ./starpic

starpost: starchart.c starchartpost.c
	cc $(CFLAGS) starchart.c starchartpost.c -lm -o ./starpost

stardsp: starchart.c starchartdsp.c
	cc $(CFLAGS) starchart.c starchartdsp.c -lm -o ./stardsp

all:	moonphase\
	planet\
	startek\
	starpic\
	starpost\
	stardsp

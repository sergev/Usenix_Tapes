CFLAGS = -O

stardsp: starchart.c starchartdsp.c
	cc $(CFLAGS) starchart.c starchartdsp.c -lIkonas -lm -o ./stardsp

starpic: starchart.c starchartpic.c
	cc $(CFLAGS) starchart.c starchartpic.c -lIkonas -lm -o ./starpic

startek: starchart.c starcharttek.c
	cc $(CFLAGS) starchart.c starcharttek.c -lIkonas -lm -o ./startek

test: starpic
	./starpic 5.6 -4 10 "Orion" >orion.pic

all:	stardsp\
	starpic\
	startek

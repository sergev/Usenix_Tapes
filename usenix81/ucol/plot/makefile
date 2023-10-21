CFLAGS = -n -O -s

all:	tek t300 t300s t450 vplot hplot
	:

cp:	all
	cp tek t300 t300s t450 vplot hplot /bin
	rm tek t300 t300s t450 vplot hplot
	rm *.o

cmp:	all
	cmp tek /bin/tek
	rm tek
	cmp t300 /bin/t300
	rm t300
	cmp t300s /bin/t300s
	rm t300s
	cmp t450 /bin/t450
	rm t450
	cmp vplot /bin/vplot
	rm vplot
	rm -f driver.o pread.o chrtab.o

tek:	driver.o flib
	cc -n -s -o tek  driver.o flib -lt4014 -lm
t300:	driver.o  flib
	cc -n -s -o t300 driver.o flib -lt300 -lm
t300s:	driver.o  flib
	cc -n -s -o t300s driver.o flib -lt300s -lm
t450:	driver.o  flib
	cc -n -s -o t450 driver.o  flib -lt450 -lm

vplot:	driver.o  flib
	cc -i -n -s -o vplot driver.o flib -ltver -lm

hplot:	driver.o flib  
	cc -i -o hplot driver.o flib -lthp -lHP -lF77 -lI77 -lm

flib:	hfonts.o hershyplot.o pread.o
	ar rv flib $?
	ranlib flib



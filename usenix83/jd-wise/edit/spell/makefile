CFLAGS = -O
objs=debug.o spell.o dict.o work.o
srcs=debug.c spell.c dict.c work.c
all:	spell hlist hlista hstop spellin
	:

install:	all
	cp spell /usr/lib/edit
	cp hlista /usr/lib/edit/hdict
	cp hstop /usr/lib/edit
	rm *.o spell hlista hstop hlist

spell: $(objs)
	cc -i $(objs) -o spell
dict.o:	dict.h
lint:	$(srcs)
	lint -hxa $(srcs)>lintfile
spellin: spellin.c dict.h
	cc -i -O spellin.c -o spellin
spellout: spellout.c
	cc -i -O -s spellout.c -o spellout

hlist: /usr/dict/words spellin
	spellin -d </usr/dict/words >hlist
hlista: american local hlist spellin
	(cat american local)|spellin -d hlist >hlista
hlistb: british local hlist spellin
	(cat british local)|spellin -d hlist >hlistb
hstop: stop spellin
	spellin -s <stop >hstop

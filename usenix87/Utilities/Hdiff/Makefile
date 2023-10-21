SRC = hdiff.c remwhite.c stripnl.c stripnl.h hdiff.mk
# also the hdiff help file is source but it is renamed on the copy

# change this for your site
INSTALLDIR = .


hdiff: hdiff.o remwhite.o stripnl.o
	cc -O hdiff.o remwhite.o stripnl.o
	mv a.out hdiff

hdiff.o: stripnl.h
	cc -O -c hdiff.c

remwhite.o: stripnl.h
	cc -O -USTAND -c remwhite.c

stripnl.o: stripnl.h
	cc -O -c stripnl.c

clean:
	rm -f hdiff.o remwhite.o stripnl.o hdiff

install: hdiff
	cp hdiff $(INSTALLDIR)

# distribute hdiff. personal for dennis only.
dist:
	rm -rf /tmp/dpb
	mkdir /tmp/dpb
	cp $(SRC) /tmp/dpb
	cp ../help/hdiff /tmp/dpb/hdiff.hlp	# help file
	(cd /tmp/dpb; make -f hdiff.mk makeshar)

makeshar:
	splitfiles *  	# split source files into little bundles
	for i in list.* ; \
	do \
		makeshar `cat $$i` > shar.$$i ; \
	done


# you must run make -f hdiff.mk makeshar first
# sends shar files to mod.sources
# hardcoded for 2 bundles
sendtonet:
	for i in 1 2 ; \
	do \
		Mail < shar.list.$$i -s "hdiff: - part $$i of 2" sources@panda.uucp; \
	done

# SUMacC makefile

DP=/usr/stanford/bin
DL=/usr/sun/lib
DI=/usr/include

# for Eunice, comment out the lines above and enable the lines below
#MACROOT=/usr/local/mac
#DP=$(MACROOT)/bin
#DL=$(MACROOT)/lib
#DI=$(MACROOT)/include

all:
	-mkdir $(DI) $(DI)/mac $(DP) $(DL)
	cp h/*.h $(DI)/mac
	cp man/*.1 /usr/man/man1
	cd cmd; make DP=$(DP) DL=$(DL) 
	cd cmd; make DP=$(DP) DL=$(DL) install
	cd lib; make DP=$(DP) DL=$(DL) 
	cd lib; make DP=$(DP) DL=$(DL) install
	cd mac; make DP=$(DP) DL=$(DL) 

clean:
	cd cmd; make clean
	cd lib; make clean
	cd mac; make clean

cleanback:
	cd h; rm -f *-
	cd cmd; rm -f *-
	cd lib; rm -f *-
	cd mac; rm -f *- */*-
	cd gw; rm -f *-

TAR= Makefile README cc41 cc42 cceun cmd etc fix \
 h lib mac man order sites sumacc.ms sumacc.mail 
OUT=/tmp/mac

tar1:
	tar cf $(OUT) ${TAR}

tar2:
	tar cf $(OUT) ws 

tartape:
	tar cf $(OUT) ${TAR} info ws ws-

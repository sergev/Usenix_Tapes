DV:	dlib2.c dlib2a.c dchols.r dcholn.r
	f77 -c -O dchols.r
	f77 -c -O dcholn.r
	cc -c -O dlib2.c
	ar cr DV dlib2.o dchols.o dcholn.o
	ranlib DV
	mv DV /usr/lib/libDV.a
	rm *.o

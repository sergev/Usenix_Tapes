all:		mapfile findpath findfile

mapfile:	mapfile.c
		cc -O mapfile.c -o mapfile -ldbm -s

findpath:	findpath.c
		cc -O findpath.c -o findpath -ldbm -s

findfile:	findfile.c psystem.c
		cc -O findfile.c psystem.c -o findfile -ldbm -s

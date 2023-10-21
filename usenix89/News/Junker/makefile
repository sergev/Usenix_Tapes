# For BSD sites:
bsdflag = -DBSD
# For non-BSD sites:
# bsdflag = 
junker:		first.c check.c junker.c
		cc -O $(bsdflag) -o junker junker.c
junker.d:	first.c check.c junker.c
		cc -g $(bsdflag) -Ddebugging -o junker.d junker.c
junker.cat:	junker.doc
		nroff -man junker.doc >junker.cat
all:		junker junker.d junker.cat

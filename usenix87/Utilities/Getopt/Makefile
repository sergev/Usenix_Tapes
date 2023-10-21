# Pick one
CFLAGS	= -O -DINDEX=index
#CFLAGS	= -O -DINDEX=strchr
# Pick one
M	= /usr/man/man1/getopt.1
#M	= /usr/man/u_man/man1/getopt.1#  Is this path right?
# Where executable ends up; don't forget the trailing /
D	= /bin/

getopt:		getopt.c
	$(CC) $(CFLAGS) -o getopt getopt.c
install:	getopt
	cp getopt $Dgetopt
	strip $Dgetopt
	cp getopt.1 $M

##  If on a BSD system, enable the next line.  Also, make arrangements to
##  get strtok(3) pulled in somehow.
#BSD	= -DBSD

##  If you want getopt, enable this line:
GETOPT	= -DUSE_GETOPT
##  You may have to have something like this line, if getopt isn't in libc.a
#GETOBJ	= -lgetopt
##  If no getopt, enable this line:
#SCAN	= scanargs.o

CFLAGS=$(BSD) $(GETOPT)

newscnt: newscnt.o $(SCAN)
	cc -o newscnt newscnt.o $(SCAN) $(GETOBJ)

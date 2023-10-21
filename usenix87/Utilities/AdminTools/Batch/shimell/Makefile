# Makefile for local libraries
# %W%
#

SCCS	=	.
BIN	=	/usr/local/lib

LIB	=	liblocal.a
CFLAGS	=	-O

#-------------------------------------------------------------------------------

all:		$(LIB)
		@echo programs are up to date

.DEFAULT:
		$(GET) $(GFLAGS) $(SCCS)/s.$<

rm clean tidy:

cp install:	$(LIB)
		-rm -f $(BIN)/$(LIB)
		cp $(LIB) $(BIN)/$(LIB)
		ranlib $(BIN)/$(LIB)

# The following is a list of files for BDS4.2.

$(LIB):		\
		$(LIB)(curdir.o)\
		$(LIB)(dname.o)\
		$(LIB)(fswitch.o)\
		$(LIB)(getopt.o)\
		$(LIB)(sname.o)\
		$(LIB)(error.o)\
		$(LIB)(quit.o)\
		$(LIB)(warning.o)\
		$(LIB)(salloc.o)\
		$(LIB)(ename.o)

# The following is a complete list of files.
# This will need editing for your system and
# moved above.

#$(LIB):		
#
#		$(LIB)(curdir.o)\
#		$(LIB)(dname.o)\
#		$(LIB)(fswitch.o)\
#		$(LIB)(getopt.o)\
#		$(LIB)(rename.o)\
#		$(LIB)(sname.o)\
#		$(LIB)(umask.o)\
#		$(LIB)(ndir.h)\
#		$(LIB)(opendir.o)\
#		$(LIB)(closedir.o)\
#		$(LIB)(readdir.o)\
#		$(LIB)(seekdir.o)\
#		$(LIB)(telldir.o)\
#		\
#		$(LIB)(error.o)\
#		$(LIB)(quit.o)\
#		$(LIB)(warning.o)\
#		$(LIB)(salloc.o)\
#		$(LIB)(ename.o)\

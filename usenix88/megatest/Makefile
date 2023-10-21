
# This makefile uses several features peculiar to SunOS-3.  
# If you have some other kind of "make", you will need to write another
# makefile.

FILES =  $$@(idents.o) $$@(_handler.o) $$@(queue.o) $$@(heap.o) \
	$$@(Assoc.o) $$@(flex_vector.o)

LIB= libCdbj.a

.SUFFIXES: .C .a .c .o .h .H

all: rm_deps replace $(LIB)

$(LIB): $(FILES)

#
# The following two rules make archive-entries from .C and .c files.
# See the manual Sun Rel 3.2, vol 7., Chap. 3, p. 64.
#
# They also update the dependancy-list, kept in the file
# "depend", which is included at the bottom of this makefile. The technique,
# (which could be improved), is as follows:
#
#    1. Remove the old dependancies and put the result in Stripped_depend.
#    2. Put new dependancies of the form 
#           foo.o: bar.h
#       into Incr_depend, using the -M option to cpp.
#    3. Change the dependancy to this form:
#           library.a(foo.o): bar.h
#    4. Concatenate Stripped_depend and Incr_depend into depend.
#
.C.a:
	@rm_deps $@\($*.o\) < depend > Stripped_depend; \
	/lib/cpp -M -I/usr/include/CC $< > Incr_depend; \
	replace $*.o $@\($*.o\) Incr_depend > /dev/null ; \
	cat Stripped_depend Incr_depend > depend; \
	/usr/bin/CC $(CFLAGS) -c $< ;  \
	ar r $@ $*.o; rm -f $*.o; \
	ranlib $@ ; rm Stripped_depend Incr_depend

.c.a:
	@rm_deps $@\($*.o\) < depend > Stripped_depend; \
	/lib/cpp -M  $< > Incr_depend; \
	replace $*.o $@\($*.o\) Incr_depend > /dev/null ; \
	cat Stripped_depend Incr_depend > depend; 
	cc $(CFLAGS)  -c $*.c
	@ar r $@ $*.o; rm -f $*.o; \
	ranlib $@ ; rm Stripped_depend Incr_depend

# This rule is for making programs from single source files.
#  CAVEAT:  It knows nothing about include-files.
.C:
	/usr/bin/CC $(CFLAGS) $@.C $(LIB) -o $@ ; rm -f $@.o


replace: replace.c
	cc replace.c -o replace

rm_deps: rm_deps.C Assoc.C heap.C Assoc.H heap.H
	/usr/bin/CC rm_deps.C Assoc.C heap.C -o rm_deps
	rm rm_deps.o Assoc.o

clean:
	rm -f *.o $(LIB) *..c temp t foo.* bar.* depend Stripped_depend \
	  Incr_depend ; \
	echo "" > depend

include depend

# Makefile for g++ library version 1.20.0

# Copyright (C) 1988 Free Software Foundation
#   written by Doug Lea (dl@rocky.oswego.edu)

# This file is part of GNU CC.

# GNU CC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.  No author or distributor
# accepts responsibility to anyone for the consequences of using it
# or for whether it serves any particular purpose or works at all,
# unless he says so in writing.  Refer to the GNU CC General Public
# License for full details.

# Everyone is granted permission to copy, modify and redistribute
# GNU CC, but only under the conditions described in the
# GNU CC General Public License.   A copy of this license is
# supposed to have been given to you along with GNU CC so you
# can know your rights and responsibilities.  It should be in a
# file named COPYING.  Among other things, the copyright notice
# and this notice must be preserved on all copies.


# you must have write permission to the following directory that
# will hold libg++.a and g++-include. 
LIBDIR = /usr/local/lib

# location of include file directories 
DISTI = ../incl
# I = $(LIBDIR)/g++-include
I=../incl

GXX = g++

# g++ compliation flags:
# note that -O flag is mandatory if using g++ on vax, etc.
# the fchar-charconst flag is mandatory for some stream ops to work sensibly
# -fkeep-inline-functions forces all inlines to be written as
#   provided library functions too.

GFLAGS= -g -O -I$I -fchar-charconst -fkeep-inline-functions

TSTFLAGS = -g -O -I$I -fchar-charconst

# use gcc for archived C files
CC = gcc
# -O causes a ld++ problem when compiling regex.c
CFLAGS = -g -I$I

# compilation notes:
# ONLY the following g++ warnings are expected to appear:
#
# 1. in String.cc Regex::Regex, several warnings about passing
#    const char* as char*. This is because the non-const declaration
#    of regexp fct char* args is required by regex.c. I did not want
#    to touch the regex.c code, so that any updates from gnu emacs
#    version can used here by only adding function prototypes to
#    regex.h, and not touching regex.c.


# g++ library sources 
SRCS = File.cc stream.cc PlotFile.cc SFile.cc Obstack.cc regex.c String.cc Integer.cc 

OBJS = File.o  stream.o  PlotFile.o  SFile.o  Obstack.o  regex.o String.o  Integer.o 


# standard C compatibility header files
STDH = $I/std.h $I/stddef.h $I/stdio.h $I/math.h $I/stdarg.h $I/regex.h

# g++ library class declaration files

GXXH = $I/File.h $I/stream.h $I/SFile.h $I/PlotFile.h $I/Obstack.h $I/String.h $I/Integer.h 

# test files
TSRCS = test.hello.cc test.bye.cc test.bye2.cc test.shell.cc test1.cc test2.cc test3.cc test4.cc
TOUTS = test0 test.bye test.bye2 test.shell test1 test2 test3 test4

# all g++ files should have extension .cc
.SUFFIXES: .cc
.cc.o:
	$(GXX) $(GFLAGS) -c  $<


all:  test0 installincl libg++.a tests

installincl:
#	cp -r $(DISTI) $I


# test0 should be run first to check g++, ld++, crt0/1 and std includes

test0: test.hello.o test.bye test.bye2 test.shell dummylibg++.a
	install -c dummylibg++.a $(LIBDIR)/libg++.a
	ranlib $(LIBDIR)/libg++.a
	$(GXX) test.hello.o -o test0
	@echo "please run test0 and follow instructions"

test.hello.o: test0.h test.hello.cc
	$(GXX) -c $(GFLAGS) test.hello.cc -o test.hello.o

test.bye: test0.h test.bye.cc
	$(GXX) -c $(GFLAGS) test.bye.cc -o test.bye

test.bye2: test0.h test.bye2.cc
	$(GXX) -c $(GFLAGS) test.bye2.cc -o test.bye2

test.shell: test0.h test.shell.cc
	$(GXX) -c $(GFLAGS) test.shell.cc -o test.shell

dummylibg++.a: dummylibg++.cc  $(STDH)
	$(GXX) -c $(GFLAGS) dummylibg++.cc
	ar r dummylibg++.a dummylibg++.o

    

libg++.a: $(STDH) $(GXXH) $(OBJS)
	ar r libg++.a $(OBJS)
	install -c libg++.a $(LIBDIR)
	ranlib $(LIBDIR)/libg++.a


tests: libg++.a test1 test2 test3 test4
	@echo "now run test1, test2 ... to check implementation"


test1: test1.cc
	$(GXX) $(TSTFLAGS) test1.cc  -o test1

test2: test2.cc
	$(GXX) $(TSTFLAGS) test2.cc  -o test2

test3: test3.cc
	$(GXX) $(TSTFLAGS) test3.cc  -o test3

test4: test4.cc
	$(GXX) $(TSTFLAGS) test4.cc  -o test4

clean:
	rm -f *.o core libg++.a dummylibg++.a $(TOUTS) plot.out


# dependencies

File.o:     $I/File.h $I/std.h $I/stddef.h $I/stdio.h
stream.o:   $I/stream.h $I/File.h $I/stdio.h
PlotFile.o: $I/PlotFile.h $I/File.h $I/stdio.h
SFile.o:    $I/SFile.h
Obstack.o:  $I/Obstack.h
String.o:   $I/String.h $I/stream.h $I/regex.h $I/std.h
Integer.o:  $I/Integer.h $I/stream.h $I/File.h

regex.o:    $I/regex.h



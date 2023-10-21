#makefile for the display system

all : message gone dispd whatmess

message : message.o dispd.h
	cc -o message message.o
gone : gone.o dispd.h
	cc -o gone gone.o
whatmess : whatmess.o dispd.h
	cc -o whatmess whatmess.o
dispd : dispd.o dispd.h
	cc -o dispd dispd.c -lcurses -ltermlib

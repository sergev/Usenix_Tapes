#Copyright (c) 1981 Gary Perlman  All rights reserved
# Remove all the .o files before starting
# Be sure to change menudir (where all menus are kept) in menu.defs (sorry)
OBJS = menu.o init.o internalrun.o file.o display.o run.o input.o linedit.o utility.o alias.o image.o movecursor.o
LIBES = -lcurses -ltermlib
DESTDIR = .
menu: $(OBJS) menu.defs
	cc $(OBJS) $(LIBES)
	mv -i a.out $(DESTDIR)/menunix
$(OBJS): menu.h
menu.o: menu.defs

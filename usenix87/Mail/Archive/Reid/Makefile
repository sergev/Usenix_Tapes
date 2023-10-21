configure:
	sh configure.sh

date+: date+.c
	cc -o date+ date+.c

install: date+
	sh install.sh

bundle:
	shar README list Makefile *.sh *.X *.c \
		> archive-system.shar

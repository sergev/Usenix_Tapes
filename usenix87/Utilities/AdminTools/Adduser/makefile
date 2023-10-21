P=| ci -c17 -o10
BIN=/usr/bin
ETC=/etc
all: pwentry.c
	pcc -O -o pwentry pwentry.c
	pcc -O -o pwsort pwsort.c

install: 
	cp pwentry.8 /usr/man/man8/pwentry.8
	cp pwentry $(BIN)/pwentry
	cp pwmakedir $(BIN)/pwmakedir
	chmod 4110 $(BIN)/pwentry
	chmod 4110 $(BIN)/pwmakedir
	cp /dev/null $(ETC)/tempfile
	chmod 755 $(ETC)/tempfile

print: README makefile pwentry.8 pwentry.c pwmakedir pwsort.c
	pr `ls $?` $P
	-touch print

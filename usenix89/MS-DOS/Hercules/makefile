# Makefile for HERCBIOS.COM
SRC=hercbios gchar graph
DEST=a:

hercbios.com : hercbios.obj gchar.obj graph.obj
	link hercbios gchar graph,,/MAP;
	exe2bin  hercbios.exe  hercbios.com
	del hercbios.exe

hercbios.obj : hercbios.asm hercbios.h
gchar.obj : gchar.asm hercbios.h
graph.obj : graph.asm hercbios.h


# Makes for the demo & test program, using deSmet C

testpix.exe : testpix.o hercpixl.o
	bind testpix hercpixl

testpix.o : testpix.c
	c88 testpix

hercpixl.o : hercpixl.c
	c88 hercpixl

# Make a backup or distribution disk
backup :
	for %f in ($(SRC)) do  copy %f.asm $(DEST)
	copy hercbios.h   $(DEST)
	copy hercbios.com $(DEST)
	copy hercbios.doc $(DEST)
	copy makefile $(DEST)

distrib :
	make backup
	copy hercpixl.c $(DEST)
	copy testpix.* $(DEST)
	copy hcharset.* $(DEST)

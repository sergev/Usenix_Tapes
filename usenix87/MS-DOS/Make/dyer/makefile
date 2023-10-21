!
! MSDOS Make utility
! (compile with Lattice C version 2.1)
!

CLIB = e:\lc\s\lc
COBJ = e:\lc\s\c
LCS = lc -ms -ie:\lc\
H = make.h
FILES = #H make.c macro.c token.c parsedir.c file.c osdate.asm
DOCUMENTATION = readme make.man makefile

xmake.exe : make.obj macro.obj token.obj parsedir.obj file.obj osdate.obj
	link #(COBJ) make macro token parsedir file osdate,xmake,nul,#(CLIB)

make.obj : make.c #H
	#(LCS) make

macro.obj : macro.c #H
	#(LCS) macro

token.obj : token.c #H
	#(LCS) token

parsedir.obj : parsedir.c #H
	#(LCS) parsedir

file.obj : file.c
	#(LCS) file

osdate.obj : osdate.asm
	masm osdate;

!
! Print files associated with MAKE
!
print :
	print make.man #(FILES) makefile


!
! collect source and documentation files
!
collect :
	collect -o make.col @make.lis


!
! copy to distribution disk (on A:)
!
distribution :
	copy readme a:
	copy make.man a:
	copy makefile a:
	copy make.bat a:
	copy make.c a:
	copy macro.c a:
	copy token.c a:
	copy parsedir.c a:
	copy file.c a:
	copy osdate.asm a:
	copy cmake.bat a:
	copy make.lis a:
	copy xmake.exe a:

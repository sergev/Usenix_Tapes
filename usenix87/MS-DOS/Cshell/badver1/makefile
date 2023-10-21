# home for all commands
BINDIR=\\bin\\
# small model library path
CLIB=-l/clibs/c

# source files for shell
shsrc=main.c cd.c cp.c doprog.c fexecvp.c more.c cmds.c chmod.c y.c fexec.asm \
invalid.c ls.c pushd.c fgrep.c md.c mv.c pwd.c rm.c crlf.c drive.c dump2.c \
rmdir.c savestr.c stdsave.c touch.c _croot.c env.c fexecv.c cat.c echo.c \
cmdlist.c ctlbrk.asm mvcp.c

# object files for shell
shobj=main.o cd.o cp.o doprog.o fexecvp.o more.o cmds.o chmod.o y.o fexec.o \
invalid.o ls.o pushd.o fgrep.o md.o mv.o pwd.o rm.o crlf.o drive.o dump2.o \
rmdir.o savestr.o stdsave.o touch.o _croot.o env.o fexecv.o cat.o echo.o \
cmdlist.o ctlbrk.o mvcp.o

main.c : main.s
	statecom main

#
# make shell - check currency of all percursors, link and then move to
# bin directory
#
shell.exe : $(shobj) 
	ln -t -o $@ -f shell.lnk
	\\atron\\aztoat <shell.sym >shell.map
	chmod +w $(BINDIR)$@
	mv $@ $(BINDIR)$@
	chmod -w $(BINDIR)$@

#
# make entries to produce stand-alone versions of shell builtins
#
chmod.com : 
	cc -dMAIN $*
	ln -o $@ $*.o -l/clibs/c
	rm $*.o
	chmod +w $(BINDIR)chmod.com
	mv chmod.com $(BINDIR)chmod.com
	chmod -w $(BINDIR)chmod.com

more.com : 
	cc -dMAIN $*
	ln -o $@ $*.o -l/clibs/s -l/clibs/c
	rm $*.o
	chmod +w $(BINDIR)$@
	mv $@ $(BINDIR)$@
	chmod -w $(BINDIR)$@

mv.com : croot.o savestr.o mvcp.o
	cc -dMAIN $*
	ln -o $@ $*.o savestr.o croot.o mvcp.o  -l/clibs/c
	rm $*.o
	chmod +w $(BINDIR)$@
	mv $@ $(BINDIR)$@
	chmod -w $(BINDIR)$@

cp.com : croot.o savestr.o mvcp.o
	cc -dMAIN $*
	ln -o $@ $*.o savestr.o croot.o  mvcp.o -l/clibs/c
	rm $*.o
	chmod +w $(BINDIR)$@
	mv $@ $(BINDIR)$@
	chmod -w $(BINDIR)$@

ls.com : 
	cc -dMAIN $*
	ln -o $@ $*.o -l/clibs/c
	rm $*.o
	chmod +w $(BINDIR)$@
	mv $@ $(BINDIR)$@
	chmod -w $(BINDIR)$@
editall :
	z $(shsrc)
clean :
	rm *.o *.bak *.sym

# Makefile for OSK MicroGnuEMACS by Robert A. Larson
#	07/12/86	MicroGnuEMACS version
# Copy ./sys/osk/makefile to . before making.  Check
# OPTS and LIBS.
#
# mg is short for micrognuemacs.  Call it what you want.
NAME = mg
# terminal type or termcap
TERM = TERMCAP
# opts is for user definable options, such as:
#	BACKUP	define for automatic backups
#	CVMVAS	define for ^V and M-V to work in pages
#	MAXMEM	memory available to malloc in kbytes.  Default 512 (OSK only)
#	NLINE	maximum line length, default 256.  If big, use -m in LFLAGS.
#	DPROMPT	print delayed prompt strings for ESC, ^X, etc.
#	STARTUP startup file handling
#	PREFIXREGION	functions for prefixing a region
OPTS = -dBACKUP -dMAXMEM=320 -dNLINE=2048 -dDPROMPT -dSTARTUP
# termlib is needed for termcap.
LFLAGS = -l=/dd/lib/termlib.l -m=4
OBJ = basic.r \
 buffer.r \
 cinfo.r \
 display.r \
 echo.r \
 extend.r \
 file.r \
 kbd.r \
 line.r \
 main.r \
 match.r \
 paragraph.r \
 prefix.r \
 random.r \
 region.r \
 search.r \
 symbol.r \
 version.r \
 window.r \
 word.r
# these files are listed individually below
SYSOBJ = SYS/OSK/fileio.r \
 SYS/OSK/spawn.r \
 SYS/OSK/ttyio.r
# ditto
TERMOBJ = TTY/$(TERM)/tty.r \
 TTY/$(TERM)/ttykbd.r
#
$(NAME): $(OBJ) $(SYSOBJ) $(TERMOBJ) makefile
 cc -i -t=/r0 -f=$(NAME) $(LFLAGS) $(OBJ) $(SYSOBJ) $(TERMOBJ)
$(OBJ): def.h sysdef.h ttydef.h makefile
 cc -r -t=/r0 $(OPTS) $*.c
SYS/OSK/fileio.r: SYS/OSK/fileio.c def.h sysdef.h ttydef.h makefile
 cc -r=SYS/OSK -t=/r0 $(OPTS) SYS/OSK/fileio.c
SYS/OSK/spawn.r: SYS/OSK/spawn.c def.h sysdef.h ttydef.h makefile
 cc -r=SYS/OSK -t=/r0 $(OPTS) SYS/OSK/spawn.c
SYS/OSK/ttyio.r: SYS/OSK/ttyio.c def.h sysdef.h ttydef.h makefile
 cc -r=SYS/OSK -t=/r0 $(OPTS) SYS/OSK/ttyio.c
TTY/$(TERM)/tty.r: TTY/$(TERM)/tty.c def.h sysdef.h ttydef.h makefile
 cc -r=TTY/$(TERM) -t=/r0 $(OPTS) TTY/$(TERM)/tty.c
TTY/$(TERM)/ttykbd.r: TTY/$(TERM)/ttykbd.c def.h sysdef.h ttydef.h makefile
 cc -r=TTY/$(TERM) -t=/r0 $(OPTS) TTY/$(TERM)/ttykbd.c
# The touch commands are needed to update the modified time.
sysdef.h: SYS/OSK/sysdef.h
 copy -r -b=16 -w=. SYS/OSK/sysdef.h
 touch sysdef.h
ttydef.h: TTY/$(TERM)/ttydef.h makefile
 copy -r -b=16 -w=. TTY/$(TERM)/ttydef.h
 touch ttydef.h

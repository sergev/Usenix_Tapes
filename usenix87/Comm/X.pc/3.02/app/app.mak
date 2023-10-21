#
#	Makefile for X.PC Application using Microsoft's Make utility.
#
INCDIR=..\INC
CFLAGS=-i$(INCDIR)\ -oobj\ -n -v
AFLAGS=/a /i$(INCDIR)
LIBDIR=\lib


obj\abortc.obj:		ABORTC.ASM $(INCDIR)\DOS.MAC
	masm abortc.asm $(AFLAGS), obj\abortc.obj; 

obj\cio_ctrl.obj:	CIO_CTRL.ASM
	masm cio_ctrl.asm $(AFLAGS), obj\cio_ctrl.obj; 

obj\p_aplint.asm:	P_APLINT.ASM
	masm p_aplint.asm $(AFLAGS), obj\p_aplint.obj; 

obj\p_appfil.obj:	P_APPFIL.C P_PADFNC.H APP.H 
	lc $(CFLAGS) p_appfil 

obj\p_appl.obj:		P_APPL.C P_PADFNC.H APP.H
	lc $(CFLAGS) p_appl

obj\p_applcl.obj:	P_APPLCL.C P_PADFNC.H APP.H
	lc $(CFLAGS) p_applcl

obj\p_applio.obj:	P_APPLIO.C P_PADFNC.H APP.H
	lc $(CFLAGS) p_applio

obj\p_applpt.obj:	P_APPLPT.C P_PADFNC.H APP.H
	lc $(CFLAGS) p_applpt

obj\p_applut.obj:	P_APPLUT.C P_PADFNC.H
	lc $(CFLAGS) p_applut

obj\scr_ctrl.obj:	SCR_CTRL.C
	lc $(CFLAGS) scr_ctrl

app.exe:	 obj\abortc.obj obj\cio_ctrl.obj obj\p_aplint.obj \
	obj\p_appfil.obj obj\p_appl.obj obj\p_applcl.obj obj\p_applio.obj \
	obj\p_applpt.obj obj\p_applut.obj obj\scr_ctrl.obj
	link @app.lnk,app,nul,$(LIBDIR)\lcs.lib; 

/*	SCAME macro.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

#include "scame.h"

definekbdmac(flg)
Bool flg;
{
	if (flg) {
		if (defining_kbd_mac) errmes(RKM);
		else {
			if ((kbdmfd = creat(kbdmacfile, 0600)) >= 0)
				defining_kbd_mac = TRUE;
			else echo("can't create tempfile!");
		}
	}
	else if (defining_kbd_mac) {
		close(kbdmfd);
#ifdef BSD42
		truncate(kbdmacfile, (int) filesize(kbdmacfile)-2);
#endif
		defining_kbd_mac = FALSE;
	}
	modeline();
}

savekbdmac(fname)
char *fname;
{
	if (fileexists(kbdmacfile))
		copyfile(kbdmacfile, fname);
	else errmes(NKM);
}

loadkbdmac(fname)
char *fname;
{
	if (fileexists(fname))
		copyfile(fname, kbdmacfile);
	else errmes(NSF);
}

execfile(fn)
char *fn;
{
char cc, fname[FILENAMESIZE];
off_t fz;
int tfd, c;
	tfd = execfd;
	sprintf(fname,fn);
	if ((execfd=open(fname,0)) < 0) {
		sprintf(fname,"%s/%s",getenv("HOME"),fn);
		if ((execfd=open(fname,0)) < 0) {
			sprintf(fname,"%s/%s",SCAMELIB,fn);
			execfd=open(fname,0);
		}
	}
	if (execfd < 0 || (fz = filesize(fname)) == 0) return;
	quiet = TRUE;
	editloop();
	close(execfd);
	quiet = FALSE;
	execfd = tfd;
}


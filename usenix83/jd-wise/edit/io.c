/*
 * file io routines
 */

#include "ed.h"
#include "window.h"
#include "edit.h"
#include "file.h"

exfile()
	{
	close(io);
	io = -1;
	}

filename(fn)
	{
	register char *p1, *p2;
	register c;
	struct filedata *fp;

	fp = &filedata[fn];
	count[1] = 0;
	c = getchr();
	if (c=='\n' || c==EOF) {
		p1 = fp->savedfile;
		if (*p1==0)
			errfunc("No remembered file name");
		p2 = file;
		while (*p2++ = *p1++);
		return;
		}
	if (c!=' ')
		errfunc("Bad file name format");
	while ((c = getchr()) == ' ');
	if (c=='\n' || c==EOF)
		errfunc("No file name given");
	p1 = file;
	do {
		*p1++ = c;
		} while ((c = getchr()) != '\n' && c!=EOF);
	*p1++ = 0;
	if (fp->savedfile[0]==0) {
		p1 = fp->savedfile;
		p2 = file;
		while (*p1++ = *p2++);
		}
	filemode = (stat(file,&statbuf) < 0 ? FILEMODE : statbuf.st_mode);
	}

getfile()
	{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				return(EOF);
			fp = genbuf;
			}
		if (lp >= &linebuf[LBSIZE])
			errfunc("Input line too long");
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
			}
		if (++count[1] == 0)
			++count[0];
		} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
	}

putfile(wp)
	struct window *wp;
	{
	int *a1;
	register char *fp, *lp;
	register nib;
	int j;

	j=alarm(0);
	nib = 512;
	fp = genbuf;
	a1 = addr1.ad_addr;
	do {
		lp = getline(wp->wi_fileno, *a1++);
		for (;;) {
			if (--nib < 0) {
				write(io, genbuf, fp-genbuf);
				nib = 511;
				fp = genbuf;
				}
			if (++count[1] == 0)
				++count[0];
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
				}
			}
		} while (a1 <= addr2.ad_addr);
	write(io, genbuf, fp-genbuf);
	alarm(j?j:30);
	}

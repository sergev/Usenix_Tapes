
#include "hd.h"
#include "strings.h"
#include "command.h"
#include "classify.h"
#include "mydir.h"

#ifndef	NDIR
extern char **d_namep;
#else
extern struct direct **d_dirp;
#endif
extern char quitcmds[];

find(argv)
char **argv;
{
	register int c;

	if (*argv == CNULL) {
		putmsg("Find: ");
		c = getch();
		putch(c);
	}
	else
		c = **argv;
	if (c == 0 || c == EOF || any(c, quitcmds) != NULL) {
		clearmsg(1);
		return NOREPLOT;
	}
	if (*argv == CNULL)
		printf("'s");
	return findc(c, 1);
}

findc(c, forw)
register int c;
register int forw;
{
	register int i;

	for (i = 0; i < tfiles; i++)
#ifndef	NDIR
		if (c <= d_namep[i][0]) {
#else
		if (c <= d_dirp[i]->d_name[0]) {
#endif
			if (!forw && i)
				i--;
			return findfile(i);
		}
	return findfile(tfiles-1);
}

findfile(index)
register int index;
{
	register int i, j, f;

	i = (index+nfpp)/nfpp;
	j = index%nfpp;
	if ((column < 2 && i != cpage) || i < cpage || i > cpage+column-1) {
		pageoff = 0;
		cpage = i;
		*selectname = 0;
		dispdir(0);
		if (VSHMODE == SELECTMODE)
			dircmd(j+'a');
	}
	else {
		f = (cpage+pageoff > i);
		while (cpage+pageoff != i)
			dircmd(f ? ESLF : TABCMD);
		f = (selectcmd < j);
		if (VSHMODE == SELECTMODE)
			while (*selectname == 0 || selectcmd != j)
				dircmd(f ? ESDN : ESUP);
	}
	return NOREPLOT;
}

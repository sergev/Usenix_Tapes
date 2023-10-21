/* stream I/O for cpm program */

# include <stdio.h>
# include "cpm.h"

struct acon conv;	/* for stream buffering */
struct fcb rloc;	/* floppy file location on input */
extern struct fcb dir[];
extern int cfd;
extern int sat[];



copen(name, mode, contyp)
char *name;
char mode;
char contyp;
{
	int c, found, slot;
	char *v[ARG];
	char rmbuf[20];
	char cn[12];

	if (cpmname(cn, name) == -1)
		return -1;
	slot = slotnum(cn, -1);
	found = (slot >= 0);
	switch(mode) {

	case 'r':
		if (! found) {
			fprintf(stderr, "%.8s.%.3s not found\n", cn, &cn[8]);
			return -1;
		}
		conv.ccnt = 0;
		strcpy(rloc.fn, cn);
		rloc.ex = 0;
		rloc.rc = 0;
		break;

	case 'w':
		if (found) {		/* purge */
			strcpy(rmbuf, "rm ");
			strcat(rmbuf, cn);
			c = getarg(rmbuf, v, ARG);
			rm(c, v);
		}
		if ((slot = addext(cn, 0)) == -1)
			return -1;
		conv.ccnt = IOBUF;
		conv.cdir = &dir[slot];
		break;

	default:
		err(SREV);
		return -1;

	}
	conv.cmod = mode;
	conv.ccvt = contyp;
	conv.cptr = conv.cbuf;
	return cfd;
}



cclose()
{
	if (conv.cmod == 'w' && conv.cptr-conv.cbuf > 0) {
		cflsbuf(0);
		putdir();
	}
	return;
}



cflsbuf(c)	/* 1 full group written */
unsigned c;
{
	int offset, slot, group, recs;

	recs = (conv.cptr-conv.cbuf+127)/128;	/* 0 to 8 */
	if (recs) {				/* something to dump */
		if (conv.cdir->rc % 8)
			err(SREV);
		offset = conv.cdir->rc/8;
		if (offset > 15) {		/* time for new extent */
			offset = 0;
			slot = addext(conv.cdir->fn, conv.cdir->ex+1);
			if (slot == -1)
				return;
			conv.cdir = &dir[slot];
		}
		if ((group = newgroup()) == -1)
			return;
		conv.cdir->dm[offset] = group;
		conv.cdir->rc += recs;
		grpio(group, conv.cbuf, 1);
	}
	conv.cptr = conv.cbuf;
	conv.ccnt = IOBUF-1;
	*conv.cptr++ = c;
}



cfilbuf()
{
	int count, slot;

	slot = slotnum(rloc.fn, rloc.ex);
	while (rloc.rc >= dir[slot].rc) {
		if ((slot = slotnum(rloc.fn, rloc.ex+1)) == -1)
			return EOF;
		rloc.ex++;
		rloc.rc = 0;
	}
	conv.cptr = conv.cbuf;
	grpio(dir[slot].dm[rloc.rc/8], conv.cbuf, 0);
	count = (dir[slot].rc >= 8 ? 8 : dir[slot].rc);
	rloc.rc += count;
	conv.ccnt = (count*128)-1;
	return(*conv.cptr++ & 0377);
}



addext(name, extent)
char *name;
int extent;
{
	register int slot, i;

	for (slot = 0; slot < SLOTS; slot++)
		if (dir[slot].et == MDEL) {
			dir[slot].et = 0;
			strcpy(dir[slot].fn, name);
			dir[slot].ex = extent;
			dir[slot].rc = 0;
			for (i = 0; i < 16; i++)
				dir[slot].dm[i] = 0;
			return slot;
		}
	err("cpm: no more room in directory\n");
	return -1;
}



newgroup()
{
	register int grpindex;

	buildsat();
	for (grpindex = 2; grpindex < GROUPS; grpindex++)
		if (sat[grpindex] == 0) {
			sat[grpindex] = 1;
			return grpindex;
		}
	err("cpm: no more room on disk\n");
	return -1;
}



tocpm(ufp)
FILE *ufp;
{
	register int c;

	switch(conv.ccvt) {

	case 'a':	/* ASCII transfer */
		while ((c = getc(ufp)) != EOF)
			switch (c) {

			case '_':	/* possible continued line */
				if ((c = getc(ufp)) != '\n') {
					cputc('_');
					ungetc(c, ufp);
				}
				else {
					cputc('\n'); cputc('\r'); cputc('\0');
				}
			break;

			case '\n':
				cputc('\r'); cputc('\n');
				break;

			default:
				cputc(c);

			}
		cputc(CNTLZ);
		return;

	case 'b':	/* binary transfer */
		while ((c = getc(ufp)) != EOF)
			cputc(c);
		return;

	default:
		err(SREV);
		return;
	}
}



tounix(ufp)
FILE *ufp;
{
	register int c;

	switch(conv.ccvt) {

	case 'a':	/* ASCII transfer */
		while ((c = cgetc()) != CNTLZ && c != EOF) {
			switch (c) {

			case '\r':	/* <cr> <lf> */
				if ((c = cgetc()) != '\n') {
					putc('\r', ufp); putc(c, ufp);
				}
				else
					putc('\n', ufp);
				break;

			case '\n':	/* <lf> <cr> <null> */
				if ((c = cgetc()) != '\r') {
					putc('\n', ufp); putc(c, ufp);
				}
				else {
					putc('_', ufp); putc('\n', ufp);
				}
				break;

			case '\0':	/* eat up nulls */
				break;

			default:	/* normal char */
				putc(c, ufp);
				break;

			}
		}
		return;

	case 'b':	/* binary transfer */
		while ((c = cgetc()) != EOF)
			putc(c, ufp);
		return;

	default:
		err(SREV);
		return;
	}
}

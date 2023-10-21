/*
 * Copyright 1986 by Larry Campbell, 73 Concord Street, Maynard MA 01754 USA
 * (maynard!campbell).  You may freely copy, use, and distribute this software
 * subject to the following restrictions:
 *
 *  1)	You may not charge money for it.
 *  2)	You may not remove or alter this copyright notice.
 *  3)	You may not claim you wrote it.
 *  4)	If you make improvements (or other changes), you are requested
 *	to send them to me, so there's a focal point for distributing
 *	improved versions.
 *
 * John Chmielewski (tesla!jlc unti< 9/1/86, then rogue!jlc) assisted
 * by doing the System V port and adding some nice features.  Thanks!
 */

#include <stdio.h>
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#include "x10.h"

extern char flag, hc2char();
extern struct nstruct dtab[];
extern struct hstruct housetab[];
extern struct id id[];

/*
 * print unit numbers as specified in bitmap
 * bitmap is jammed into int backwards from X10 manual depiction:
 *
 *	 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
 *
 * instead of
 *
 *	 9 10 11 12 13 14 15 16  1  2  3  4  5  6  7  8 
 */

punits(bitmap)
unsigned bitmap;
{
register nf, unitno, saveno, seq;
char c;

saveno = -1;
for (unitno = 1, nf = seq = 0; unitno <= 16; unitno++)
    if ((1 << (16 - unitno)) & bitmap)
	{
	if (seq++) c = '-';
	else c = ',';
	if (saveno + 1 != unitno)
	    {
	    if (nf++) (void) printf("%c", c);
	    if (seq == 1) (void) printf("%d", unitno);
	    else (void) printf("%d,%d", saveno, unitno);
	    seq = 0;
	    }
	saveno = unitno;
	}
if (seq) (void) printf("-%d", saveno);
}

/*
 * Names must have a capitalized first letter for mode2code().
 * C_schedule() expects the first two modes listed to require
 * an argument of specific days.
 */

struct nstruct
    modnames[] =
	{
	"Normal",   0x08,
	"Security", 0x09,
	"Today",    0x04,
	"Tomorrow", 0x02,
	"",         0<00
	},
    funcnames[] =
	{
	"On",  0x02,
	"Off", 0x03,
	"DIM", 0x04,
	"Dim", 0x05,
	"",    0x00
	};

/*
 * Print out an event
 */

pevent(p, i)
unsigned i;
unsigned char p[];
{
char *modestr, *funcstr, *daystr, levelstr[8];
int j, mode, func;

/* decode mode */
mode = p[0] & 0x0F;
for (j = 0, modestr = NULL; modnames[j].n_code != 0; j++)
    if (modnames[j].n_code == mode)
	modestr = modnames[j].n_name;
if (modestr == NULL)
    modestr = "Unknown";

/* decode state */
func = p[7] & 0x0F;
for (j = 0, funcstr = NULL; funcnames[j].n_code != 0; j++)
    if (funcnames[j].n_code == func)
	funcstr = funcnames[j].n_name;
if (funcstr == NULL)
    funcstr = "Unknown";
if (func == 4 || func == 5)
    {
    (void) strcpy(levelstr, funcstr);
    (void) sprintf(levelstr+3, " %2d", (p[7] >> 4) & 0x0f);
    funcstr = levelstr;
    }

/* decode days */
for (j = 0, daystr = NULL; dtab[j].n_code != 0; j++)
    if (dtab[j].n_code == p[1])
	daystr = dtab[j].n_name;

/* print timer event header first */
if (!flag)
    (void) printf(
	"   EVENT   STATE    MODE       DAYS        TIME     UNITS\n");
flag++;		/* indicate header was printed */

/* print timer event */
(void) printf("     %3d   %-7s  %-8s   %-9s   %2d:%02d    %c",
       i, funcstr, modestr, daystr, p[2], p[3], hc2char(p[6]));
punits((p[4] << 8) | p[5]);
(void) printf("\n");
}

/*
 * Print out graphics data
 */

pdata(p, i)
unsigned i;
unsigned char p[];
{
char hletter, ucode, *state, icode;
unsigned char hcode;
int j;

hcode = p[0] & 0xf0;
for (j = 0; housetab[j].h_code != hcode; j++);
hletter = housetab[j].h_letter;
ucode = (p[0] & 0x0f) + 1;

if (p[1] & 0x80) state = funcnames[0].n_name;
else state = funcnames[1].n_name;

icode = p[1] & 0x7f;

if (!flag)
(void) printf("    SLOT    UNIT    STATE    ID    DESCRIPTION\n");
flag++;
(void) printf("     %3d    %c%-2d     %-3s      %03d   %s\n",
    i, hletter, ucode, state, icode, id[icode].describe);
}

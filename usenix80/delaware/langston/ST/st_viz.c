#include "st_def.h"
/*********************************************************
		Created by Peter Langston

		   All rights reserved

Compile: cc -O -c -q st_viz.c

Copyright (c) 1976 by Peter S. Langston - New  York,  N.Y.
*********************************************************/

char    *whatviz "@(#)st_viz.c	1.7  last mod 12/28/79 -- (c) psl 1976";

/*
**      ST_VIZ -- All device-dpenedent routines for Star_Drek
**       modified to run on terms without CLOSE_LINE   3/1/79
*/

#define WAS_DISP    61
#define VERT_LINE   62
#define HORIZ_LINE  63
#define BODY_FILL   128

#ifdef NOCRTLDEL
#define MSG_LIMIT   (MSG_BOTTOM - MSG_TOP)  /* message line that wraps */
int     linlen[MSG_LIMIT];                  /* displayed line lengths */
int     curlin;                             /* most recent display line */
#endif

viz()
{
        register int i, j, k;
	int center, r, jj, kk, l, lsq, m, dksq;
	double dists[64], d, zfact, *pi;

	new[12][3] = new[12][4] = new[12][6] = new[12][7] = VERT_LINE;
	new[6][5] = new[8][5] = new[10][5] = HORIZ_LINE;
	new[14][5] = new[16][5] = new[18][5] = HORIZ_LINE;
	dists[VERT_LINE] = dists[HORIZ_LINE] = 99999.;
	dists[0] = dists[WAS_DISP] = 99999.;
	range = center = 9999;
	     /* go through all objects & place visible ones on screen */
        for (i = 1; i < NUM_OBJECTS; i++) {
            r = rac[i];
            pi = pos[i];
	    if (active[i] <= 0 || pi[2] <= 0.)
		continue;                          /* if dead or behind us */
	    if (pi[2] > sc[r].sc_visd || (d = size(pi)) > sc[r].sc_visd)
		continue;                               /* if too far away */
	    zfact = M_HWIDTH / pi[2];
	    j = pi[0] * zfact + M_HWIDTH;
	    k = (M_HEIGHT - pi[1] * zfact) * .5;
	    if (j >= 0 && j < M_WIDTH && k >= 0 && k < M_HEIGHT
	     && dists[new[j][k] & 63] > d)
		center = vizadd(d>sc[r].sc_visd * .4? i + 64 : i,
		 j, k, center, d);
	    dists[i] = d;
	    l = sc[r].sc_rad * .1 * zfact;  /* size of obj in char columns */
	    m = (l + 1) >> 1;                 /* size of obj in char lines */
	    lsq = l * l;
	    for (kk = max(0, k - m); kk <= min(M_HEIGHT - 1, k + m); kk++) {
		dksq = (kk - k) * 2;
		dksq =* dksq;
		for (jj = max(0, j - l); jj <= min(M_WIDTH - 1, j + l); jj++)
		    if ((jj - j) * (jj - j) + dksq <= lsq         /* round */
		     && dists[new[jj][kk] & 63] > d)
			center = vizadd(i + BODY_FILL, jj, kk, center, d);
	    }
	}
	/* now display the changes */
	for (j = 0; j < M_HEIGHT; j++) {
	    for (i = 0; i < M_WIDTH; i++) {
		if ((k = new[i][j]) == 0)
                    continue;
		if (k == WAS_DISP) {
                    monit(' ', i, j);
                    old[i][j] = new[i][j] = 0;
                    continue;
                }
                kk = old[i][j];
		new[i][j] = WAS_DISP;
		if (-kk == k)
                    continue;
		old[i][j] = -k;
		if (k == VERT_LINE)
                    monit('|', i, j);
		else if (k == HORIZ_LINE)
                    monit('-', i, j);
		else if (k >= BODY_FILL) {
		    if (-kk >= BODY_FILL)
			continue;
		    monit(bodyfill, i, j);
		} else {
		    r = rac[k & 63];
		    if (k < 64)
			monit(shape[r].near, i, j);
		    else
			monit(shape[r].far, i, j);
		}
                continue;
            }
        }
	if (*crtrpt)
	    monit(0, 0, 0);             /* flush saved characters */
}

vizadd(i, j, k, center, d)
double d;
{
	register int l, dx, dy;

	new[j][k] = i;
	dx = j - M_HWIDTH;
	dy = (M_HHEIGHT - k) * 2.;
	l = dx * dx + dy * dy;
	if (l <= center) {
	    center = l;
	    range = d;
	}
	return(center);
}

monit(c, x, y)
char c;
{
	static int lmx, lmy;
	static int lmc, count;

	if (*crtrpt) {
	    if (x == lmx && y == lmy && c == lmc) {
		count++;
		lmx++;
		return;
	    }
	    if (count > 0) {
		printf("%s", cursor(lmx + M_LEFT - count, lmy + M_TOP));
		if (count > 2)
		    printf("%s%c%c", crtrpt, 040 + count, lmc);
		else if (count == 2)
		    printf("%c%c", lmc, lmc);
		else if (count == 1)
		    printf("%c", lmc);
	    }
	    if (c == 0) {
		count = 0;
		lmx = lmy = lmc = -1;
	    } else {
		lmx = x + 1;
		lmy = y;
		lmc = c;
		count = 1;
	    }
	    return;
	}
	if (x == lmx && y == lmy)
	    printf("%c", c);
	else
	    printf("%s%c", cursor(x + M_LEFT, y + M_TOP), c);
	lmx = x + 1;
	lmy = y;
}


pow(dx, dy, dst, siz)
double  dx, dy, dst, siz;
{
        register int i, j;
	int  x, y, xspread, yspread;

	xspread = dst > 50? 1 : 2;
	 yspread = dst > 50? 0 : 1;
	x = M_HWIDTH + M_HWIDTH * dx;
	x = min(M_WIDTH - xspread - 1, x);
	x = max(xspread, x);
	y = M_HHEIGHT - M_HHEIGHT * dy;
	y = min(M_HEIGHT - yspread - 1, y);
	y = max(yspread, y);
	for (i = x - xspread; i <= x + xspread; i++)
	    for (j = y - yspread; j <= y + yspread; j++)
                old[i][j] = new[i][j] = 0;
	x = x + M_LEFT;
	y = y + M_TOP;
	if (dst > 50.) {
	    printf("%s+O=X=)0(*#*)0(*#*",
	     cursor(x, y));
	    while ((siz =* .5) > 35.)
		printf("(*)) (>+<");
	    printf(" )*(* *> <O- -   ");
	} else {
	    printf("%s+O>X<=)0(=*#*", cursor(x, y));
	    printf("%s\\ /", cursor(x - 1, y - 1));
	    printf("%s/ \\", cursor(x - 1, y + 1));
	    printf(cursor(x, y));
	    while ((siz =* .5) > 35.)
		printf(" (*))) (( + ");
	    printf("%s\\` '/", cursor(x - 2, y - 1));
	    printf("%s/' `\\", cursor(x - 2, y + 1));
	    printf("%s>#0#<)> <(>   <     ",
	     cursor(x - 2, y));
	    printf("%s     ", cursor(x - 2, y - 1));
	    printf("%s     ", cursor(x - 2, y + 1));
	}
}

phaz_effect(dx, dy, f)
double  dx, dy, f;
{
        register  int  i, x, y;

	x = M_HWIDTH + M_HWIDTH * dx;
	y = M_HHEIGHT + M_HHEIGHT * dy;
	new[x][y] = old[x][y] = new[x - 1][y] = old[x - 1][y] = 0;
	printf("%s| / - \\ | /",
	 cursor(x + M_LEFT, y + M_TOP));
	x = f + 1.;
	for (i = 0; i <= x; i =+ 3)
	    printf(" O%c%c %c%c%c %c%c%c  %c%c",
	     8,8, "-\\|/"[i & 3], 8,8, "-\\|/"[i & 3], 8,8, 8,8);
}

moninit()
{
        register int i, j;
	extern char dislen;

        printf(crtclr);
	for (i = 0; i < M_WIDTH; i++)
	    for (j = 0; j < M_HEIGHT; j++)
                old[i][j] = new[i][j] = 0;
	printf(crtinit);
	for (i = 0; i < MAX_O; i++)
            o[i] = 9876;
	for (i = 1; i <= MAX_LABELLED_DIAL; i++)
	    init_dial(i);
	dislen = 0;
#ifdef NOCRTLDEL
	for (i = MSG_LIMIT; --i >= 0; )
	    linlen[i] = 0;
	curlin = MSG_LIMIT - 1;
#endif
}

    /* these two routines are for terminals with a CLOSE_LINE feature */
#ifndef NOCRTLDEL
display(secs, string)
char *string;
{
        register char *cp;
	extern char dislen;

	printf("%s%s", cursor(0, MSG_TOP), crtldel);     /* scroll up 1 */
	printf("%s%s", cursor(0, MSG_BOTTOM), string);   /* display msg */
	for (cp = string; *cp++; );
	dislen = cp - string;
        sleep(secs);
}

disapp(secs, string)
char *string;
{
	register char *cp;
	register int length;
	extern char dislen;

	for (cp = string; *cp++; );
	length = dislen + cp - string;
	if (length < 79) {
	    printf("%s%s", cursor(dislen + 1, MSG_BOTTOM), string);
	    dislen = length;
        } else
            display(0, string);
        sleep(secs);
}
#endif

    /* these two routines are for terminals without a CLOSE_LINE feature */
#ifdef NOCRTLDEL
display(secs, string)
char *string;
{
        register char *cp;
	register int length, i;
	extern char dislen;

	printf("%s  ", cursor(0, MSG_TOP + curlin));    /* wipe old ">>" */
	curlin = (curlin + 1) % MSG_LIMIT;
	printf("%s>> %s", cursor(0, MSG_TOP + curlin), string);
	for (cp = string; *cp++; );
	length = cp - string;
	if (linlen[curlin] > length)
	    for (i = linlen[curlin]; --i > length; )
		printf(" ");
	linlen[curlin] = length + 2;
        sleep(secs);
}

disapp(secs, string)
char *string;
{
	register char *cp;
        register int length, i;
	extern char dislen;

	for (cp = string; *cp++; );
	dislen = linlen[curlin];
	length = dislen + cp - string;
	if (length < 79) {
	    printf("%s%s", cursor(dislen, MSG_TOP + curlin), string);
	    if (linlen[curlin] > length)
		for (i = linlen[curlin]; --i >= length; )
		    printf(" ");
	    linlen[curlin] = length + 2;
        } else
            display(0, string);
        sleep(secs);
}
#endif

text_xy(x, y, sp)
char *sp;
{
        printf("%s%s", cursor(x, y), sp);
}

dial_int(n, val0, val1, val2)
int     n, val0, val1, val2;
{
        struct dialstr *dip;

        dip = &dialinfo[n];
	printf("%s", cursor(dip->valx, dip->valy));
        printf(dip->valfmt, val0, val1, val2);
}

dial_dbl(n, val0, val1, val2)
int     n;
double  val0, val1, val2;
{
        struct dialstr *dip;

        dip = &dialinfo[n];
        printf("%s", cursor(dip->valx, dip->valy));
        printf(dip->valfmt, val0, val1, val2);
}

init_dial(n)                    /* put dial #n in initial state */
int     n;
{
        struct dialstr *dip;

        dip = &dialinfo[n];
	printf("%s%s", cursor(dip->labx, dip->laby), dip->label);
}

dials(which)
{
        register int i, j, k;
	long now;

	switch (which) {
	case ALL_DIALS:
	case MISS_ANG_DIAL:
	case MISS_DST_DIAL:
	    dp = pos[targ];
	    i = arcpsltan(dp[1], dp[2]);
	    i =+ 32;
	    i =>> 6;
	    j = arcpsltan(dp[0], dp[2]);
	    j =+ 32;
	    j =>> 6;
	    k = size(dp);
	    if (o[0] != i || o[1] != j)
		dial_int(MISS_ANG_DIAL,  o[0]=i, o[1]=j);
	    if (o[2] != k)
		dial_int(MISS_DST_DIAL, o[2]=k);
	    if (which != ALL_DIALS)
		break;
	case ENERGY_DIAL:
	    if (o[3] != (i=energy))
		dial_int(ENERGY_DIAL, o[3]=i);
	    if (which != ALL_DIALS)
		break;
	case ETIME_DIAL:
	    time(&now);
	    etime = now / 60. - time0;
	    if (o[4] != (i=etime))
		dial_int(ETIME_DIAL, o[4]=i);
	    if (which != ALL_DIALS)
		break;
	case SB_ANG_DIAL:
	case SB_DST_DIAL:
	    dp = pos[S_BASE_NUM];
	    i = arcpsltan(dp[1], dp[2]);
	    i =+ 32;
	    i =>> 6;
	    j = arcpsltan(dp[0], dp[2]);
	    j =+ 32;
	    j =>> 6;
	    k = size(dp);
	    if (o[5] != i || o[6] != j)
		dial_int(SB_ANG_DIAL,  o[5]=i, o[6]=j);
	    if (o[7] != k)
		dial_int(SB_DST_DIAL, o[7]=k);
	    if (which != ALL_DIALS)
		break;
	case RANGE_DIAL:
	    if (o[8] != (i=range))
		dial_int(RANGE_DIAL, o[8]=i);
	    if (which != ALL_DIALS)
		break;
	case SENSORS_DIAL:
	    if (o[9] != (i=sensors))
		dial_int(SENSORS_DIAL, o[9]=i);
	    if (which != ALL_DIALS)
		break;
	case VEL_ANG_DIAL:
	case VEL_DST_DIAL:
	    dp = vel[0];
	    i = arcpsltan(dp[1], dp[2]);
	    i =+ 32;
	    i =>> 6;
	    j = arcpsltan(dp[0], dp[2]);
	    j =+ 32;
	    j =>> 6;
	    if (o[10] != i || o[11] != j)
		dial_int(VEL_ANG_DIAL,  o[10]=i, o[11]=j);
	    k = size(dp) * 10.;
	    if (o[12] != k)
		dial_dbl(VEL_DST_DIAL, (o[12]=k) / 10.);
	    if (which != ALL_DIALS)
		break;
	case SPEED_DIAL:
	    k = v0[2] * 10.;
	    if (o[13] != k)
		dial_dbl(SPEED_DIAL, (o[13]=k) / 10.);
	    if (which != ALL_DIALS)
		break;
	case CONDITION_DIAL:
	    if (o[14] != (i=sensors/128.)) {
		o[14] = i;
		if (i <= 0)
		    dial_int(CONDITION_DIAL, "", " GREEN ", "");
		else if (i < 4)
		    dial_int(CONDITION_DIAL, "", "YELLOW ", "");
		else
		    dial_int(CONDITION_DIAL, crtblon, " RED ", crtbloff);
	    }
	    if (which != ALL_DIALS)
		break;
	case ROTATION_DIAL:
	    if (o[15] != xrots || o[16] != yrots || o[17] != zrots)
		dial_dbl(ROTATION_DIAL, o[15]=xrots, o[16]=yrots, o[17]=zrots);
	}
}

/*
**      CURSOR -- X Y positioning routine (set up for ITEM OWLs)
*/

cursor(x, y)
int    x, y;
{
     register char *cp;
     static char cursbuf[4][4], next;

     next = (next + 1) & 3;
     cp = &cursbuf[next][4];
     *--cp = '\0';
     *--cp = x + 040;
     *--cp = y + 040;
     *--cp = 017;
     return(cp);
}

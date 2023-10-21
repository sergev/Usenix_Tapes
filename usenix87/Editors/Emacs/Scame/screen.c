/*	SCAME screen.c				*/

/*	Revision 1.0.0  1985-02-09		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"

update()
{
register char *tdot;
register int i, j, y, x;
int sw = screenwidth-1;
int wb;
	upd2();
	if (typeahead()) return;
	for (j=(windows > 1 && bufno == otherbuf); j >= 0; j--) {
		y=wintop;
		x=0;
/*
		upd2();
*/
		wb=winbot+1;
		tdot=home;
		if (away < z) {
			while (y<wb) {
/* vvvvvvvvvvvvvvvv THIS CODE IS DOUBLED FOR SPEED */
				if (*tdot == '\n') {
					screen[y++][x]='\0'; x = -1; }
				else {
				    if (x==sw) {
				    	screen[y][x]='!';
					screen[y++][++x]='\0';
					x=0; }
				    if (*tdot == '\t') {
					i = min(x+TABWID-(x&7), sw);
					while (x < i)
						screen[y][x++]=' ';
					x--;
				    }
				    else if (*tdot < ' ' || *tdot == DEL) {
					screen[y][x++]='^';
					if (x==sw){
						screen[y][x]='!';
						screen[y++][++x]='\0';
						x=0;}
					screen[y][x] = *tdot + 64;
				    }
				    else  screen[y][x] = *tdot;
				}			
				tdot++; x++;
			}
		}
/* ^^^^^^^^^^^^^^^^ */
		else {
			while (tdot<z) {
/* vvvvvvvvvvvvvvvv */
				if (*tdot == '\n') {
					screen[y++][x]='\0'; x = -1; }
				else {
				    if (x==sw) {
				    	screen[y][x]='!';
					screen[y++][++x]='\0';
					x=0; }
				    if (*tdot == '\t') {
					i = min(x+TABWID-(x&7), sw);
					while (x < i)
						screen[y][x++]=' ';
					x--;
				    }
				    else if (*tdot < ' ' || *tdot == DEL) {
					screen[y][x++]='^';
					if (x==sw){
						screen[y][x]='!';
						screen[y++][++x]='\0';
						x=0;}
					screen[y][x] = *tdot + 64;
				    }
				    else  screen[y][x] = *tdot;
				}			
				tdot++; x++;
			}
/* ^^^^^^^^^^^^^^^^ */
			if (y < wb) screen[y][x]='\0';
			while (++y < wb) *screen[y]='\0';
		}
		if (j) { otherwindow(); upd2(); }
	}
	if (windows > 1 && bufno == otherbuf) { otherwindow(); upd2(); }
	oldhome = home;
	updateflag = FALSE;
}

upd2()				/* Updates the values of home,	*/
{				/* curx, cury and away.		*/
register char *tdot;
register int i;
int y=wintop;
register int x=0;
int sw=screenwidth-1;
	if (gaps < home + GAPDIST) closegap();
	if (dot < home) {
		home = dot;
		while (home > buf && *(home-1) != '\n') home--;
		center();
	}
	tdot=home;
	while (y < winbot+1 && tdot < z) {
	    if (tdot==dot) { curx=x; cury=y; }
	    if (*tdot == '\n') { y++; x = -1; }
	    else {
		if (x==sw) { y++; x=0; }
		if (*tdot == '\t')
		    x = min(x + TABWID -(x & 7), sw)-1;
		else if (*tdot < ' ' || *tdot == DEL) {
		    x++;
		    if (x==sw) { y++; x=0; } } }
	    tdot++; x++; }
	if (tdot==dot) { curx=x; cury=y; }
	if (*(tdot-1) == '\n' && tdot < z) away=tdot-1;
	else away=tdot;
	if (dot > away
	    || cury > wintop + (winbot-wintop)*gvars.bottom_display_margin/100
	    || cury < wintop + (winbot-wintop)*gvars.top_display_margin/100) {
		home = dot;
		while (home > buf && *(home-1) != '\n') home--;
		center();
		upd2();
	}
}

refline(y,invflg)
int y;
Bool invflg;
{
register char *pt1,*pt2,*pt3,*pt4;
int len, len1, len2, i;
	pt1 = screen[y]; pt2 = oldscreen[y];
	pt1[screenwidth] = '\0';	/* In case line was too long */
	len1 = strlen(pt1);
	len2 = strlen(pt2);
	while (*pt1 == *pt2 && *pt1 != '\0' && *pt2 != '\0') {
		pt1++; pt2++; }
	if (*pt2 != *pt1) {
		if (*pt2 == '\0') while (*pt1 == ' ') pt1++;
		cur(y, pt1 - screen[y]);
		if (len1 == len2) {
			pt3 = screen[y] + len1 - 1;
			pt4 = oldscreen[y] + len1 - 1;
			while (*pt3 == *pt4) { pt3--; pt4--; }
			pt3++;
			len = pt3 - pt1;
		}
		else len = len1 - (pt1 - screen[y]);
		if (invflg) invmod(TRUE);
		write(1,pt1,len);
		if (invflg) invmod(FALSE);
		hpos += len;
		if (len1 < len2) {
			if (CE != NIL) cleol(FALSE);
			else for(i=1; i<=len2-len1;i++,hpos++) pchar(' ');
		}
		strcpy(oldscreen[y],screen[y]);
	}
}

refresh(modeflg)
int modeflg;
{
register int y;
	if (!typeahead()) refline(cury,FALSE);
	if (modeflg) refline(winbot+1,TRUE);
	if (typeahead()) return;
	if (modeflg) {
 		if (winbot < screenlen-3) refline(screenlen-2,TRUE);
		else if (wintop > 0) refline(wintop-1,TRUE);
	}
	for (y=0; y < screenlen-2; y++) {
		if (typeahead()) return;
		if (y != cury) refline(y,FALSE);
	}
	if (!typeahead() && (cury != vpos || curx != hpos)) cur(cury,curx);
}

findxy()			/* Updates the values of */
{				/* linpos, curx and cury */
register char *t;
register int i;
int y=wintop;
register int x=0;
int sw=screenwidth-1;
int tlinpos=0;
	if (gaps < home + GAPDIST) closegap();
	if (dot < home) {
		home = dot;
		while (home > buf && *(home-1) != '\n') home--;
		center();
	}
	t=home;
	while (y <= winbot && t < z) {
	    if (t==dot) { curx=x; cury=y; linpos=tlinpos; return; }
	    if (*t == '\n') { y++; x = -1; tlinpos = -1; }
	    else {
		if (x==sw) { y++; x=0; }
		if (*t == '\t') {
			i = min(x + TABWID -(x & 7), sw)-1;
			tlinpos += i-x;
			x = i;
		}
		else if (*t < ' ' || *t == DEL) {
			x++;
			tlinpos++;
			if (x==sw) { y++; x=0; } } }
	    t++;
	    x++;
	    tlinpos++;
	}
	if (t==dot) { curx=x; cury=y; linpos=tlinpos; return; }
	home = dot;
	while (home > buf && *(home-1) != '\n') home--;
	center();
	findxy();
}

int findcurx()
{
char *tdot;
register int i,x,y;
int sw = screenwidth-1;
	tdot = dot;
	while (tdot > buf && *(tdot-1) != '\n') tdot--;
	x = 0;
	y = wintop;
	while (tdot < dot) {
		if (x == sw) {
			x = 0;
			y++;
		}
		if (*tdot == '\t')
			x = min(x + TABWID -(x & 7), sw) - 1;
		else if (*tdot < ' ' || *tdot == DEL) {
			x++;
			if (x==sw) {
				x = 0;
				y++;
			}
		}
		tdot++;
		x++;
	}
	curx = x;
	return y;
}

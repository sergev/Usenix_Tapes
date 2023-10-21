/* sub2.c -- subroutines for visual star trek */

#include "vtrek.h"

/* klingon movement and firing */
klingmove()
{
	int k, dx, dy, dam, kx, ky, nx, ny, i, j;
	int minj, mind, d, sk;
	static int dirx[9] = {1, 1, 0, -1, -1, -1, 0, 1, 0};
	static int diry[9] = {0, 1, 1, 1, 0, -1, -1, -1, 0};
	double dist;
	char str[40];

	for (k = 0; k < 3; k++) {
	    if (klingon[k].sh < 0)
		continue;
	    kx = klingon[k].xs;
	    ky = klingon[k].ys;

	    /* movement */
	    if (rnd(100) < 30 + skill * 5) {
		for (i = 0; i < numkmove; i++) {
		    mind = 999;
		    minj = -1;
		    for (j = 0; j < 9; j++) {
			nx = kx + dirx[j];
			ny = ky + diry[j];
			if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
			    continue;
			if (quadrant[nx][ny] != EMPTY)
			    continue;
			dx = xsect - nx;
			dy = ysect - ny;
			d = dx * dx + dy * dy;
			if (d < mind) {
			    mind = d;
			    minj = j;
			}
		    }

		    if (minj < 0)
			break;

		    nx = kx + dirx[minj];
		    ny = ky + diry[minj];
		    quadrant[kx][ky] = EMPTY;
		    quadrant[nx][ny] = KLINGON;
		    plt_srs(ELEMENT, nx, ny);
		    plt_srs(ELEMENT, kx, ky);
		    kx = nx;
		    ky = ny;
		}
		klingon[k].xs = kx;
		klingon[k].ys = ky;
	    }

	    /* fire disrupters */
	    dx = kx - xsect;
	    dy = ky - ysect;
	    dist = sqrt((double)(dx * dx) + (double)(dy * dy));
	    dam = (rnd(100) + 100 + skill * 20) / dist;
	    if (damage[SRS] > 0)
	        sprintf(str, "Klingon at [%d,%d] hits with %d units.", kx+1,
		ky+1, dam);
	    else
		sprintf(str, "Klingon hits with %d units.", dam);
	    readout(ADDLINE, str);
	    if (condition == DOCKED)
		readout(ADDLINE, "Starbase shields protect you!");
	    else {
		if ((shields -= dam) < 0)
		    die();
		sk = skill * 250;
		if (shields <= 500 + sk)
		    fixdev(REL, RND, -dam / 2);
		else if (shields < 1500 + sk)
		    fixdev(REL, RND, -(1500 + sk - shields) * dam / 2000);
	    }
	}

	plt_stat(ELEMENT, SHIELDS);
}

/* fire phasers */
phasers()
{
	int dam, k, dx, dy, e, kx, ky;
	char str[40];
	double dist;

	if (damage[PHASER] < 0)
	    readout(ADDLINE, "Phasers are damaged.");
	else {
	    for (k = 0; k < 3; k++) {
		if (klingon[k].sh < 0)
		    continue;
		kx = klingon[k].xs;
		ky = klingon[k].ys;

		if (damage[SRS] > 0) {
		    sprintf(str, "Units aimed at [%d,%d] : ", kx + 1, ky + 1);
		    prompt(str);
		}
		else
		    prompt("Units aimed at [?,?] : ");
		e = getnum();
		dx = kx - xsect;
		dy = ky - ysect;
		dist = sqrt((double)(dx * dx) + (double)(dy * dy));
		dam = e / dist;

		if (energy < e)
		    readout(ADDLINE, "Insufficient energy for command.");
		else {
		    energy -= e;
		    damkling(kx, ky, dam);
		    plt_stat(ELEMENT, ENERGY);
		}
	    }
	}
}

/* set condition */
setcondition()
{
	int dx, dy, i, oldcond;
	int plotgal, plotsrs;

	oldcond = condition;

	if (galaxy[xquad][yquad].nkling)
	    condition = RED;
	else if (shields < 100) {
	    condition = YELLOW;
	    if (oldcond != DOCKED)
	        readout(ADDLINE, "Captain, shields are dangerously low.");
	}
	else
	    condition = GREEN;

	if (galaxy[xquad][yquad].nbase) {
	    dx = xsect - base_xsect;
	    dy = ysect - base_ysect;
	    if (abs(dx) <= 1 && abs(dy) <= 1) {
		condition = DOCKED;
		if (oldcond != DOCKED) {
		    energy = 3000;
		    torps = 10;
		    shields = 0;
		    plotsrs = damage[SRS] <= 0;
		    plotgal = damage[COMPUTER] <= 0;
		    for (i = 0; i < 8; i++)
		        damage[i] = 100;
		    readout(ADDLINE, "Shields lowered for docking.");
		    playership[0] = playership[2] = ' ';
		    plt_srs(ELEMENT, xsect, ysect);
		    plt_stat(INFO);
		    plt_dam(INFO);
		    if (plotsrs)
			plt_srs(INFO);
		    if (plotgal)
		        plt_gal(INFO);
		    oldcond = DOCKED;
		}
	    }
	}

	if (oldcond != condition)
	    plt_stat(ELEMENT, CONDITION);
}

/* move torp to a sector and see if it hit */
mvtorp(x, y)
int x, y;
{
	int ch, status = ALIVE;
	char str[40];

	ch = quadrant[x][y];
	quadrant[x][y] = TBLAST;
	plt_srs(ELEMENT, x, y);

	switch (ch) {
	case EMPTY :
	    break;
	case KLINGON :
	    damkling(x, y, AUTOKILL);
	    status = DEAD;
	    break;
	case STARBASE :
	    readout(ADDLINE, "Starbase destroyed.");
	    galaxy[xquad][yquad].nbase--;
	    numbases--;
	    plt_num(INFO);
	    status = DEAD;
	    break;
	case STAR :
	    sprintf(str, "Star at [%d,%d] has gone supernova.", x + 1, y + 1);
	    readout(ADDLINE, str);
	    galaxy[xquad][yquad].nstar--;
	    status = DEAD;
	    break;
	}

	if (status == DEAD)
	    ch = EMPTY;
	quadrant[x][y] = ch;
	plt_srs(ELEMENT, x, y);

	return status;
}

/* fire a torpedo */
torpedo()
{
	int dir, i, ch, x, y;
	int dx, dy;
	double th;
	static char dirstr[] = "DEWQAZXC";

	if (damage[TUBES] < 0)
	    readout(ADDLINE, "Photon torpedo tubes are damaged.");
	else if (torps < 1)
	    readout(ADDLINE, "You're out of photon torpedos!");
	else {
	    torps--;
	    plt_stat(ELEMENT, TORPS);
	    prompt("Direction ? ");
	    dir = get_a_char();
	    if (isdigit(dir)) {
		putch(dir);
		dir -= '0';
		while ((ch = get_a_char()) != '\r') {
		    putch(ch);
		    if (isdigit(ch))
			dir = dir * 10 + ch - '0';
		    else
			break;
		}
	    }
	    else {
		ch = Toupper(dir);
		dir = 0;
		for (i = 0; i < 8; i++) {
		    if (ch == dirstr[i])
			break;
		    dir += 45;
		}
		if (i >= 8) {
		    readout(ADDLINE, "Illegal direction.");
		    return;
		}
	    }

	    th = dir / 180.0 * PI;
	    if (dir >= 315 || dir <= 45 || (dir >= 135 && dir <= 225)) {
		dx = (dir >= 315 || dir <= 45) ? 1 : -1;
		for (x = xsect + dx; x >= 0 && x <= 7; x += dx) {
		    y = ysect - (x - xsect) * tan(th) + 0.5;
		    if (y < 0 || y > 7)
			break;
		    if (mvtorp(x, y) == DEAD) {
			break;
		    }
		}
	    }
	    else {
		th -= PI / 2.0;
		dy = (dir >= 45 && dir <= 135) ? -1 : 1;
		for (y = ysect + dy; y >= 0 && y <= 7; y += dy) {
		    x = xsect + (y - ysect) * tan(th) + 0.5;
		    if (x < 0 || x > 7)
			break;
		    if (mvtorp(x, y) == DEAD)
			break;
		}
	    }
	}
}

/* repair devices somewhat */
repdevices()
{
	int i;

	for (i = 0; i < 8; i++)
	    fixdev(REL, i, 5);
}

/* out of star dates */
timeout()
{
	char str[44];

	readout(CLEAR);
	readout(ADDLINE, "You have run out of stardates an there");
	sprintf(str, "are still %d Klingons left.  Some captain you", numkling);
	readout(ADDLINE, str);
	readout(ADDLINE, "are.");
	die();
}

/* out of energy */
dead()
{
	readout(CLEAR);
	readout(ADDLINE, "Your ship is dead in space.  Eventually,");
	readout(ADDLINE, "Klingons show up and relieve you of your");
	readout(ADDLINE, "task . . .");
	die();
}

/* reset terminal and exit program */
vexit(status)
{
	termreset();
	exit(status);
}

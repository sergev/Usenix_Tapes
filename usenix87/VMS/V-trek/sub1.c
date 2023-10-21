/* sub1.c -- subroutines for visual star trek */
#include curses
#include "vtrek.h"

/* print short help information */
help()
{
	if (rolines > 5)
	    readout(CLEAR, NULL);
	readout(ADDLINE, "Directions = Q,W,E,A,D,Z,X,C");
	readout(ADDLINE, "H = Hyper-space      S = Short range scan");
	readout(ADDLINE, "L = Long range scan  P = Fire phasers");
	readout(ADDLINE, "T = Fire torpedo     U = Defense control");
	readout(ADDLINE, "R = Redraw screen    F = Fix devices");
}

/* get command character */
getcmd()
{
	int ch, i;
	char str[40];

	sprintf(str, "Command %s ? ", captain);
	prompt(str);

	ch = get_a_char();

	return Toupper(ch);
}

/* give option to see instructions */
instructions()
{
	FILE *fp;
	char line[80];
	int n, ch;

	printf("Instructions ? ");

	/* This would seem to be a silly place to randomize,  but this */
	/* location made things a bit easier for the CPM version. */
	randomize();

	if (fgets(line, 80, stdin) == NULL)
	    vexit(1);
	ch = Toupper(line[0]);
	if (ch == 'Y') {
	    if ((fp = fopen(VTREKINS, "r")) == NULL)
		printf("Error: Missing instruction file\n");
	    else {
		terminit();
		for (;;) {
		    for (n = 0; n < 22; n++) {
			if (fgets(line, 80, fp) == NULL)
			    break;
			printf("%s\n", line);
		    }
		    if (n < 22)
			break;
		    printf("--More--");
		    get_a_char();
		    printf("\r        \r");
		}
		fclose(fp);
		termreset();
	    }
	}
}

/* get captain's name and ship's name */
getnames()
{
	int ch, tmp;

	printf("Your name captain ? ");
	if (fgets(captain, 11, stdin) == NULL)
	    vexit(-1);
	if (captain[tmp = strlen(captain) - 1] == '\n')
	    captain[tmp] = '\0';
	else
	    while (getchar() != '\n');
	if (*captain == '\0')
	    strcpy(captain, "Duncel");

	printf("Your ship's name captain ? ");
	if (fgets(shipname, 11, stdin) == NULL)
	    vexit(-1);
	if (shipname[tmp = strlen(shipname) - 1] == '\n')
	    shipname[tmp] = '\0';
	else
	    while (getchar() != '\n');
	if (*shipname == '\0')
	    strcpy(shipname, "Fruit");
}

/* get skill level */
getskill()
{
	char level[10];

	printf("Skill level (0-5, 0=easy, 5=hard) ? ");
	if (fgets(level, 10, stdin) == NULL)
	    vexit(-1);
	if (isdigit(level[0]))
	    skill = level[0] - '0';
	else
	    skill = 3;
}

/* initialize galaxy */
initgal()
{
	int i, j, k, r, n;

	numkling = 0;

	for (i = 0; i < 8; i++) {
	    for (j = 0; j < 8; j++) {
		if ((r = rnd(100)) < 10)
		    k = 3;
		else if (r < 20)
		    k = 2;
		else if (r < 30)
		    k = 1;
		else
		    k = 0;
		numkling += k;
		galaxy[i][j].nkling = k;
		galaxy[i][j].nstar = rnd(8) + 1;
		galaxy[i][j].nbase = 0;
		galaxy[i][j].known = 0;
	    }
	}

	numbases = rnd(3) + 2;

	for (n = 0; n < numbases; n++) {
	    for (;;) {
		if (galaxy[i = rnd(7)][j = rnd(7)].nbase == 0) {
		    galaxy[i][j].nbase = 1;
		    break;
		}
	    }
	}
}

/* initialize variables */
initvars()
{
	int i;

	getnames();
	getskill();
	initgal();

	numkmove = skill;
	begkling = numkling;
	xquad = rnd(7);
	yquad = rnd(7);
	old_xsect = (xsect = rnd(7));
	old_ysect = (ysect = rnd(7));
	shields = 0;
	energy = 3000;
	stardate = 2000.0;
	begdate = stardate;
	lastdate = stardate + (float)numkling;
	torps = 10;
	playership[1] = *shipname;
	condition = YELLOW;

	for (i = 0; i < 8; i++)
	    damage[i] = 100;
	
	old_xquad = -1;
	old_yquad = -1;
	setpos();
}

/* set new position -- check for outside galaxy, set up new quadrant */
setpos()
{
	int dam = 0, i, j, n, dev, status = 0;
	static int notfirstcall = 0;

	if (xsect > 7) {
	    xsect = 0;
	    xquad++;
	}
	else if (xsect < 0) {
	    xsect = 7;
	    xquad--;
	}

	if (ysect > 7) {
	    ysect = 0;
	    yquad++;
	}
	else if (ysect < 0) {
	    ysect = 7;
	    yquad--;
	}

	if (xquad > 7) {
	    dam = 1;
	    xquad = 7;
	    xsect = 7;
	}
	else if (xquad < 0) {
	    dam = 1;
	    xquad = 0;
	    xsect = 0;
	}

	if (yquad > 7) {
	    dam = 1;
	    yquad = 7;
	    ysect = 7;
	}
	else if (yquad < 0) {
	    dam = 1;
	    yquad = 0;
	    ysect = 0;
	}

	if (dam) {
	    readout(ADDLINE, "You encounter a force field.");
	    shields -= rnd(20) + 90;
	    plt_stat(ELEMENT, SHIELDS);
	    status = DAMAGE;
	    if (shields < 0)
		die();
	}

	if (xquad != old_xquad || yquad != old_yquad) {
	    galaxy[xquad][yquad].known = 1;
	    if (notfirstcall) {
		plt_gal(ELEMENT, old_xquad, old_yquad);
	        plt_gal(ELEMENT, xquad, yquad);
	    }

	    for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
		    quadrant[i][j] = EMPTY;

	    quadrant[xsect][ysect] = PLAYER;

	    for (n = 0; n < galaxy[xquad][yquad].nkling; n++) {
		for (;;) {
		    if (quadrant[i = rnd(7)][j = rnd(7)] == EMPTY) {
			quadrant[i][j] = KLINGON;
			break;
		    }
		}
		klingon[n].xs = i;
		klingon[n].ys = j;
		klingon[n].sh = 200;
	    }

	    for (; n < 3; n++)
		klingon[n].xs = klingon[n].ys = klingon[n].sh = -1;

	    for (n = 0; n < galaxy[xquad][yquad].nbase; n++)
		for (;;) {
		    if (quadrant[i = rnd(7)][j = rnd(7)] == EMPTY) {
			quadrant[i][j] = STARBASE;
			base_xsect = i;
			base_ysect = j;
			break;
		    }
		}

	    for (n = 0; n < galaxy[xquad][yquad].nstar; n++)
		for (;;) {
		    if (quadrant[i = rnd(7)][j = rnd(7)] == EMPTY) {
			quadrant[i][j] = STAR;
			break;
		    }
		}

	    old_xquad = xquad;
	    old_yquad = yquad;
	    if (notfirstcall) {
	        plt_srs(INFO);
	        plt_stat(ELEMENT, CONDITION);
	        plt_stat(ELEMENT, QUADRANT);
	        plt_gal(ELEMENT, xquad, yquad);
	    }
	}
	else {
	    switch (quadrant[xsect][ysect]) {

	    case KLINGON:
		dam = rnd(20) + 90;
		if (damkling(xsect, ysect, dam) != DEAD) {
		    xsect = old_xsect;
		    ysect = old_ysect;
		}
		dam >>= 1;
		if ((shields -= dam) < 0)
		    die();
		fixdev(REL, RND, -dam);
		plt_stat(ELEMENT, SHIELDS);
		break;

	    case STAR:
		readout(ADDLINE, "There's a star in the way, Duncel!");
		if (damage[SRS] > 0)
		    strcpy(captain, "Duncel");
		xsect = old_xsect;
		ysect = old_ysect;
		break;

	    case STARBASE:
		xsect = old_xsect;
		ysect = old_ysect;
		readout(ADDLINE, "There's a star base in the way!");
		break;
	    }

	    status = quadrant[xsect][ysect];
	    quadrant[old_xsect][old_ysect] = EMPTY;
	    quadrant[xsect][ysect] = PLAYER;
	    plt_srs(ELEMENT, old_xsect, old_ysect);
	    plt_srs(ELEMENT, xsect, ysect);
	}
	old_xsect = xsect;
	old_ysect = ysect;
	if (notfirstcall)
	    plt_stat(ELEMENT, SECTOR);
	notfirstcall = 1;

	return status;
}

/* "So this is it.  We're going to die." -- Arthur Dent */
die()
{
	char str[10];
	char buf[80];

	plt_stat(ELEMENT, SHIELDS);
	prompt("(Press RETURN)");
	while (get_a_char() != '\n')
	    ;
	cls();
	sprintf(buf,"Your ship the %s has been destroyed.", shipname);
	mvaddstr(11,21,buf);
	mvaddstr(12,21,"The Federation will be conquered.");
	refresh();
	prompt("(Press RETURN)");
	while (get_a_char() != '\n')
	    ;
	vexit(1);
}

win()
{
	char str[10];
	char buf[132];

	readout(ADDLINE, "Congratulations!");
	prompt("(Press RETURN)");
	while (get_a_char() != '\n')
	    ;
	cls();
	mvaddstr(11,16,"The last of the Klingon battle cruisers have been");
	sprintf(buf,"%15sdestroyed.  You alone have saved the Federation.  You", "");
	mvaddstr(12,16,buf);
	sprintf(buf,"%15sare promoted to Admiral %s !!!", "", captain);
	mvaddstr(13,16,buf);
	refresh();
	prompt("(Press RETURN)");
	while (get_a_char() != '\n')
	    ;
	vexit(1);
}

/* random (?) number generator */
rnd(max)
int max;
{
	return (int)(rand() % max);
}

randomize()
{
	srand(time(0));
}

/* impulse drive -- move one sector */
impulse(dir)
int dir;
{
	int dx, dy, status;

	if (energy <= 0) {
	    readout(ADDLINE, "Insufficient energy for command.");
	    return ERROR;
	}

	if (checkdir(dir, &dx, &dy) != ERROR) {
	    xsect += dx;
	    ysect += dy;
	}

	energy--;
	status = setpos();
	plt_stat(ELEMENT, ENERGY);

	return status;
}

/* defense -- set shield level */
defense()
{
	int i;

	if (damage[DEFENSE] <= 0)
	    readout(ADDLINE, "Defense control is damaged.");
	else {
	    energy += shields;
	    for (;;) {	
	        prompt("Shield level ? ");
	        if ((shields = getnum()) >= 0 && shields <= energy) {
	            energy -= shields;
		    break;
		}
	    }
	    if (shields > 0) {
		playership[0] = '(';
		playership[2] = ')';
	    }
	    else {
		playership[0] = ' ';
		playership[2] = ' ';
	    }
	    plt_srs(ELEMENT, xsect, ysect);
	    plt_stat(ELEMENT, SHIELDS);
	    plt_stat(ELEMENT, ENERGY);
	}
}

/* get a number */
getnum()
{
	int num = 0, ch;

	while ((ch = get_a_char()) != '\r') {
	    addch(ch);
	    refresh();
	    if (isdigit(ch))
		num = num * 10 + ch - '0';
	    else if (ch == '\b') {
		num = num / 10;
		addch(' ');
		addch('\b');
	    }
	    else
		break;
	}

	return num;
}

/* damage a klingon */
damkling(xs, ys, dam)
int xs, ys, dam;
{
	char str[40];
	int k;

	if ((k = findkling(xs, ys)) == ERROR)
	    readout(ADDLINE, "damkling: error");
	else {
	    if (dam != AUTOKILL) {
	        sprintf(str, "You did %d to the Klingon.", dam);
	        readout(ADDLINE, str);
	    }

	    if ((klingon[k].sh -= dam) < 0) {
		readout(ADDLINE, "Klingon destroyed.");
		klingon[k].xs = klingon[k].ys = -1;
		quadrant[xs][ys] = EMPTY;
		plt_srs(ELEMENT, xs, ys);
		numkling--;
		plt_num(INFO);
		galaxy[xquad][yquad].nkling--;
		plt_gal(ELEMENT, xquad, yquad);

		return DEAD;
	    }
	}

	return ALIVE;
}

/* fix/damage a device */
fixdev(type, dev, value)
int type;	/* ABSolution fix or RELative fix */
int dev;	/* device (if RND then pick a damaged device) */
int value;	/* new device value for ABS, amount to add for REL */
{
	int i, old_dam;

	if (dev == RND) {
	    dev = rnd(8);
	    if (value > 0 && damage[dev] >= 100) {
		for (i = 0; i < 8; i++) {
		    if (++dev > 7)
			dev = 0;
		    if (damage[dev] < 100)
			break;
		}
	    }
	}

	old_dam = damage[dev];

	if (type == ABS)
	    damage[dev] = value;
	else
	    damage[dev] += value;

	if (damage[dev] > 100)
	    damage[dev] = 100;

	plt_dam(ELEMENT, dev);

	/* see if device changed from fixed <==> broken */
	if ((old_dam <= 0 && damage[dev] > 0) || (old_dam > 0 && damage[dev] <= 0)) {
	    switch (dev) {
	    case SRS :
	        plt_srs(INFO);
	        break;
	    case DAMAGE :
	        plt_dam(INFO);
	        break;
	    case COMPUTER :
	        plt_gal(INFO);
	        break;
	    }
	}
}

/* print a prompt in the upper left hand corner */
prompt(str)
char *str;
{
char buf[80];
	int i;

	mvaddstr(1,1,"                        ");
	refresh();
	sprintf(buf,"%-.26s", str);
	mvaddstr(1,1,buf);
	move(1,strlen(buf)+2);
	refresh();
}

/* check a direction for validity and return delta-x and delta-y */
checkdir(dir, dx, dy)
int dir, *dx, *dy;
{
	static struct {
	    int d, x, y;
	} dirs[8] = {
	    {'Q', -1, -1}, {'W', 0, -1}, {'E', 1, -1}, {'D', 1, 0},
	    {'C', 1, 1}, {'X', 0, 1}, {'Z', -1, 1}, {'A', -1, 0}
	};
	int i;

	for (i = 0; i < 8; i++)
	    if (dirs[i].d == dir) {
		*dx = dirs[i].x;
		*dy = dirs[i].y;
		return 0;
	    }

	return ERROR;
}

/* hyperspace drive */
hyperspace()
{
	int i, dir, dx, dy, w, tmp, savex, savey;

	if (damage[WARP] <= 0)
	    readout(ADDLINE, "Warp engines are damaged.");
	else {
	    prompt("Direction ? ");
	    if (islower(dir = get_a_char()))
		dir = toupper(dir);
	    if (checkdir(dir, &dx, &dy) == ERROR) {
		readout(ADDLINE, "Illegal direction.");
		return;
	    }

	    prompt("Warp ? ");
	    addch(w = get_a_char());
	    if (isdigit(w)) {
		w -= '0';
	        if (w <= 0)
		    return;
	        savex = xsect;
	        savey = ysect;
	        while ((tmp = xsect + dx) >= 0 && tmp <= 7 && (tmp = ysect + dy) >= 0 && tmp <= 7) {
		    if (impulse(dir))
		        return;
	        }
	        if (energy < 20 * w)
		    readout(ADDLINE, "Insufficient energy for command.");
	        else {
	            xquad += w * dx;
	            yquad += w * dy;
	            energy -= 20 * w;
		    stardate += (double)(w) / 5.0;
	            xsect = savex;
	            ysect = savey;
	            setpos();
	            plt_stat(ELEMENT, ENERGY);
	        }
	    }
	    else if (w == '.' && isdigit(w = get_a_char())) {
		addch(w);
		w -= '0';
		stardate += (double)(w) / 50.0;
		for (; w > 0; w--)
		    if (impulse(dir))
			break;
	    }
	    else
		readout(ADDLINE, "Illegal warp factor.");
	}
}

/* long range scan */
lrs()
{
	int x, y;
	char str[20];

	if (damage[LRS] <= 0)
	    readout(ADDLINE, "Long range sensors are damaged.");
	else {
	    if (rolines > 3)
		readout(CLEAR, NULL);

	    for (y = yquad - 1; y <= yquad + 1; y++) {
		readout(ADDLINE, "+---+---+---+");
		strcpy(str, "|");
		for (x = xquad - 1; x <= xquad + 1; x++) {
		    if (x < 0 || x > 7 || y < 0 || y > 7)
			strcat(str, "XXX");
		    else {
			if (damage[COMPUTER] > 0)
			    galaxy[x][y].known = 1;
			sprintf(str + strlen(str), "%d%d%d", galaxy[x][y].nkling,
			galaxy[x][y].nbase, galaxy[x][y].nstar);
			plt_gal(ELEMENT, x, y);
		    }
		    strcat(str, "|");
		}
		str[13] = '\0';
		readout(ADDLINE, str);
	    }

	    readout(ADDLINE, "+---+---+---+");
	    plt_gal(ELEMENT, xquad, yquad);
	}
}

/* short range scan */
srs()
{
	int k, dx, dy;
	double dir, dist;
	char str[40];

	if (damage[SRS] <= 0)
	    readout(ADDLINE, "Short range sensors are damaged.");
	else {
	    if (rolines > 6)
		readout(CLEAR, NULL);
	    readout(ADDLINE, "Sector  Direction  Distance");
	    for (k = 0; k < 3; k++) {
		if (klingon[k].sh >= 0) {
		    if (damage[COMPUTER] <= 0)
			sprintf(str, "[%d,%d]", klingon[k].xs+1, klingon[k].ys+1);
		    else {
			dx = klingon[k].xs - xsect;
			dy = klingon[k].ys - ysect;
			dist = sqrt((double)(dx*dx) + (double)(dy*dy));
			if (dx) {
			    dir = atan2(-(double)dy, (double)dx) * 180.0 / PI;
			    if (dir < 0.0)
				dir += 360.0;
			}
			else if (dy > 0)
			    dir = 270.0;
			else
			    dir = 90.0;
			sprintf(str, "[%d,%d]     %5.1f      %4.1f", klingon[k].xs+1,
			klingon[k].ys+1, dir, dist);
		    }
		    readout(ADDLINE, str);
		}
	    }
	}
}

findkling(x, y)
int x, y;
{
	int i;

	for (i = 0; i < 3; i++)
	    if (x == klingon[i].xs && y == klingon[i].ys)
		return i;

	return ERROR;
}

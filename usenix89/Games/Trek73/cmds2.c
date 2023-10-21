/*
 * TREK73: cmds2.c
 *
 * User Commands
 *
 * pos_report, pos_display, pursue, helm, self_scan, scan
 *
 * (print_damages)
 *
 */

#include "defines.h"
#include "structs.h"
#include <stdio.h>
#include <strings.h>

extern char title[];
extern char science[];
extern char engineer[];
extern char helmsman[];
extern char nav[];


pos_report(sp)
struct ship *sp;
{
	extern	int rangefind();
	extern	int bearing();
	extern	struct ship *shiplist[];
	extern	struct list head, *tail;
	struct	ship *sp1;
	struct	torpedo *tp;
	struct	list *lp;
	int	x, y;
	int	range;
	int	bear;
	float	speed;
	int	course;
	int	maxlen = 0;
	char	whitespace[5], who[80];

	/*
	 * Go through tht list of objects and find out the longest
	 * name of any of them.  This is to insure that no matter
	 * what the names of the ship, the position report will
	 * come out formatted.
	 */
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		if (lp->type == I_SHIP) {
			sp1 = lp->data.sp;
			strcpy(who, sp1->name);
			maxlen = max(maxlen, strlen(who));
		} else {
			tp = lp->data.tp;
			strcpy(who, tp->from->name);
			switch (lp->type) {
				case I_SHIP:
				case I_TORPEDO: 
					continue;
				case I_PROBE:
					maxlen = max(maxlen, strlen(who) + 9);
				case I_ENG:
					maxlen = max(maxlen, strlen(who) + 12);
					break;
				default:
					break;
			}
		}
	}
	/*
	 * Construct a string %ns where n is the length of the
	 * longest name.
	 */
	sprintf(whitespace, "%%%ds", maxlen);
	/*
	 * And print out the position report
	 */
	printf(whitespace, " ");
	printf(" warp course bearing range\n");
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		sp1 = NULL;
		tp = NULL;
		if (lp->type == I_SHIP) {
			sp1 = lp->data.sp;
			if (sp1->status & S_DEAD)
				continue;
			x = sp1->x;
			y = sp1->y;
			speed = sp1->warp;
			course = sp1->course;
			strcpy(who, sp1->name);
		} else {
			tp = lp->data.tp;
			if (lp->type == I_TORPEDO)
				continue;
			x = tp->x;
			y = tp->y;
			speed = tp->speed;
			course = tp->course;
			strcpy(who, tp->from->name);
			switch(lp->type) {
				case I_PROBE:
					sprintf(who, "%s probe %d", who, tp->id);
					break;
				case I_ENG:
					strcat(who, " engineering");
					break;
				default:
					break;
			}
		}
		bear = bearing(sp->x, x, sp->y, y);
		range = rangefind(sp->x, x, sp->y, y);
		printf(whitespace, who);
		printf(" %4.1f   %3d   ", speed, course);
		if ((sp1 != NULL) && (sp1 == shiplist[0])) {
			if (shiplist[0]->target != NULL)
				if (shiplist[0]->eluding)
					printf("eluding %s\n", shiplist[0]->target->name);
				else
					printf("pursuing %s\n", shiplist[0]->target->name);
			else
				printf("           \n");
		} else
			printf("%3d  %5d\n",  bear, range);
	}
}


pos_display(sp)
struct ship *sp;
{
	extern	struct list head, *tail;
	register int i;
	register int j;
	int	range;
	char	buf1[20];
	int	x, y;
	float	xf, yf;
	int	h, v;
	struct	list *lp;
	struct	ship *sp1;
	struct	torpedo *tp;
	char	map[13][23];
	char	c;

	if (sp->status & S_SENSOR) {
		printf("%s: sensors are damaged.\n", science);
		return 0;
	}
	printf("   display to [100+] ");
	if (Gets(buf1) == NULL)
		return 0;
	range = atoi(buf1);
	if (range < 100 || range > 10000)
		return 0;
	/*
	 * Compensation for aspect ratio of the output device
	 */
	x = range/10;
	y = range/6;
	for (i=0; i<13; i++) {
		if (i == 0 || i == 12)
			strcpy(map[i], "---------------------");
		else
			strcpy(map[i], "                     ");
	}
	map[6][10] = '+';
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->data.sp == sp)
			continue;
		if (lp->type == I_SHIP) {
			sp1 = lp->data.sp;
			xf = sp1->x - sp->x;
			yf = sp1->y - sp->y;
		} else {
			tp = lp->data.tp;
			xf = tp->x - sp->x;
			yf = tp->y - sp->y;
		}
		v = yf/y + 6.5;
		h = xf/x + 10.5;
		if (v < 0 || v > 12)
			continue;
		if (h < 0 || h > 20)
			continue;
		switch (lp->type) {
			case I_SHIP:
				c = lp->data.sp->name[0];
				break;
			case I_TORPEDO:
				c = ':';
				break;
			case I_ENG:
				c = '#';
				break;
			case I_PROBE:
				c = '*';
				break;
			default:
				c = '?';
				break;
		}
		map[12 - v][h] = c;
	}
	for (i=0; i<13; i++) {
		for (j=0; j<21; j++)
			if (map[i][j] != ' ')
				break;
		if (j != 21)
			printf("%s", map[i]);
		printf("\n");
	}
	return 0;
}


pursue(sp)
struct ship *sp;
{
	extern	struct ship *ship_name();
	extern	double atof();
	register int i;
	char	buf1[20];
	struct	ship *ep;
	float	warp;

	if (sp->status & S_COMP) {
		printf("%s: Impossible, %s, our computer is dead\n",science ,title);
		return 0;
	}
	printf("   Mr. %s, pursue [who] ", nav);
	if (Gets(buf1) == NULL)
		return 0;
	ep = ship_name(buf1,ENEMYONLY);
	if (ep == NULL)
		return 0;
	printf("   Mr. %s, warp factor [-9 to 9] ", helmsman);
	if (Gets(buf1) == NULL)
		return 0;
	warp = (float) atof(buf1);
	if (warp > 9.0 || warp < -9.0) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		return 0;
	}
	if ((warp > 1.0 || warp < -1.0) && (sp->status & S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = warp < 0.0 ? -1.0 : 1.0;
	}
	sp->newwarp = warp;
	sp->target = ep;
	/* Set eluding flag to 0, which means pursue */
	sp->eluding = 0;
	i = bearing(sp->x, ep->x, sp->y, ep->y);
	printf("%s: Aye, %s, coming to course %d.\n", nav, title, i);
	sp->newcourse = i;
	return 1;
}


elude(sp)
struct ship *sp;
{
	extern	struct ship *ship_name();
	extern	double atof();
	register int i;
	char	buf1[20];
	struct	ship *ep;
	float	warp;

	if (sp->status & S_COMP) {
		printf("%s: impossible, %s, our computer is dead\n",science, title);
		return 0;
	}
	printf("   Mr. %s, elude [who] ", nav);
	if ((Gets(buf1) == NULL) || (buf1 == NULL))
		return 0;
	ep = ship_name(buf1,ENEMYONLY);
	if (ep == NULL)
		return 0;
	printf("   Mr. %s, warp factor [-9 to 9] ", helmsman);
	if (Gets(buf1) == NULL)
		return 0;
	warp = (float) atof(buf1);
	if (warp > 9.0 || warp < -9.0) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		return 0;
	}
	if ((warp > 1.0 || warp < -1.0) && (sp->status & S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = warp < 0.0 ? -1.0 : 1.0;
	}
	sp->newwarp = warp;
	sp->target = ep;
	/* Set eluding flag to 1, which means elude */
	sp->eluding = 1;
	i = bearing(sp->x, ep->x, sp->y, ep->y);
	i = rectify(i + 180);
	printf("%s: Aye, %s, coming to course %d.\n", nav, title, i);
	sp->newcourse = i;
	return 1;
}

helm(sp)
struct ship *sp;
{
	extern	double atof();
	char	buf1[20];
	register int course;
	float	warp;

	printf("   Mr. %s, come to course [0-359] ", nav);
	if (Gets(buf1) == NULL)
		return 0;
	course = atoi(buf1);
	if (course < 0 || course > 360)
		return 0;
	printf("   Mr. %s, warp factor [-9 to 9] ", helmsman);
	if (Gets(buf1) == NULL)
		return 0;
	warp = (float) atof(buf1);
	if (warp > 9.0 || warp < -9.0) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		return 0;
	}
	if ((warp > 1.0 || warp < -1.0) && (sp->status & S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = warp < 0.0 ? -1.0 : 1.0;
	}
	sp->newwarp = warp;
	sp->newcourse = course;
	sp->target = NULL;
	sp->eluding = 0;
	printf("%s: aye, %s.\n", nav, title);
	return 1;
}

self_scan(sp)
struct ship *sp;
{
	struct ship *ep;
	extern struct ship *shiplist[];

	ep = shiplist[0];			/* Scanning ourself */
	print_damage(ep);
	sp = sp;				/* LINT */
	return 1;
}

scan(sp)
struct ship *sp;
{
	struct	ship *ep;
	char	buf1[20];

	printf("   %s, scan [who] ", science);
	if (Gets(buf1) == NULL)
		return 0;
	ep = ship_name(buf1,ENEMYONLY);
	if (ep == NULL)
		return 0;
	if (sp != ep && (sp->status & S_SENSOR)) {
		printf ("%s: The sensors are damaged, Captain.\n", science);
		return 0;
	}
	if (sp == ep) {
		printf ("%s: Captain, don't you mean 'Damage Report'?\n", science);
		return 0;
	}
	print_damage(ep);
	return 1;
}

print_damage(ep)
struct ship *ep;
{	
	extern char *statmsg[];
	register int i;
	register int j;
	register float k;

	printf("\n\nDamages to the %s\n", ep->name);
	for (i=0; i<5; i++) {
		if (ep->status & 1<<i)
			printf("%s.\n", statmsg[i]);
	}
	printf("survivors: %d\n", ep->crew);
	printf("\nPhasers Control");
	for (i=0; i<4; i++) {
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\tdamaged");
		else if (ep->phasers[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", ep->phasers[i].target->name);
	}
	printf("\n\t turned");
	for (i=0; i<4; i++)
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\t");
		else if (ep->phasers[i].target == NULL)
			printf("\t%d", ep->phasers[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n\t  level");
	for (i=0; i<4; i++)
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\t");
		else
			printf("\t%-2d", ep->phasers[i].load);
	printf("\n");
	printf("\nTubes\tcontrol");
	for (i=0; i<6; i++) {
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\tdamaged");
		else if (ep->tubes[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", ep->tubes[i].target->name);
	}
	printf("\n\t turned");
	for (i=0; i<6; i++)
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\t");
		else if (ep->tubes[i].target == NULL)
			printf("\t%d", ep->tubes[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n\t  level");
	for (i=0; i<6; i++)
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\t");
		else
			printf("\t%-2d", ep->tubes[i].load);
	printf("\n");
	printf("\nShields\t levels");
	for (i=0; i<4; i++) {
		j = 100 * ep->shields[i].eff * ep->shields[i].drain;
		printf("\t%-2d", j);
	}
	printf("\n\t drains");
	for (i=0; i<4; i++) {
		k = ep->shields[i].attemp_drain;
		printf("\t%-4.2f", k);
	}
	printf("\n\nefficiency: %3.1f\t\tfuel remaining: %d\n",
		ep->eff, ep->energy);
	printf("regeneration: %4.1f\tfuel capacity: %d\n",
		ep->regen, ep->pods);
	return 1;
}

/*
 * TREK73: cmds2.c
 *
 * User Commands
 *
 * pos_report, pos_display, pursue, elude, helm, self_scan, scan
 *
 * (print_damages)
 *
 */

#include "externs.h"
#include <ctype.h>

pos_report(sp)
struct ship *sp;
{
	struct	ship *sp1;
	struct	torpedo *tp;
	struct	list *lp;
	int	x, y;
	int	range;
	float	bear;
	float	speed;
	float	course;
	float	relbear;
	float	revrelbear;
	int	maxlen = 0;
	int	incltorp = 0;
	char	whitespace[5], who[80];
#ifdef SHOWTORP
	char	buf[20];
#endif

#ifdef SHOWTORP
	printf("%s:  Include torpedoes?\n", science);
	printf("%s:  [yes or no] ", captain);
	if (Gets(buf, sizeof buf) != NULL && (buf[0] == 'y' || buf[0] == 'Y'))
		incltorp = 1;
#endif
	/*
	 * Go through the list of objects and find out the longest
	 * name of any of them.  This is to insure that no matter
	 * what the names of the ship, the position report will
	 * come out formatted.
	 */
	for (lp = &head; lp != tail; lp = lp->fwd)
		switch(lp->type) {
		case I_SHIP:
			sp1 = lp->data.sp;
			maxlen = max(maxlen, strlen(sp1->name));
			break;
#ifdef SHOWTORP
		case I_TORPEDO:
			if (!incltorp)
				break;
			tp = lp->data.tp;
			maxlen = max(maxlen, strlen(tp->from->name) + 8);
			break;
#endif
		case I_PROBE:
			tp = lp->data.tp;
			maxlen = max(maxlen, strlen(tp->from->name) + 9);
			break;
		case I_ENG:
			tp = lp->data.tp;
			maxlen = max(maxlen, strlen(tp->from->name) + 12);
			break;
		}
	maxlen += 2;			/* For "cloaked" column */
	/*
	 * Construct a string %ns where n is the length of the
	 * longest name.
	 */
	(void) sprintf(whitespace, "%%%ds", maxlen);
	/*
	 * And print out the position report
	 */
	printf(whitespace, " ");
	puts("                     abs           rel   rev rel");
	printf(whitespace, " ");
	puts(" class warp course bearing range bearing bearing");
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		sp1 = NULL;
		tp = NULL;
		if (lp->type == I_SHIP) {
			sp1 = lp->data.sp;
			if (is_dead(sp1, S_DEAD))
				continue;
			if (cansee(sp1)) {
				x = sp1->x;
				y = sp1->y;
				speed = sp1->warp;
				course = sp1->course;
				/* Reset what we know about his position */
				sp1->position.x = sp1->x;
				sp1->position.y = sp1->y;
				sp1->position.warp = sp1->warp;
				sp1->position.course = sp1->course;
				sp1->position.bearing = bearing(sp->x,
				    sp1->x, sp->y, sp1->y);
				sp1->position.range = rangefind(sp->x,
				    sp1->x, sp->y, sp1->y);
				(void) sprintf(who, "%.*s  ",
				    sizeof who - 3, sp1->name);
			} else {
				x = sp1->position.x;
				y = sp1->position.y;
				speed = sp1->position.warp;
				course = sp1->position.course;
				(void) sprintf(who, "%.*s *",
				    sizeof who - 3, sp1->name);
			}
		} else {
			tp = lp->data.tp;
			if (lp->type == I_TORPEDO && !incltorp)
				continue;
			x = tp->x;
			y = tp->y;
			speed = tp->speed;
			course = tp->course;
			switch(lp->type) {
#ifdef SHOWTORP
				case I_TORPEDO:
					(void) sprintf(who,"%.*s torp %d  ",
					    sizeof who - 11, tp->from->name,
					    tp->id);
					break;
#endif
				case I_PROBE:
					(void) sprintf(who, "%.*s probe %d  ",
					    sizeof who - 12, tp->from->name,
					    tp->id);
					break;
				case I_ENG:
					(void) sprintf(who,"%.*s engineering  ",
					    sizeof who - 15, tp->from->name);
					break;
				default:
					(void) sprintf(who, "lp->type = %d  ",
					    lp->type);
					break;
			}
		}
		printf(whitespace, who);
		if (sp1)
			printf("%5s", sp1->class);
		else
			printf("     ");
		printf("%6.1f   %3.0f   ", speed, course);
		if (sp1 == sp) {
			if (sp->target != NULL)
				printf("helm locked on %s", sp->target->name);
			putchar('\n');
		} else {
			bear = bearing(sp->x, x, sp->y, y);
			range = rangefind(sp->x, x, sp->y, y);
			relbear = rectify(round(bear - sp->course));
			revrelbear = rectify(round(bear + 180.0 - course));
			printf(" %3.0f   %5d   %3.0f     %3.0f\n",
			    bear, range, relbear, revrelbear);
		}
	}
	return 0;
}


pos_display(sp)
struct ship *sp;
{
	register int i;
	register int j;
	int	range;
	char	buf1[20];
	int	x, y;
	float	xf, yf;
	int	h, v;
	int 	hpitch = 10;
	int	vpitch = 6;
	char	map[13][23];	/* [2*MAXvpitch + 1][2*MAXhpitch + 1] */
	struct	list *lp;
	struct	ship *sp1;
	struct	torpedo *tp;
	char	c;

	if (is_dead(sp, S_SENSOR)) {
		printf("%s: Sensors are damaged.\n", science);
		return 0;
	}
	if (!syswork(sp, S_SENSOR)) {
		printf("%s: Sensors are temporarily inoperative.\n",
		    science);
		return 0;
	}
	printf("   display to [%d-%d] ", MIN_SENSOR_RANGE, MAX_SENSOR_RANGE);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	range = atoi(buf1);
	if (range < MIN_SENSOR_RANGE || range > MAX_SENSOR_RANGE)
		return 0;
	/*
	 * Compensation for aspect ratio of the output device
	 */
	x = range/hpitch;
	y = range/vpitch;
	for (i=0; i<=2*vpitch; i++) {
		if (i == 0 || i == 2*vpitch)
			for(j=0; j<=2*hpitch; j++)
				map[i][j] = '-';
		else
			for(j=0; j<=2*hpitch; j++)
				map[i][j] = ' ';
	}
	map[vpitch][hpitch] = '+';
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->data.sp == sp)
			continue;
		if (lp->type == I_SHIP) {
			sp1 = lp->data.sp;
			if (cansee(sp1)) {
				/* Update the position */
				sp1->position.x = sp1->x;
				sp1->position.y = sp1->y;
				sp1->position.warp = sp1->warp;
				sp1->position.course = sp1->course;
				sp1->position.bearing = bearing(sp->x, x, sp->y, y);
				sp1->position.range = rangefind(sp->x, x, sp->y, y);
				xf = sp1->x - sp->x;
				yf = sp1->y - sp->y;
			} else {
				xf = sp1->position.x - sp->x;
				yf = sp1->position.y - sp->y;
			}
		} else {
			tp = lp->data.tp;
			xf = tp->x - sp->x;
			yf = tp->y - sp->y;
		}
		v = yf/y + vpitch + 0.5;
		h = xf/x + hpitch + 0.5;
		if (v < 0 || v > 2*vpitch)
			continue;
		if (h < 0 || h > 2*hpitch)
			continue;
		switch (lp->type) {
			case I_SHIP:
				c = lp->data.sp->name[0];
				if (cantsee(lp->data.sp))
					c = tolower(c);
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
		map[2*vpitch - v][h] = c;
	}
	for (i=0; i<=2*vpitch; i++) {
		for (j=0; j<=2*hpitch; j++)
			if (map[i][j] != ' ')
				break;
		if (j <= 2*hpitch)
			printf("%.*s", 2*hpitch + 1, map[i]);
		putchar('\n');
	}
	return 0;
}


pursue(sp)
struct ship *sp;
{
	register float i;
	char	buf1[20];
	struct	ship *ep;
	float	warp;
	
	if (is_dead(sp, S_COMP)) {
		printf("%s: Impossible, %s, our computer is dead\n",science ,title);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s: Main computer down, %s.  Rebooting.\n",
		    science, title);
		return 0;
	}
	printf("   Mr. %s, pursue [who] ", nav);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	ep = ship_name(buf1);
	if (ep == NULL)
		return 0;
	if (cantsee(ep)) {
		printf("%s:  %s, unable to acquire helm lock.\n", nav, title);
		return 0;
	}
	printf("   Mr. %s, warp factor [-%.2f to %.2f] ", helmsman, 
	    sp->max_speed, sp->max_speed);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	warp = atof(buf1);
	if (fabs(warp) > 1.0 && is_dead(sp, S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = (warp < 0.0) ? -1.0 : 1.0;
	}
	if (fabs(warp) > sp->max_speed) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		warp = (warp < 0.0) ? -sp->max_speed : sp->max_speed;
	}
	sp->newwarp = warp;
	sp->target = ep;
	sp->relbear = 0.0;
	i = bearing(sp->x, ep->x, sp->y, ep->y);
	printf("%s: Aye, %s, coming to course %3.0f.\n", nav, title, i);
	sp->newcourse = i;
	return 1;
}


elude(sp)
struct ship *sp;
{
	register float i;
	char	buf1[20];
	struct	ship *ep;
	float	warp;

	if (is_dead(sp, S_COMP)) {
		printf("%s: Impossible, %s, our computer is dead\n",
		    science, title);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s: Main computer down, %s.  Rebooting.\n",
		    science, title);
		return 0;
	}
	printf("   Mr. %s, elude [who] ", nav);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	ep = ship_name(buf1);
	if (ep == NULL)
		return 0;
	if (cantsee(ep)) {
		printf("%s:  %s, unable to acquire helm lock.\n",
		    nav, title);
		return 0;
	}
		
	printf("   Mr. %s, warp factor [-%.2f to %.2f] ", helmsman, 
	    sp->max_speed, sp->max_speed);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	warp = (float) atof(buf1);
	if (fabs(warp) > 1.0 && is_dead(sp, S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = (warp < 0.0) ? -1.0 : 1.0;
	}
	if (fabs(warp) > sp->max_speed) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		warp = (warp < 0.0) ? -sp->max_speed : sp->max_speed;
	}
	sp->newwarp = warp;
	sp->target = ep;
	sp->relbear = 180.0;
	i = bearing(sp->x, ep->x, sp->y, ep->y);
	i = rectify(i + 180.0);
	printf("%s: Aye, %s, coming to course %3.0f.\n", nav, title, i);
	sp->newcourse = i;
	return 1;
}

helm(sp)
struct ship *sp;
{
	char	buf1[20];
	register float course;
	float	warp;

	printf("   Mr. %s, come to course [0-359] ", nav);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	course = atof(buf1);
	if (course < 0.0 || course >= 360.0)
		return 0;
	printf("   Mr. %s, warp factor [-%.2f to %.2f] ", helmsman, 
	    sp->max_speed, sp->max_speed);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	warp = (float) atof(buf1);
	if (fabs(warp) > 1.0 && is_dead(sp, S_WARP)) {
		printf("%s: Warp drive is dead, Captain.\n", science);
		warp = (warp < 0.0) ? -1.0 : 1.0;
	}
	if (fabs(warp) > sp->max_speed) {
		printf("%s: %s, the engines canna go that fast!\n",engineer, title);
		warp = (warp < 0.0) ? -sp->max_speed : sp->max_speed;
	}
	sp->newwarp = warp;
	sp->newcourse = course;
	sp->target = NULL;
	sp->relbear = 0.0;
	printf("%s: Aye, %s.\n", nav, title);
	printf("%s: Aye, %s.\n", helmsman, title);
	return 1;
}

self_scan(sp)
struct ship *sp;
{
	(void) print_damage(sp);
	return 1;
}

scan(sp)
struct ship *sp;
{
	struct	ship *ep = NULL;
	struct	torpedo *tp = NULL;
	struct	list *lp;
	int	item = I_UNDEFINED;
	int	probe_num;
	char	buf1[20];

	printf("   %s, scan [who] ", science);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	if (buf1[0] == '#') {
		strcpy(buf1,buf1+1);
		if (strlen(buf1) == 0) {
			printf("%s: %s, scan whose engineering?\n", science, title);
			return 0;
		}
		ep = ship_name(buf1);
		if (ep == NULL) {
			printf("%s: %s, no such ship as the %s.\n", science, title, buf1);
			return 0;
		}
		for (lp = &head; lp != NULL; lp = lp->fwd) {
			if (lp == tail)
				break;
			if (lp->type == I_UNDEFINED)
				continue;
			if (lp->type == I_ENG && lp->data.tp->from == ep) {
				tp = lp->data.tp;
				break;
			}
		}
		if (tp == NULL) {
			printf("%s:  %s, the %s has not jettisoned it's engineering.\n",
			    science, title, ep->name);
			return 0;
		}
		item = I_ENG;
	} else if ((probe_num = atoi(buf1)) > 0) {
		for (lp = &head; lp != NULL; lp = lp->fwd) {
			if (lp == tail)
				break;
			if (lp->type != I_PROBE)
				continue;
			if (lp->data.tp->id == probe_num) {
				tp = lp->data.tp;
				break;
			}
		}
		if (tp == NULL) {
			printf("%s: %s, there is no probe %d", science, title, probe_num);
			return 0;
		}
		item = I_PROBE;
	} else {
		ep = ship_name(buf1);
		if (ep == NULL) {
			printf("%s: %s, no such ship as the %s.\n", science, title, buf1);
			return 0;
		}
		item = I_SHIP;
		if (cantsee(ep)) {
			printf("%s:  %s, I am unable to scan the %s.\n",
			    science, title, ep->name);
			return 0;
		}
	}
	if ((sp != ep) && (is_dead(sp, S_SENSOR))) {
		printf ("%s: The sensors are damaged, Captain.\n",
		    science);
		return 0;
	}
	if ((sp != ep) && (!syswork(sp, S_SENSOR))) {
		printf("%s: %s, sensors are temporarily out.\n",
		    science, title);
		return 0;
	}
	if ((sp == ep) && (item == I_SHIP)) {
		printf ("%s: Captain, don't you mean 'Damage Report'?\n", science);
		return 0;
	}
	if (item == I_SHIP)
		(void) print_damage(ep);
	else
		(void) scan_torpedo(tp);
	return 1;
}

print_damage(ep)
struct ship *ep;
{	
	register int i;
	register int j;
	register float k;

	printf("\n\nDamages to the %s\n", ep->name);
	for (i=0; i<S_NUMSYSTEMS + 1; i++) {
		if (is_dead(ep, i))
			printf("%s.\n", statmsg[i]);
		else if (ep->status[i] && i < S_NUMSYSTEMS)
			printf("%s damaged %d%%\n", sysname[i], ep->status[i]);
	}
	printf("Survivors: %d\n", ep->complement);
	printf("Helm lock: ");
	if (ep->target == NULL)
		printf("none.\n");
	else
		printf("%s\n",ep->target);
	printf("\nPhasers Control");
	for (i=0; i<ep->num_phasers; i++) {
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\tdamaged");
		else if (ep->phasers[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", ep->phasers[i].target->name);
	}
	printf("\n\t turned");
	for (i=0; i<ep->num_phasers; i++)
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\t");
		else if (ep->phasers[i].target == NULL)
			printf("\t%.0f", ep->phasers[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n\t  level");
	for (i=0; i<ep->num_phasers; i++)
		if (ep->phasers[i].status & P_DAMAGED)
			printf("\t");
		else
			printf("\t%-2d", ep->phasers[i].load);
	printf("\n");
	printf("\nTubes\tcontrol");
	for (i=0; i<ep->num_tubes; i++) {
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\tdamaged");
		else if (ep->tubes[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", ep->tubes[i].target->name);
	}
	printf("\n\t turned");
	for (i=0; i<ep->num_tubes; i++)
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\t");
		else if (ep->tubes[i].target == NULL)
			printf("\t%.0f", ep->tubes[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n\t  level");
	for (i=0; i<ep->num_tubes; i++)
		if (ep->tubes[i].status & T_DAMAGED)
			printf("\t");
		else
			printf("\t%-2d", ep->tubes[i].load);
	printf("\n");
	printf("\nShields\t levels");
	for (i=0; i<SHIELDS; i++) {
		j = 100 * ep->shields[i].eff * ep->shields[i].drain;
		printf("\t%-2d", j);
	}
	printf("\n\t drains");
	for (i=0; i<SHIELDS; i++) {
		k = ep->shields[i].attemp_drain;
		printf("\t%-4.2f", k);
	}
	printf("\n\nEfficiency: %4.1f\tFuel remaining: %d\n",
		ep->eff, (int)ep->energy);
	printf("Regeneration: %4.1f\tFuel capacity: %d\n",
		ep->regen, (int)ep->pods);
	return 1;
}

scan_torpedo(pp)
struct torpedo *pp;
{
	char kind[20];
	char tgt[20];

	printf("\nobject id time  prox units target\n");
	if (pp->type == TP_PROBE)
		strcpy(kind, "probe");
	else
		strcpy(kind, "engng");
	if (pp->target == NULL)
		strcpy(tgt, "NONE");
	else
		strcpy(tgt, pp->target->name);
	printf("%-7s%2d  %2.1f %4d   %3d   %s\n",
		kind, pp->id, pp->timedelay, pp->prox, pp->fuel, tgt);
	return 1;
}

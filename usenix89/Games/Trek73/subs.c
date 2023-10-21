/*
 * TREK73: subs.c
 *
 * Miscellaneous Subroutines
 *
 * ship_name, newitem, delitem, rangefind, bearing, phaser_hit,
 * torpedo_hit, damage, antimatter_hit, recitfy
 */

#include "defines.h"
#include "structs.h"
#include <math.h>
#include <ctype.h>

extern struct ship *shiplist[];
extern char science[];

struct ship *ship_name(name, start)
char *name;
int start;
{
	extern	int shipnum;
	register int i;
	register int j;
	register int len;

	if (islower(*name)) {
		*name = toupper(*name);
	}
	j = shipnum;
	len = strlen(name);
	for (i=start; i<=j; i++) {
		if (shiplist[i]->name[0] == NULL)
			continue;
		if (strncmp(name, shiplist[i]->name, len) == 0)
			return shiplist[i];
	}
	printf("%s: I am unable to find the %s\n", science, name);
	return NULL;
}


struct list *newitem(item)
int item;
{
	extern	struct list head;
	extern	struct list *tail;
	register struct	list *new;
	register struct	list *newtail;

	/*
	 * if there's no "tail" node, make one (only happens at init)
	 */
	if (tail == NULL) {
		new = MKNODE(struct list, *, 1);
		new->back = &head;
		new->fwd = NULL;
		new->data.tp = NULL;
		head.fwd = new;
		tail = new;
	}
	new = tail;
	/*
	 * now make the new tail node
	 */
	newtail = MKNODE(struct list, *, 1);
	newtail->back = new;
	newtail->fwd = NULL;
	newtail->data.tp = NULL;
	newtail->type = 0;
	tail = newtail;
	/*
	 * link the old tail node to the new one
	 */
	new->type = item;
	new->fwd = newtail;
	return new;
}


int delitem(item)
struct list *item;
{
	extern	struct list *tail;
	extern	struct list head;
	register struct list *bp;
	register struct list *fp;

	bp = item->back;
	fp = item->fwd;
	if (item->data.tp != NULL)
		free((char *) item->data.tp);
	/*
	 * re-arrange pointers on both the next and the previous
	 * nodes; if no forward pointer, we were the tail so make
	 * the bp the new tail node.
	 */
	if (fp != NULL) {
		bp->fwd = fp;
		fp->back = bp;
	} else {
		tail = bp;
		bp->fwd = NULL;
	}
	free((char *) item);
}

int rangefind(x1, x2, y1, y2)
int x1;
int x2;
int y1;
int y2;
{
	extern	double sqrt();
	extern	double atan();
	register int i;
	register int x, y;
	double	d1, d2;

	x = x2 - x1;
	y = y2 - y1;
	/*
	 * Both x and y must be cast as double else overflow
	 * may occur.
	 */
	d1 = (double) x * (double) x + (double) y * (double) y;
	d2 = sqrt(d1);
	i = d2;
	return i;
}

/*
 * This routine finds the bearing of (x2,y2) from (x1,y1)
 */
int bearing(x1, x2, y1, y2)
int x1;
int x2;
int y1;
int y2;
{
	extern double atan();
	float	x;
	float	y;
	register int bear;
	double	d1;
	double	d2;

	x = x2 - x1;
	y = y2 - y1;
	if (x == 0.0)
		bear = 90;
	else {
		d1 = y/x;
		d2 = atan(d1) * 57.2958;
		bear = d2;
	}
	if (x < 0.0 || y < 0.0) {
		bear += 180;
		if (x >= 0.0)
			bear += 180;
	}
	return bear;
}

int phaser_hit(sp, x, y, bank, true_bear)
struct ship *sp;
int x;
int y;
struct phaser *bank;
int true_bear;
{
	extern	double sqrt();
	register int hit;
	int	i;
	int	spread;
	int	bear;
	double	d1;
	double	d2;

	hit = 0;
	i = rangefind(sp->x, x, sp->y, y);
	if (i < 1000) {
		bear = bearing(sp->x, x, sp->y, y);
		spread = rectify(true_bear - bear);
		/*
		 * Check if a target is within the phaser spread
		 */
		if (spread > sp->p_spread && 360-spread > sp->p_spread)
			return 0;
		d1 = 1.0 - (float)i/1000.0;
		d2 = (float)bank->load * sqrt(d1) * sp->p_percent / 100;
		d2 = (float)bank->load * d2 * 45.0/(float)sp->p_spread * sp->p_percent / 100;
		hit = d2/10.0;
	}
	return hit;
}

int torpedo_hit(fuel, x, y, tx, ty)
int fuel;
int x;
int y;
int tx;
int ty;
{
	extern	double sqrt();
	register int hit;
	int	i;
	double	d1;
	double	d2;
	float	f1;
	float	f2;

	hit = 0;
	i = rangefind(x, tx, y, ty);
	f1 = fuel * 5.0;
	f2 = f1 * 10.0;
	if (i < f2) {
		d1 = 1.0 - (float)i/f2;
		d2 = (float)f1 * sqrt(d1);
		hit = d2;
	}
	return hit;
}


damage(hit, ep, s, dam)
int hit;
struct ship *ep;
int s;
struct damage *dam;
{
	register int i;
	register int j;
	register int k;
	float	f1;
	float	f2;
	struct ship *fed;

	fed = shiplist[0];
	printf("hit %d on %s's shield %d\n", hit, ep->name, s);
	s--;
	/*
	 * Note that if the shield is at 100% efficiency, no
	 * damage at all will be taken
	 */
	f1 = hit * (1.0 - ep->shields[s].eff * ep->shields[s].drain);
	if (f1 < 0)
		return 0;
	ep->eff += f1/dam->eff;
	ep->pods -= f1/dam->fuel;
	ep->energy -= f1/dam->fuel;
	ep->regen -= f1/dam->regen;
	if (ep->regen < 0.0)
		ep->regen = 0.0;
	if (ep->pods < 0.0)
		ep->pods = 0.0;
	if (ep->energy < 0.0)
		ep->energy = 0.0;
	if (ep->pods < ep->energy)
		ep->energy = ep->pods;
	f2 = dam->shield * 100;
	if (s == 0)
		f2 *= 1.5;
	ep->shields[s].eff -= max(hit/f2, 0);
	if (ep->shields[s].eff < 0.0)
		ep->shields[s].eff = 0.0;
	j = f1 * dam->crew;
	if (j > 0)
		ep->crew -= max(randm(j), 0);
	if (ep->crew < 0)
		ep->crew = 0;
	j = f1/dam->weapon;
	for(i=0; i<j; i++) {
		k = randm(10);
		if (k <= 4) {
			k--;
			if (ep->phasers[k].status & P_DAMAGED)
				continue;
			ep->phasers[k].status |= P_DAMAGED;
			/*
			 * Reroute the energy
			 * back to the engines
			 */
			ep->energy = min(ep->pods, ep->energy
			    + ep->phasers[k].load);
			ep->phasers[k].load = 0;
			ep->phasers[k].drain = 0;
			k++;
			if (ep == fed)
				printf("   phaser %d damaged.\n", k);
		} else {
			k -= 5;
			if (ep->tubes[k].status & T_DAMAGED)
				continue;
			/*
			 * If tubes are damaged, reroute the pods
			 * back to the engines
			 */
			ep->pods += ep->tubes[k].load;
			ep->energy += ep->tubes[k].load;
			ep->tubes[k].load = 0;
			ep->tubes[k].status |= T_DAMAGED;
			k++;
			if (ep == fed)
				printf("   tube %d damaged\n", k);
		}
	}
	for (i=0; i<4; i++) {
		if (ep->status & 1<<i)
			continue;
		if (randm(dam->stats[i].roll) < f1) {
			ep->status |= 1<<i;
			if (ep == fed)
				printf("   %s\n", dam->stats[i].mesg);
		}
	}
#ifdef HISTORICAL
	/*
	 * Historically, if more than 43 points of damage were done
	 * to the ship, it would destroy itself.  This led to much
	 * abuse of probes and thus has been enclosed inside of
	 * an #ifdef
	 */
	if (f1 > 43)
		ep->delay = 1;
#endif
	return 0;
}


antimatter_hit(ptr, x, y, fuel)
char *ptr;
int x;
int y;
int fuel;
{
	extern	struct list head;
	extern	struct list *tail;
	extern	struct damage a_damage;
	register struct list *lp;
	register int hit;
	int	tarx, tary;
	int	s;
	int	bear;
	struct 	torpedo *tp;
	struct	ship *sp;

	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		sp = NULL;
		tp = NULL;
		if (lp->type == I_SHIP) {
			sp = lp->data.sp;
			tarx = sp->x;
			tary = sp->y;
		} else {
			tp = lp->data.tp;
			tarx = tp->x;
			tary = tp->y;
		}
		if (sp == (struct ship *) ptr || tp == (struct torpedo *) ptr)
			continue;
		hit = torpedo_hit(fuel, x, y, tarx, tary);
		if (hit <= 0)
			continue;
		if (sp) {
			/* 
			 * Determine which shield is hit
			 */
			bear = rectify(bearing(tarx, x, tary, y) - sp->course);
			if (bear < 45 || bear > 315)
				s = 1;
			else if (bear < 135)
				s = 2;
			else if (bear < 225)
				s = 3;
			else
				s = 4;
			damage(hit, sp, s, &a_damage);
		} else {
			if (tp->timedelay <= 2)
				continue;
			tp->timedelay = 2;
			switch (lp->type) {
				case I_TORPEDO:
					printf("hit on torpedo %d\n", 
						tp->id);
					break;
				case I_PROBE:
					printf("hit on probe %d\n", 
						tp->id);
					break;
				case I_ENG:
					printf("hit on %s engineering\n",
						tp->from->name);
					break;
			}
		}
	}
}

int rectify(x)
int x;
{
	register int ret;

	ret = x;
	if (ret < 0)
		ret += 360;
	else if (ret > 360)
		ret -= 360;
	return ret;
}

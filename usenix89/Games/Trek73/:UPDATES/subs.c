/*
 * TREK73: subs.c
 *
 * Miscellaneous Subroutines
 *
 * ship_name, newitem, delitem, rangefind, bearing, phaser_hit,
 * torpedo_hit, antimatter_hit, round, rectify
 */

#include "externs.h"
#include <math.h>
#include <ctype.h>


struct ship *ship_name(name)
char *name;
{
	register int i;
	register int j;
	register int len;

	if (isascii(*name) && islower(*name))
		*name = toupper(*name);
	j = shipnum;
	len = strlen(name);
	for (i=1; i<=j; i++) {
		if (shiplist[i]->complement < 0)
			continue;
		if (!strncmp(name, shiplist[i]->name, len))
			return shiplist[i];
	}
	printf("%s: I am unable to find the %s\n", science, name);
	return NULL;
}


struct list *newitem(item)
int item;
{
	register struct	list *new;
	register struct	list *newtail;

	/*
	 * if there's no "tail" node, make one (only happens at init)
	 */
	if (tail == NULL) {
		new = MKNODE(struct list, *, 1);
		if (new == (struct list *)NULL) {
			fprintf(stderr, "newitem: malloc failed\n");
			exit(2);
		}
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
	if (newtail == (struct list *)NULL) {
		fprintf(stderr, "newitem: malloc failed\n");
		exit(2);
	}
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

int rangefind(xfrom, xto, yfrom, yto)
int xfrom;
int xto;
int yfrom;
int yto;
{
	register double x, y;

	x = xto - xfrom;
	y = yto - yfrom;
	if (x == 0.0 && y == 0.0)
		return 0;
	else
	return (int) hypot(x, y);
}

/*
 * This routine finds the bearing of (xto,yto) from (xfrom,yfrom)
 */
float bearing(xfrom, xto, yfrom, yto)
int xfrom;
int xto;
int yfrom;
int yto;
{
	register double x, y;
	register float bear;

	x = xto - xfrom;
	y = yto - yfrom;
	if (x == 0.0 && y == 0.0)
		bear = 0.0;
	else
		bear = todegrees(atan2(y, x));
	bear = rectify(bear);
	return bear;
}

int phaser_hit(sp, x, y, bank, true_bear)
struct ship *sp;
int x;
int y;
struct phaser *bank;
float true_bear;
{
	register int hit;
	int	i;
	float	spread;
	float	bear;
	double	d1;
	double	d2;

	hit = 0;
	i = rangefind(sp->x, x, sp->y, y);
	if (i < MAX_PHASER_RANGE) {
		bear = bearing(sp->x, x, sp->y, y);
		spread = rectify(true_bear - bear);
		/*
		 * Check if a target is within the phaser spread
		 */
		if (betw(spread, sp->p_spread, 360-spread))
			return 0;
		d1 = 1.0 - (float)i/MAX_PHASER_RANGE;
		d2 = (float)bank->load * sqrt(d1) * sp->p_percent / 100;
		/* XXXX */
		/*
		 * This may have to be changed if phaser spread or maximum
		 * phaser load is changed
		 */
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
	register int hit;
	int	i;
	double	d1;
	double	d2;
	float	f1;
	float	f2;

	hit = 0;
	i = rangefind(x, tx, y, ty);
	f1 = fuel * HIT_PER_POD;
	f2 = fuel * PROX_PER_POD;
	if (i < f2) {
		d1 = 1.0 - (float)i/f2;
		d2 = (float)f1 * sqrt(d1);
		hit = d2;
	}
	return hit;
}


antimatter_hit(ptr, x, y, fuel)
char *ptr;
int x;
int y;
int fuel;
{
	register struct list *lp;
	register int hit;
	int	tarx, tary;
	int	s;
	float	bear;
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
			if (bear <= 45.0 || bear >= 315.0)
				s = 1;
			else if (bear <= 135.0)
				s = 2;
			else if (bear < 225.0)
				s = 3;
			else
				s = 4;
			(void) damage(hit, sp, s, &a_damage, D_ANTIMATTER);
		} else {
			if (tp->timedelay <= segment)
				continue;
			tp->timedelay = segment;
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
				default:
					printf("hit on unknown item %d\n",
					    tp->id);
			}
		}
	}
}

float round(x)
float x;
{
	return(floor(x + 0.5));
}

float rectify(x)
float x;
{
	while (x < 0.0) {
		x += 360.0;
	}
	while (x >= 360.0){
		x -= 360.0;
	}
	return x;
}

/*
 * TREK73: firing.c
 *
 * Take care of firing phasers and torpedos for both enemy
 * and fed ships.
 *
 * phaser_firing, torpedo_firing, ship_detonate, torp_detonate
 *
 */

#include "defines.h"
#include "structs.h"

extern struct ship *shiplist[];
extern char shutup[];

phaser_firing(sp)
struct ship *sp;
{
	extern	struct list head;
	extern	struct list *tail;
	extern	struct damage p_damage;
	register int i;
	register int j;
	int	hit;
	struct	ship *ep;
	struct	torpedo *tp;
	int	s;
	int	x, y;
	struct	ship *target;
	struct	list *lp;
	int	bear;
	struct 	ship *fed;


	fed = shiplist[0];
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_FIRING)
			break;
	}
	if (i == 4)
		return 0;
	sp->phasers[i].status &= ~P_FIRING;
	target = sp->phasers[i].target;
	/*
	 * Put in j the exact bearing of the phasers relative to
	 * the ship
	 */
	if (target == NULL) {
		bear = sp->phasers[i].bearing + sp->course;
		j = sp->phasers[i].bearing;
	} else {
		bear = bearing(sp->x, target->x, sp->y, target->y);
		j = bear - sp->course;
	}
	j = rectify(j);
	if (j > 125 && j < 235 && !(sp->status & S_ENG))
		return 0;
	if (target != NULL && (target->status & S_DEAD)) {
		if ((sp = fed) && (!shutup[PHASERS+j])&& !(sp->status & S_DEAD))
			printf("%s phaser %d disengaging\n", sp->name, i+1);
		sp->phasers[i].target = NULL;
		shutup[PHASERS+j]++;
		return 0;
	}
	if (target != NULL)
		sp->phasers[i].bearing = j;
	printf(" <%s frng phasers>\n", sp->name);
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type == 0)
			continue;
		ep = NULL;
		tp = NULL;
		if (lp->type == I_SHIP) {
			ep = lp->data.sp;
			if (ep == sp)
				continue;
			x = ep->x;
			y = ep->y;
		} else {
			tp = lp->data.tp;
			x = tp->x;
			y = tp->y;
		}
		hit = phaser_hit(sp, x, y, &sp->phasers[i], bear);
		if (hit == 0)
			continue;
		if (tp) {
			if (tp->timedelay > 2) {
				switch (lp->type) {
				default:
				case I_SHIP:
					printf("oops...\n");
					break;
				case I_TORPEDO:
					printf("hit on torpedo %d\n",
						tp->id);
					break;
				case I_ENG:
					printf("%s's engineering hit.\n",
						tp->from->name);
					break;
				case I_PROBE:
					printf("hit on probe %d\n", 
						tp->id);
					break;
				}
				tp->timedelay = 2;
			}
			tp->fuel -= hit/2;
			if (tp->fuel < 0)
				tp->fuel = 0;
			continue;
		}
		/*
		 * Determine which shield was hit
		 */
		j = rectify(bearing(x, sp->x, y, sp->y) - ep->course);
		if (j > 315 || j < 45)
			s = 1;
		else if (j < 135)
			s = 2;
		else if (j < 225)
			s = 3;
		else
			s = 4;
		damage(hit, ep, s, &p_damage);
	}
	/*
	 * Reduce the load by the firing percentage
	 */
	sp->phasers[i].load = sp->phasers[i].load
	    - sp->phasers[i].load * sp->p_percent / 100;
	return 0;
}

torpedo_firing(sp)
struct ship *sp;
{
	extern	struct damage a_damage;
	extern	struct list *newitem();
	register int i;
	register int j;
	register int th;
	struct	torpedo *tp;
	struct	ship *target;
	struct	list *lp;
	int	bear;
	struct	ship *fed;


	fed = shiplist[0];
	for (i=0; i<6; i++) {
		if (sp->tubes[i].status & T_FIRING)
			break;
	}
	if (i == 6)
		return 0;
	sp->tubes[i].status &= ~T_FIRING;
	th = sp->tubes[i].load;
	if (th == 0)
		return 0;
	target = sp->tubes[i].target;
	/*
	 * Put in j the relative bearing of the tube
	 */
	if (target == NULL) {
		bear = sp->tubes[i].bearing + sp->course;
		j = sp->tubes[i].bearing;
	} else {
		bear = bearing(sp->x, target->x, sp->y, target->y);
		j = bear - sp->course;
	}
	j = rectify(j);
	if (j > 125 && j < 235 && !(sp->status & S_ENG))
		return 0;
	if (target != NULL && (target->status & S_DEAD)) {
		if ((sp = fed) && (!shutup[TUBES+j])&&!(sp->status & S_DEAD))
			printf("   tube %d disengaging\n", i+1);
		sp->tubes[i].target = NULL;
		shutup[TUBES+j]++;
		return 0;
	}
	if (target != NULL)
		sp->tubes[i].bearing = j;
	sp->tubes[i].load = 0;
	lp = newitem(I_TORPEDO);
	lp->type = I_TORPEDO;
	lp->data.tp = MKNODE(struct torpedo, *, 1);
	tp = lp->data.tp;
	tp->from = sp;
	tp->x = sp->x;
	tp->y = sp->y;
	tp->target = NULL;
	tp->course = rectify(bear);
	tp->fuel = th;
	tp->speed = sp->t_lspeed + sp->warp;
	tp->newspeed = tp->speed;
	tp->timedelay = sp->t_delay * 10;
	tp->prox = sp->t_prox;
	tp->id = new_slot();
	printf(" <<%s frng torpedo %d>>\n", sp->name, tp->id);
	return 1;
}

int ship_detonate(sp, lp)
struct ship *sp;
struct list *lp;
{
	register int fuel;
	register int i;

	fuel = 0;
	printf("++%s++ destruct.\n", sp->name);
	for (i=0; i<4; i++)
		if (sp->phasers[i].status & ~P_DAMAGED)
			fuel += min(sp->phasers[i].load, 10);
	for (i=0; i<6; i++)
		if (sp->tubes[i].status & ~T_DAMAGED)
			fuel += min(sp->tubes[i].load, 10);
	fuel += sp->pods;
	antimatter_hit((char *) sp, sp->x, sp->y, fuel);
	lp->type = 0;
	sp->status |= S_DEAD;
}


int torp_detonate(tp, lp)
struct torpedo *tp;
struct list *lp;
{

	switch (lp->type) {
		case I_SHIP:
			printf("we aren't supposed to be here \n");
			break;
		case I_TORPEDO:
			printf(":: torp %d ::\n", tp->id);
			break;
		case I_PROBE:
			printf("** probe %d **\n", tp->id);
			break;
		case I_ENG:
			printf("## %s engineering ##\n", tp->from->name);
			break;
		default:
			printf("what the heck is this\n");
			break;
	}
	antimatter_hit((char *) tp, tp->x, tp->y, tp->fuel);
	return_slot(tp->id);
	delitem(lp);
}

/*
 * TREK73: moveships.c
 *
 * Actual command execution
 *
 * move_ships
 *
 */

#include "defines.h"
#include "structs.h"

extern int defenseless;
extern int corbomite;
extern int surrender;
extern int surrenderp;
extern char shutup[];
extern int global;
extern int reengaged;


move_ships() {
	extern	double sin();
	extern	double cos();
	extern	double sqrt();
	extern	float fabs();
	extern	struct list head;
	extern	struct list *tail;
	extern	int shipnum;
	extern	struct ship *shiplist[];
	register int i;
	register int j;
	register int k;
	register struct list *lp;
	register struct ship *sp;
	struct	torpedo *tp;
	struct	ship *fed;
	struct	list *lp1;
	struct	ship *sp1;
	int	iterations;
	float	tmpf;
	int	course, newcourse, energy;
	float	warp, newwarp;
	int	x, y;
	struct	ship *target;
	int	kills, others, fedstatus;
	struct	ship *ep;
	double	d0;
	extern	float	segment;
	extern	float	timeperturn;
	float	Mpersegment;

	/*
	 * The value 100 is the number of Megameters per second
	 * per warp-factor
	 */
	Mpersegment = segment * 100.0;
	iterations = timeperturn/segment;
	for (i=0; i<=shipnum; i++)
		distribute(shiplist[i]);
	fed = shiplist[0];
	for (i=0; i<iterations; i++) {
		for (lp = &head; lp != NULL; lp = lp->fwd) {
			if (lp == tail)
				break;
			if (lp->type == 0)
				continue;
			sp = NULL;
			tp = NULL;
			if (lp->type == I_SHIP)
				sp = lp->data.sp;
			else
				tp = lp->data.tp;
			if (sp && sp->status & S_DEAD)
				continue;
			if (sp) {
				phaser_firing(sp);
				torpedo_firing(sp);
			}
			/*
			 * time fuses
			 */
			if (tp) {
				tp->timedelay--;
				if (tp->timedelay <= 0) {
					torp_detonate(tp, lp);
					continue;
				}
			} else {
				sp->delay--;
				if (sp->delay <= 0) {
					ship_detonate(sp, lp);
					continue;
				}
			}
			/*
			 * proximity fuse
			 */
			if (tp && tp->prox != 0) {
				for (lp1 = &head; lp1 != tail; lp1 = lp1->fwd) {
					if (lp1->type != I_SHIP)
						continue;
					sp1 = lp1->data.sp;
					if (sp1 == tp->from)
						continue;
					j=rangefind(tp->x,sp1->x,tp->y,sp1->y);
					if (j > tp->prox)
						continue;
					torp_detonate(tp, lp);
					tp = 0;
					break;
				}
				if (!tp)
					continue;
			}
			/*
			 * movement simulation
			 */
			if (sp && sp->status & S_DEAD)
				continue;
			if (sp) {
				x = sp->x;
				y = sp->y;
				warp = sp->warp;
				newwarp = sp->newwarp;
				course = sp->course;
				newcourse = sp->newcourse;
				target = sp->target;
				energy = sp->energy;
				/*
				 * fuel consumption
				 */
				if (fabs((double) warp) >= 1)
					j = (int) (warp * sp->eff * segment);
				else
					j = 0;
				if (j < 0)
					j *= -1;
				if (j > energy) {
					if (!shutup[BURNOUT + sp->id]  && !(sp->status & S_WARP)) {
						printf("%s's warp drive burning out.\n",
							sp->name);
						shutup[BURNOUT + sp->id]++;
					}
					newwarp = warp < 0 ? -.99 : .99;
					energy = 0;
				} else
					energy -= j;
			} else {
				x = tp->x;
				y = tp->y;
				warp = tp->speed;
				newwarp = tp->newspeed;
				course = tp->course;
				newcourse = course;
				target = tp->target;
			}
			/*
			 * destroyed warp drive
			 */
			if (sp && (sp->status & S_WARP))
				if (fabs((double)warp) > 1.0)
					newwarp = warp < 0.0 ? -.99 : .99;
			/*
			 * automatic pilot
			 */
			if (target != NULL) {
				if (sp && (target->status & S_DEAD)) {
					if ((sp == fed) && (!shutup[DISENGAGE]) && !(sp->status & S_DEAD))
						printf("%s's autopilot disengaging.\n",
						sp->name);
					newcourse = course;
					shutup[DISENGAGE]++;
					target = NULL;
					sp->eluding = 0;
				} else {
					j = bearing(x, target->x, y, target->y);
					if (sp && sp->eluding)
						j = rectify(j + 180);
					newcourse = (float) j;
					/*if ((sp) && (sp != fed)) {
						sp->newwarp = 1.0;
						newwarp = 1.0;
					}*/
					if (tp)
						course = newcourse;
				}
			}
			/*
			 * turn rate
			 */
			if (course != newcourse) {
				j = rectify(newcourse - course);
				if (j > 180)
					j -= 360;
				/*
				 * maximum degrees turned in one turn
				 */
				k = (int) ((12.0 - warp) * 4.0 * segment);
				if (sp->status & S_WARP)
					k /= 2;
				k = (int) course + (j < 0 ? -1 : 1) *
					min(abs(j), k);
				course = (float) rectify(k);
			}
			/*
			 * acceleration
			 */
			tmpf = newwarp - warp;
			if (tmpf < 0.0)
				d0 = (double) (tmpf * -1.0);
			else
				d0 = (double) tmpf;
			if (tmpf != 0.0)
				warp += (tmpf < 0 ? -1 : 1) * sqrt(d0)
				    * segment;
			d0 = (double) toradians(course);
			x += (int) (warp * cos(d0) * Mpersegment);
			y += (int) (warp * sin(d0) * Mpersegment);
			if ((warp > -0.1) && (warp < 0.1)){
				warp = 0.0;
			}
			/*
			 * reset all the vars
			 */
			if (sp) {
				sp->x = x;
				sp->y = y;
				sp->warp = warp;
				sp->newwarp = newwarp;
				sp->course = course % 360;
				sp->newcourse = newcourse % 360;
				sp->energy = energy;
				sp->target = target;
			} else {
				tp->x = x;
				tp->y = y;
				tp->speed = warp;
				tp->course = course % 360;
				tp->target = target;
			}
		}
	}
	for (i=0; i <= shipnum; i++) {		/* Check targets */
		sp = shiplist[i];
		if (sp->status & S_DEAD)
			continue;
		for (j=0; j<4; j++) {
			target = sp->phasers[j].target;
			if (target && (target->status & S_DEAD)) {
				if ((sp == fed) && (!shutup[PHASERS+j])&& !(sp->status & S_DEAD))
					printf("  phaser %d disengaging\n",j+1);
				sp->phasers[j].target = NULL;
				shutup[PHASERS+j]++;
			}
		}
		for (j=0; j<6; j++) {
			target = sp->tubes[j].target;
			if (target && (target->status & S_DEAD)) {
				if ((sp == fed) && (!shutup[TUBES+j]) && !(sp->status & S_DEAD))
					printf("  tube %d disengaging\n", j+1);
				sp->tubes[j].target = NULL;
				shutup[TUBES+j]++;
			}
		}
	}
	/*
	 * self-destruct warning
	 */
	if ((fed->delay < 1000) && (fed->delay > 8)) {
		printf("Computer:   %d seconds to self destruct.\n",fed->delay/9);
	} 
	/*
	 * Ruses, bluffs, surrenders
	 */
	if (defenseless)
		defenseless++;
	if (corbomite)
		corbomite++;
	if (surrender)
		surrender++;
	if (surrenderp)
		surrenderp++;
	/*
	 * Federation dispostion
	 */
	sp = fed;
	kills = others = fedstatus = 0;
	if ((sp->crew <= 0) || (sp->status & S_DEAD)) {
		fedstatus = 1;
		sp->status |= S_DEAD;
	} else
		for (j=0; j<1; j++) {
			for (i=0; i<4; i++)
				if (~sp->phasers[i].status | ~P_DAMAGED)
					break;
			if (i != 4)
				continue;
			for (i=0; i<6; i++)
				if (~sp->tubes[i].status | ~T_DAMAGED)
					break;
			if (i != 6)
				continue;
			fedstatus = 1;
		}
	/*
	 * enemy disposition
	 */
	for (k=1; k <= shipnum; k++) {
		ep = shiplist[k];
		if (ep->status & S_DEAD) {
			kills++;
			continue;
		}
		if (ep->crew <= 0) {
			ep->status |= S_DEAD;
			kills++;
			continue;
		}
		if (fedstatus)
			continue;
		j = rangefind(sp->x, ep->x, sp->y, ep->y);
		if ((j>3500 && (ep->status & S_WARP)) ||
			(j>4500 && ep->delay<100)) {
				others++;
				continue;
		}
		/* Check if we are within range to turn off the flag */
		if ((j <= 3500) && (reengaged))
			reengaged = 0;
		if (ep->energy > 10)
			continue;
		for (i=0; i<4; i++)
			if (~ep->phasers[i].status | ~P_DAMAGED)
				break;
		if (i != 4)
			continue;
		for (i=0; i<6; i++)
			if (~ep->tubes[i].status | ~T_DAMAGED)
				break;
		if (i != 6)
			continue;
		kills++;
	}
	if (fedstatus == 0 && (global & E_SURRENDER)) {
		if (leftovers())
			warn(4);
		else {
			final(4);
		}
	}
	if ((fed->status & S_SURRENDER) && (kills + others < shipnum)) {
		if (leftovers())
			warn(3);
		else {
			final(3);
		}
	}
	if (fedstatus == 0 && kills+others < shipnum)
		return 0;
	if (fedstatus == 1 && kills+others < shipnum) {
		if (leftovers())
			warn(0);
		else
			final(0);
	}
	if (fedstatus == 0 && kills == shipnum) {
		if (leftovers())
			warn(1);
		else
			final(1);
	}
	if (fedstatus == 0 && kills+others == shipnum) {
		if (leftovers())
			warn(2);
		else
			final(2);
	}
	if (fedstatus == 1 && kills == shipnum) {
		final(5);
	}
	return 0;
}

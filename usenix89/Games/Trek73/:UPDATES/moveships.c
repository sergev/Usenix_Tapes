/*
 * TREK73: moveships.c
 *
 * Actual command execution
 *
 * move_ships, check_targets, misc_timers, disposition
 *
 */

#include "externs.h"


move_ships() 
{
	register int i;
	register float j;
	register float k;
	register struct list *lp;
	register struct ship *sp;
	register float fuel_use;
	struct	torpedo *tp;
	struct	ship *fed;
	struct	list *lp1;
	struct	ship *sp1;
	float	iterations;
	float	tmpf;
	float	energy;
	float	course, newcourse;
	float	warp, newwarp;
	int	x, y;
	struct	ship *target;
	double	d0;
	float	Mpersegment;
	double	floor(), fabs();

	/*
	 * The value 100 is the number of Megameters per second
	 * per warp-factor
	 */
	Mpersegment = segment * 100.0;
	iterations = (int)floor(timeperturn/segment + 0.5); /* What a crock! */
	for (i=0; i<=shipnum; i++)
		distribute(shiplist[i]);
	fed = shiplist[0];
	for (i=0; i<iterations; i++) {
		for (lp = &head; lp != NULL; lp = lp->fwd) {
			if (lp == tail)
				break;
			if (lp->type == I_UNDEFINED)
				continue;
			sp = NULL;
			tp = NULL;
			if (lp->type == I_SHIP)
				sp = lp->data.sp;
			else
				tp = lp->data.tp;
			if (sp && is_dead(sp, S_DEAD))
				continue;
			if ((sp) && (i % sp->p_firing_delay == 0))
				(void) phaser_firing(sp);
			if ((sp) && (i % sp->t_firing_delay == 0))
				(void) torpedo_firing(sp);
			/*
			 * time fuses
			 */
			if (tp) {
				tp->timedelay -= segment;
				if (tp->timedelay <= 0.) {
					torp_detonate(tp, lp);
					continue;
				}
			} else {
				sp->delay -= segment;
				if (sp->delay <= 0.) {
					ship_detonate(sp, lp);
					continue;
				}
			}
			/*
			 * proximity fuse
			 */
			if (tp && tp->prox != 0) {
				for (lp1 = &head; lp1 != tail; lp1 = lp1->fwd) {
					if (lp1->type == I_SHIP) {
						sp1 = lp1->data.sp;
						if (sp1 == tp->from)
							continue;
						if (cantsee(sp1))
							continue;
						j = rangefind(tp->x, sp1->x,
						    tp->y, sp1->y);
					} else if (lp1->type == I_ENG) {
						sp1 = lp1->data.sp;
						if (sp1 == tp->from)
							continue;
						if (cantsee(sp1))
							continue;
						j = rangefind(tp->x, sp1->x,
						    tp->y, sp1->y);
					} else
						continue;
					if (j >= tp->prox)
						continue;
					torp_detonate(tp, lp);
					break;
				}
				if (lp1 != tail)
					continue;
			}
			/*
			 * movement simulation
			 */
			if (sp && is_dead(sp, S_DEAD))
				continue;
			if (sp) {
				x = sp->x;
				y = sp->y;
				warp = sp->warp;
				if (fabs(sp->newwarp) > sp->max_speed)
					sp->newwarp = (sp->newwarp > 0)
					    ? sp->max_speed : -sp->max_speed;
				newwarp = sp->newwarp;
				course = sp->course;
				newcourse = sp->newcourse;
				target = sp->target;
				energy = sp->energy;
				/*
				 * fuel consumption
				 */
				if (fabs(warp) > 1.0)
					fuel_use = (fabs(warp) * sp->eff * segment);
				else
					fuel_use = 0.;
#ifdef TRACE
				/*
				if (trace)
					printf("*** Fuel use = %.2f, energy = %.2f\n",
					    fuel_use, energy);
				*/
#endif
				if (fuel_use > energy) {
					if (!shutup[BURNOUT + sp->id] &&
					    !(is_dead(sp, S_WARP)) && cansee(sp)) {
						printf("%s's warp drive burning out.\n",
							sp->name);
						shutup[BURNOUT + sp->id]++;
					}
					newwarp = (warp < 0.0) ? -0.99 : 0.99;
					energy = 0;
				} else
					energy -= fuel_use;
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
			if (sp && (is_dead(sp, S_WARP)))
				if (fabs(warp) > 1.0)
					newwarp = (warp < 0.0) ? -0.99 : 0.99;
			/*
			 * automatic pilot
			 */
			if (target != NULL)
				if (sp && (is_dead(target, S_DEAD))) {
					if ((sp == fed) && !shutup[DISENGAGE]
					    && !is_dead(sp, S_DEAD)) {
						printf("%s's autopilot disengaging.\n",
						    sp->name);
						shutup[DISENGAGE]++;
					}
					newcourse = course;
					target = NULL;
					sp->relbear = 0.0;
				} else {
					if (cantsee(target))
						j =bearing(x,target->position.x,
						    y, target->position.y);
					else
						j = bearing(x, target->x, y, target->y);
					if (sp)
						j = rectify(j - sp->relbear);
					newcourse = j;
				}
			/*
			 * turn rate (known to be a ship)
			 */
			if (tp)
				course = newcourse;
			if (sp && course != newcourse) {
				j = rectify(newcourse - course);
				if (j > 180)
					j -= 360;
				/*
				 * maximum degrees turned in one turn
				 */
				k = ((sp->max_speed + 2.0 - warp)
				    * sp->deg_turn * segment);
				/* If you have no warp drive, you're less
				 * maneuverable
				 */
				if (is_dead(sp, S_WARP))
					k /= 2;
				k = course + (j < 0.0 ? -1.0 : 1.0) *
					min(fabs(j), k);
				course = rectify(k);
			}
			/*
			 * acceleration
			 */
			tmpf = newwarp - warp;
			d0 = fabs(tmpf);
			warp += ((tmpf < 0.0) ? -1 : 1) * sqrt(d0) * segment;
			d0 = toradians(course);
			x += (int) (warp * cos(d0) * Mpersegment);
			y += (int) (warp * sin(d0) * Mpersegment);
			/*
			 * projected position (cloaked)
			 */
			if (sp && cantsee(sp)) {
				d0 = toradians(sp->position.course);
				sp->position.x += (sp->position.warp * cos(d0)
				    * segment);
				sp->position.y += (sp->position.warp * sin(d0)
				    * segment);
			}
			/*
			 * reset all the vars
			 */
			if (sp) {
				sp->x = x;
				sp->y = y;
				sp->warp = warp;
				sp->newwarp = newwarp;
				/* XXXXX should these be rectified? */
				sp->course = rectify(course);
				sp->newcourse = rectify(newcourse);
				sp->energy = energy;
				sp->target = target;
			} else {
				tp->x = x;
				tp->y = y;
				tp->speed = warp;
				tp->course = rectify(course);
				tp->target = target;
			}
		}
	}
}


check_targets()
{
	register int i;
	register int j;
	struct ship *sp;
	struct ship *fed;
	struct ship *target;

	fed = shiplist[0];
	for (i=0; i <= shipnum; i++) {		/* Check targets */
		sp = shiplist[i];
		if (is_dead(sp, S_DEAD))
			continue;
		target = sp->target;
		if (target && is_dead(target, S_DEAD)) {
			if ((sp == fed) && !shutup[DISENGAGE]) {
				puts("   helm lock disengaging");
				shutup[DISENGAGE]++;
			}
			sp->target = NULL;
			sp->relbear = 0.0;
		}
		for (j=0; j<sp->num_phasers; j++) {
			target = sp->phasers[j].target;
			if (target && is_dead(target, S_DEAD)) {
				if ((sp == fed) && (!shutup[PHASERS+j])
				    && !(is_dead(sp, S_DEAD))) {
					printf("  phaser %d disengaging\n",j+1);
					shutup[PHASERS+j]++;
				}
				sp->phasers[j].target = NULL;
			} else if (target)
				sp->phasers[j].bearing = rectify(bearing(sp->x,
				    (cantsee(target)) ? target->position.x
						     : target->x, sp->y,
				    (cantsee(target)) ? target->position.y
						     : target->y) - sp->course);
		}
		for (j=0; j<sp->num_tubes; j++) {
			target = sp->tubes[j].target;
			if (target && is_dead(target, S_DEAD)) {
				if ((sp == fed) && (!shutup[TUBES+j])
				    && !(is_dead(sp, S_DEAD))) {
					printf("  tube %d disengaging\n", j+1);
					shutup[TUBES+j]++;
				}
				sp->tubes[j].target = NULL;
			} else if (target)
				sp->tubes[j].bearing = rectify(bearing(sp->x,
				    (cantsee(target)) ? target->position.x
						     : target->x, sp->y,
				    (cantsee(target)) ? target->position.y
						     : target->y) - sp->course);
		}
	}
	if (sp->cloak_delay > 0)
		sp->cloak_delay--;
}

misc_timers()
{
	struct ship *fed;

	/*
	 * self-destruct warning
	 */
	fed = shiplist[0];
	if ((fed->delay < 1000.) && (fed->delay > 0.)) {
		if (is_dead(fed, S_COMP)) {
			printf("%s:  Self-destruct has been aborted due to computer damage\n",
			    science);
			fed->delay = 10000.;
		} else
			printf("Computer:   %5.2f seconds to self destruct.\n",
			    fed->delay);
	} 

	fed->probe_status = PR_NORMAL;

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
}

disposition()
{
	struct ship *sp;
	struct ship *fed;
	struct ship *ep;
	int kills;
	int others;
	int fedstatus;
	register int i;
	register int j;
	register int k;

	/*
	 * Federation dispostion
	 */
	fed = shiplist[0];
	sp = fed;
	kills = others = fedstatus = 0;
	if (sp->complement <= 0 && !is_dead(sp, S_DEAD)) {
		sp->status[S_DEAD] = 100;
		sp->newwarp = 0.0;
		sp->newcourse = sp->course;
		sp->target = NULL;
		sp->relbear = 0.0;
		for (i = 0; i < sp->num_phasers; i++) {
			sp->phasers[i].target = NULL;
			sp->phasers[i].drain = MIN_PHASER_DRAIN;
			sp->phasers[i].status &= ~P_FIRING;
		}
		for (i = 0; i < sp->num_tubes; i++) {
			sp->tubes[i].target = NULL;
			sp->tubes[i].status &= ~T_FIRING;
		}
		for (i = 0; i < SHIELDS; i++)
			sp->shields[i].attemp_drain = 0.0;
		sp->regen = 0.0;
		sp->cloaking = C_NONE;
	}
	if (is_dead(sp, S_DEAD))
		fedstatus = 1;
	else {
		for (i=0; i<sp->num_phasers; i++)
			if (!(sp->phasers[i].status & P_DAMAGED))
				break;
		if (i == sp->num_phasers) {
			for (i=0; i<sp->num_tubes; i++)
				if (!(sp->tubes[i].status & T_DAMAGED))
					break;
			if (i == sp->num_tubes)
				fedstatus = 1;
		}
	}
	/*
	 * enemy disposition
	 */
	for (k=1; k <= shipnum; k++) {
		ep = shiplist[k];
		if (ep->complement <= 0 && !is_dead(ep, S_DEAD)) {
			ep->status[S_DEAD] = 100;
			ep->newwarp = 0.0;
			ep->newcourse = ep->course;
			ep->target = NULL;
			ep->relbear = 0.0;
			for (i = 0; i < ep->num_phasers; i++) {
				ep->phasers[i].target = NULL;
				ep->phasers[i].drain = MIN_PHASER_DRAIN;
				ep->phasers[i].status &= ~P_FIRING;
			}
			for (i = 0; i < ep->num_tubes; i++) {
				ep->tubes[i].target = NULL;
				ep->tubes[i].status &= ~T_FIRING;
			}
			for (i = 0; i < SHIELDS; i++)
				ep->shields[i].attemp_drain = 0.0;
			ep->regen = 0.0;
			ep->cloaking = C_NONE;
		}
		if (is_dead(ep, S_DEAD)) {
			kills++;
			continue;
		}
		if (fedstatus)
			continue;
		j = rangefind(sp->x, ep->x, sp->y, ep->y);
		if ((j>3500 && is_dead(ep, S_WARP)) ||
			(j>4500 && ep->delay<10.)) {
				others++;
				continue;
		}
		/* Check if we are within range to turn off the flag */
		if ((j <= 3500) && (reengaged))
			reengaged = 0;
		if (ep->energy > 10)
			continue;
		for (i=0; i<ep->num_phasers; i++)
			if (!(ep->phasers[i].status & P_DAMAGED))
				break;
		if (i != ep->num_phasers)
			continue;
		for (i=0; i<ep->num_tubes; i++)
			if (!(ep->tubes[i].status & T_DAMAGED))
				break;
		if (i != ep->num_tubes)
			continue;
		kills++;
	}
	if (!fedstatus && (global & E_SURRENDER))	/* enemy surrender */
		if (leftovers())
			(void) warn(FIN_E_SURRENDER);
		else
			(void) final(FIN_E_SURRENDER);
	if ((is_dead(fed, S_SURRENDER)) && (kills + others < shipnum))
		if (leftovers())		/* federation surrender */
			(void) warn(FIN_F_SURRENDER);
		else
			(void) final(FIN_F_SURRENDER);
	if (!fedstatus && (kills + others) < shipnum)	/* play continues */
		return 0;
	if (fedstatus && kills < shipnum)		/* Fed. defeated */
		if (leftovers())
			(void) warn(FIN_F_LOSE);
		else
			(void) final(FIN_F_LOSE);
	if (!fedstatus && kills == shipnum)		/* Fed. victory */
		if (leftovers())
			(void) warn(FIN_E_LOSE);
		else
			(void) final(FIN_E_LOSE);
	if (!fedstatus && (kills + others) == shipnum)	/* outmaneuvered */
		if (leftovers())
			(void) warn(FIN_TACTICAL);
		else
			(void) final(FIN_TACTICAL);
	if (fedstatus && kills == shipnum)
		(void) final(FIN_COMPLETE);		/* both sides dead */
	return 0;
}

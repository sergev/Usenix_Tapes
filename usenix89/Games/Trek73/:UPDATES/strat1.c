/*
 * TREK73: strat1.c
 *
 * Standard Enemy Strategy
 *
 * standard_strategy
 *
 */

#include "externs.h"


standard_strategy(sp)
struct ship *sp;
{
	register struct ship *fed;
	float	bear;
	int	range;
	float	tmpf;

	fed = shiplist[0];
	if (is_dead(sp, S_DEAD))
		return;
	range = rangefind(sp->x, fed->x, sp->y, fed->y);
	bear = bearing(sp->x, fed->x, sp->y, fed->y);
	bear = rectify(bear - sp->course);
	/*
	 * Handle special requests
	 */
	special(sp, range, fed);
	/*
	 * Now check for surrendering flags
	 */
	if ((global & F_SURRENDER) || (global & E_SURRENDER))
		return;

	/*
	 * Always turn on cloaking device if we have it and if we
	 * can afford it
	 */
	if ((sp->cloaking == C_OFF) && (sp->energy >= 20)
	    && e_cloak_on(sp, fed))
		return;

	/*
	 * Check for hostile antimatter devices
	 */
	if ((sp->cloaking == C_OFF) && e_closetorps(sp, fed))
		return;
	if ((sp->cloaking == C_OFF) && e_checkprobe(sp))
		return;

	/*
	 * If cloaking is on, and we're running low on energy,
	 * drop the cloak
	 */
	/* XXXX May want to change number */
	if ((sp->cloaking == C_ON) && (sp->energy < 30)
	    && e_cloak_off(sp, fed))
		return;

	/*
	 * Short range? 
	 */
	if (range < 1050) {
		if (e_checkarms(sp) < randm((int)(sp->num_phasers+
		    sp->num_tubes)/3)) {
			if (!e_cloak_on(sp, fed))
				(void) e_runaway(sp, fed);
			return;
		}
		if (e_lockphasers(sp, fed))
			return;
		if (e_phasers(sp, fed))
			return;
		if (betw(bear, 90.0, 270.0)) {
			(void) e_pursue(sp, fed, 1.0);
			return;
		}
		if (e_launchprobe(sp, fed))
			return;
		if (sp->pods< 20 && sp->regen < 4.0 && e_destruct(sp, fed))
			return;
		/*
		 * set course?
		 */
		tmpf = fabs(fed->warp);
		if (sp->target != fed || fabs(sp->warp) + tmpf > 2.0) {
			(void) e_pursue(sp, fed, tmpf);
			return;
		}
		if (e_cloak_on(sp, fed))
			return;
	}
	if (range < 3800) {
		/*
		 * Either medium range, or we can't figure out what
		 * to do at short range
		 */
		if (e_locktubes(sp, fed))
			return;
		if (sp->energy > 30 && sp->pods > 40 && e_loadtubes(sp))
			return;
		if (e_torpedo(sp))
			return;
		/*
		 * should we run away; can we?
		 */
		if (e_checkarms(sp) < randm((int)(sp->num_phasers+
		    sp->num_tubes)/3)) {
			if (!e_cloak_on(sp, fed))
				(void) e_runaway(sp, fed);
			return;
		}
		/*
		 * Pursued from behind, low power: jettison engineering!
		 */
		if (betw(bear, 90.0, 270.0) && sp->energy < 10
		    && sp->regen < 4.0 && e_jettison(sp, fed))
			return;
		/*
		 * put in other junk later
		 */
		if (e_cloak_on(sp, fed))
			return;
	}
	/*
	 * Either distant range, or we can't figure out
	 * what to do at medium range
	 */
	/* Warp drive dead and Federation destructing, run away! */
	if (fed->delay < 15. && (is_dead(sp, S_WARP))) {
		(void) e_runaway(sp, fed);
		return;
	}
	/*
	 * enemy in our blind area?  make a quick turn.
	 */
	/* XXXX  Should we check for blind spots?
	 * or should we check for forward/aft
	 */
	if (betw(bear, sp->t_blind_left, sp->t_blind_right)) {
		(void) e_pursue(sp, fed, 1.0);
		return;
	}
	if (e_locktubes(sp, fed))
		return;
	if (e_lockphasers(sp, fed))
		return;
	/*
	 * attack?
	 */
	/*
	tmpf = fabs(fed->warp);
	if (sp->target != fed || sp->warp + tmpf > 2.0) {
		(void) e_pursue(sp, fed, (int)tmpf + 2.0 + randm(2));
		return;
	}
	*/
	if (e_attack(sp, fed))
		return;
	if (sp->energy > 30 && sp->pods > 40 && e_loadtubes(sp))
		return;
	if (e_cloak_on(sp, fed))
		return;
	/*
	 * gee, there's nothing that we want to do!
	 */
	if (cansee(sp))
		printf("%s:  We're being scanned by the %s\n",
		    science, sp->name);
}

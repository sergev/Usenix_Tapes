/*
 * TREK73: enemycom.c
 *
 * Enemy strategy sub-routines
 *
 * e_attack, e_checkarms, e_checkprobe, e_cloak_off, e_cloak_on,
 * e_closetorps, e_destruct, e_evade, e_jettison, e_launchprobe,
 * e_loadtubes, e_lockphasers, e_locktubes, e_phasers, e_pursue,
 * e_runaway, e_torpedo
 *
 */

#include "externs.h"


/*
 * This routine turns the ship at speed towards its target
 */
int e_attack(sp, fed)
struct ship *sp;
struct ship *fed;
{
	float	speed;
	float	tmpf;

	tmpf = fabs(fed->warp);
	if (fabs(sp->warp) >= tmpf + 2.0 || (is_dead(sp, S_WARP)))
		return 0;
	if ((speed = tmpf + randm(2) + 2.0) > sp->max_speed)
		speed = sp->max_speed;
	(void) e_pursue(sp, fed, speed);
	if (cansee(sp) && syswork(fed, S_SENSOR))
		printf("%s:  %s attacking.\n", helmsman, sp->name);
	return 1;
}


/*
 * Returns the number of currently loaded, undamaged weapons
 */
int e_checkarms(sp)
struct ship *sp;
{
	register int i;
	register int arms;

	arms = 0;
	for (i=0; i<sp->num_phasers; i++)
		if (!(sp->phasers[i].status & P_DAMAGED) &&
		    (sp->phasers[i].load >= 0))
			arms++;
	for (i=0; i<sp->num_tubes; i++)
		if (!(sp->tubes[i].status & T_DAMAGED) &&
		    (sp->tubes[i].load >= 0))
			arms++;
#ifdef TRACE
	if (trace)
		printf("*** Checkarms: Returning %d\n", arms);
#endif
	return arms;
}


/*
 * returns 1 if evasive action being taken (to avoid probe)
 */
int e_checkprobe(sp)
struct ship *sp;
{
	register struct list *lp;
	register int range;
	register struct torpedo *tp;

	/*
	 * If we are cloaked, do not bother with this.
	 * Since probes cannot detect us when cloaked.
	 */
	if (cantsee(sp))
		return 0;
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_PROBE)
			continue;
		tp = lp->data.tp;
		range = rangefind(sp->x, tp->x, sp->y, tp->y);
#ifdef TRACE
		if (trace)
			printf("*** Checkprobe: Range = %d\n", range);
#endif
		if (range < 2000) {
			(void) e_evade(sp, tp->x, tp->y, I_PROBE);
			return 1;
		}
	}
	return 0;
}


/*
 * Returns 1 if cloaking device was turned off
 */
int e_cloak_off(sp, fed)
struct ship *sp;
struct ship *fed;
{
	if (sp->cloaking != C_ON)
		return 0;
	sp->cloaking = C_OFF;
	sp->cloak_delay = 4;
	if (syswork(sp, S_SENSOR)) {
		printf("%s:  The %s has reappeared on our sensors %d\n",
		    science, sp->name, rangefind(sp->x, sp->position.x,
		    sp->y, sp->position.y));
		puts("   megameters from its projected position.");
	}
	fed = fed;		/* LINT */
	return 1;
}


/*
 * Returns 1 if cloaking device was turned on
 */
int e_cloak_on(sp, fed)
struct ship *sp;
struct ship *fed;
{
	if ((sp->cloak_delay > 0) || (sp->cloaking != C_OFF))
		return 0;
	sp->cloaking = C_ON;
	sp->position.x = sp->x;
	sp->position.y = sp->y;
	sp->position.warp = sp->warp;
	sp->position.course = sp->course;
	if (syswork(fed, S_SENSOR))
		printf("%s:  The %s has disappeared from our sensors.\n",
		    science, sp->name);
	fed = fed;		/* LINT */
	return 1;
}


/*
 * Returns 1 if firing phasers at or evading nearby torpedoes
 */
int e_closetorps(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register struct list *lp;
	register int range;
	register struct torpedo *tp;
	struct	torpedo *bad;

	/*
	 * If we are cloaked, forget about this.
	 * Since prox fuses won't affect us under cloak
	 */
	if (cantsee(sp))
		return 0;
	bad = NULL;
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_TORPEDO)
			continue;
		tp = lp->data.tp;
		if (tp->from != fed)
			continue;
		range = rangefind(sp->x, tp->x, sp->y, tp->y);
#ifdef TRACE
		if (trace)
			printf("*** Checktorp: Range = %d\n", range);
#endif
		if (range < 1200) {
			bad = tp;
			/*
			 * fire phasers - hope they're pointing in
			 * the right direction!
			 */
			if (e_phasers(sp, (struct ship *) NULL))
				return 1;
		}
	}
	/*
	 * we can't get a phaser shot off.
	 * try and evade (although hopeless)
	 */
	if (bad != NULL) {
#ifdef TRACE
		if (trace)
			printf("*** Checktorp: Cannot fire phasers!  Evade!\n");
#endif
		(void) e_evade(sp, tp->x, tp->y, I_TORPEDO);
		return 1;
	}
	return 0;
}


/*
 * goodbye, cruel world (Returns 1 if self-destruct was initiated)
 */
int e_destruct(sp, fed)
struct ship *sp, *fed;
{
	if (sp->delay < 5.0)
		return 0;
	sp->delay = 5.0;
	(void) e_cloak_off(sp, fed);
	sp->cloaking = C_NONE;
	if (syswork(fed, S_SENSOR)) {
		printf("%s: The %s is overloading what remains of it's\n",
		    science, sp->name);
		puts("   antimatter pods -- obviously a suicidal gesture.");
		puts("   Estimate detonation in five seconds.");
	}
	return 1;
}


/*
 * Advance to the rear! (Always returns 1)
 */
int e_evade(sp, x, y, type)
struct ship *sp;
int x;
int y;
int type;		/* Currently unused */
{
	register float newcourse;
	float	bear;

	bear = bearing(sp->x, x, sp->y, y);
	if (cansee(sp) && syswork(shiplist[0], S_SENSOR))
		printf("%s taking evasive action!\n", sp->name);
	switch (randm(3)) {
		case 1:
			newcourse = rectify(bear - 90.0);
			break;
		case 2:
			newcourse = rectify(bear + 90.0);
			break;
		case 3:
			newcourse = rectify(bear + 180.0);
			break;
		default:
			printf("error in evade()\n");
			break;
	}
	sp->target = NULL;
	sp->newcourse = newcourse;
	sp->newwarp = 2 + randm((int)(sp->max_speed - 3));
	if (is_dead(sp, S_WARP))
		sp->newwarp = 1.0;
#ifdef TRACE
	if (trace) {
		printf("*** Evade: Newcourse = %3.0f\n", newcourse);
		printf("*** Evade: Newwarp = %.2f\n", sp->newwarp);
	}
#endif
	type = type;	/* LINT */
	return 1;
}


/*
 * Returns 1 if engineering was jettisoned
 */
int e_jettison(sp, fed)
struct ship *sp, *fed;
{
	register struct list *lp;
	register struct torpedo *tp;

	if (is_dead(sp, S_ENG))
		return 0;
	(void) e_cloak_off(sp, fed);
	if (syswork(shiplist[0], S_SENSOR)) {
		printf("%s: Sensors indicate debris being left by\n", science);
		printf("   the %s.  Insufficient mass . . .\n", sp->name);
	}
	lp = newitem(I_ENG);
	tp = lp->data.tp = MKNODE(struct torpedo, *, 1);
	if (tp == (struct torpedo *)NULL) {
		fprintf(stderr, "e_jettison: malloc failed\n");
		exit(2);
	}
	tp->id = new_slot();
	/*
	 * ship slows to warp 1.0 when jettisonning engineering
	 */
	tp->newspeed = 0.0;
	tp->speed = sp->warp;
	tp->target = NULL;
	tp->course = sp->course;
	tp->x = sp->x;
	tp->y = sp->y;
	tp->prox = 0;
	tp->timedelay = 10.;
	tp->fuel = sp->energy;
	tp->type = TP_ENGINEERING;
	sp->energy = sp->pods = 0;
	sp->regen = 0.0;
	tp->from = sp;
	if (sp->newwarp < -1.0)
		sp->newwarp = -0.99;
	if (sp->newwarp > 1.0)
		sp->newwarp = 0.99;
	sp->max_speed = 1.0;
	sp->status[S_ENG] = 100;	/* List as destroyed */
	sp->status[S_WARP] = 100;
	sp->cloaking = C_NONE;
	sp->t_blind_left = sp->t_blind_right = sp->p_blind_left =
	    sp->p_blind_right = 180;
	return 1;
}


/*
 * Returns 1 if a probe was launched
 */
int e_launchprobe(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register struct list *lp;
	register struct torpedo *tp;

	if (!syswork(sp, S_PROBE) || sp->energy <= 10 || cantsee(sp))
		return 0;
	/*
	 * fed ship has to be going slow before we'll launch
	 * a probe at it.
	 */
	if (fabs(fed->warp) > 1.0)
		return 0;
	lp = newitem(I_PROBE);
	tp = lp->data.tp = MKNODE(struct torpedo, *, 1);
	if (tp == (struct torpedo *)NULL) {
		fprintf(stderr, "e_launchprobe: malloc failed\n");
		exit(2);
	}
	tp->from = sp;
	tp->speed = sp->warp;
	tp->newspeed = 3.0;
	tp->target = fed;
	tp->course = bearing(sp->x, fed->x, sp->y, fed->y);
	tp->x = sp->x;
	tp->y = sp->y;
	tp->prox = 200 + randm(200);
	tp->timedelay = 15.;
	tp->id = new_slot();
	if ((i = randm(15) + 10) > sp->energy)
		i = sp->energy;
	tp->fuel = i;
	tp->type = TP_PROBE;
	sp->energy -= i;
	sp->pods -= i;
	printf("%s launching probe #%d\n", sp->name, tp->id);
	return 1;
}


/*
 * Returns the number of tubes that were loaded
 */
int e_loadtubes(sp)
struct ship *sp;
{
	register int i;
	register int j;
	register int loaded;
	register int below;

	below = 10;
	loaded = 0;
	for (i=0; i<sp->num_tubes; i++) {
		if (sp->energy <= below)
			break;
		if (sp->tubes[i].status & T_DAMAGED)
			continue;
		j = min(sp->energy, 10 - sp->tubes[i].load);
		if (j == 0)
			continue;
		sp->energy -= j;
		sp->pods -= j;
		sp->tubes[i].load += j;
		loaded++;
	}
#ifdef TRACE
	if (trace)
		printf("*** Load tubes: Loaded %d tubes\n", loaded);
#endif
	return loaded;
}


/*
 * Returns the number of phasers that were locked
 */
int e_lockphasers(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register int banks;

	banks = 0;
	for (i=0; i<sp->num_phasers; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
			continue;
		if (sp->phasers[i].target != NULL)
			continue;
		sp->phasers[i].target = fed;
		banks++;
	}
#ifdef TRACE
	if (trace)
		printf("*** Lock phasers: Locked %d phasers\n", banks);
#endif
	return banks;
}


/*
 * Returns number of tubes locked
 */
int e_locktubes(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register int tubes;

	tubes = 0;
	for (i=0; i<sp->num_tubes; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			continue;
		if (sp->tubes[i].target != NULL)
			continue;
		sp->tubes[i].target = fed;
		tubes++;
	}
#ifdef TRACE
	if (trace)
		printf("*** Lock tubes: Locked %d tubes\n", tubes);
#endif
	return tubes;
}


/*
 * returns the number of banks we're going to fire
 * it also sets them up.
 */
int e_phasers(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register int banks;
	register int hit;
	register int howmany;
	float bear;

	banks = 0;
	howmany = randm(sp->num_phasers / 2) + sp->num_phasers / 2;
	sp->p_spread = 10 + randm(12);
	for (i=0; i<sp->num_phasers; i++) {
		if ((sp->phasers[i].status & P_DAMAGED) ||
		    (sp->phasers[i].load == 0))
			continue;
		if (fed != NULL) {
			if (sp->phasers[i].target == NULL)
				continue;
			bear = bearing(sp->x, fed->x, sp->y, fed->y);
			hit = phaser_hit(sp, fed->x, fed->y, &sp->phasers[i], bear);
			if (hit <= 0)
				continue;
		}
		banks++;
		sp->phasers[i].status |= P_FIRING;
		if (banks >= howmany)
			break;
	}
	return banks;
}


/*
 * This routine will turn the ship, slowing down if necessary to facilitate
 * the turn.  (Always returns 1)
 */
int e_pursue(sp, fed, speed)
struct ship *sp;
struct ship *fed;
float speed;
{
	float	bear;
	float	coursediff;

	bear = bearing(sp->x, fed->x, sp->y, fed->y);
	/*
	 * do a quick turn if our speed is > max_warp - 2 and
	 * (thus) we are never going to bear on the fed ship
	 * speed = max_warp / 2 is a magic cookie.  Feel free to change.
	 */
	coursediff = rectify(sp->course - bear);
	if (coursediff > 180.0)
		coursediff -= 360.0;
	if (speed >= sp->max_speed - 2 && fabs(coursediff) > 10)
		speed = (int)(sp->max_speed / 2);
	sp->target = fed;
	sp->newcourse = bear;
	sp->newwarp = speed;
	if (speed > 1 && is_dead(sp, S_WARP))
		sp->newwarp = 0.99;
#ifdef TRACE
	if (trace) {
		printf("*** Pursue: Newcourse = %.2f\n", sp->newcourse);
		printf("*** Pursue: Newwarp = %.2f\n", sp->newwarp);
	}
#endif
	return 1;
}


/*
 * This routine has the enemy ship turn its strongest shield towards
 * the enemy and then accelerate to 2/3 maximum speed.  (Always returns 1)
 */
int e_runaway(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register double bear;
	register int strong;
	register double strength;
	register double temp;
	register int sign;
	register float course;
	register int i;

	bear = bearing(sp->x, fed->x, sp->y, fed->y);
	/*
	 * Find the strongest shield
	 */
	strong = 0;
	strength = 0.;
	for (i=0; i< SHIELDS; i++) {
		temp = sp->shields[i].eff * sp->shields[i].drain *
		    (i == 0 ? SHIELD1 : 1.);
		if (temp > strength) {
			strong = i;
			strength = temp;
		}
	}
	switch (strong) {
		case 0:	course = bear;
			sign = -1;
			break;
		case 1:	course = rectify(bear - 90);
			sign = 1;
			break;
		case 2:	course = rectify(bear + 180);
			sign = 1;
			break;
		case 3:	course = rectify(bear + 90);
			sign = 1;
			break;
	}
	sp->target = NULL;
	sp->newcourse = course;
	sp->newwarp = 2 / 3 * sp->max_speed * sign;
	if (sp->newwarp > 1.0 && is_dead(sp, S_WARP))
		sp->newwarp = 0.99;
	if (cansee(sp) && syswork(fed, S_SENSOR))
		printf("%s: The %s is retreating.\n", helmsman, sp->name);
#ifdef TRACE
	if (trace) {
		printf("*** Runaway: Newcourse = %.2f\n", sp->newcourse);
		printf("*** Runaway: Newwarp = %.2f\n", sp->newwarp);
	}
#endif
	return 1;
}


/*
 * Returns the number of tubes we're going to fire
 * Also sets them up to fire
 */
int e_torpedo(sp)
struct ship *sp;
{
	register int i;
	register int tubes;
	register int howmany;
	register struct ship *sp1;
	register int range;

	/*
	 * don't shoot if someone might be in the way
	 * (i.e. proximity fuse will go off right as the
	 * torps leave the tubes!)
	 */
	for (i=1; i <= shipnum; i++) {
		sp1 = shiplist[i];
		/* This must check for dead ships too! */
		if (sp1 == sp)
			continue;
		range = rangefind(sp->x, sp1->x, sp->y, sp1->y);
		if (range <= 400)
			return 0;
	}
	tubes = 0;
	/* This is not and should not be dependent on the
	   number of tubes one has */
	howmany = randm(2) + 1;		
	for (i=0; i<sp->num_tubes; i++) {
		if ((sp->tubes[i].status & T_DAMAGED)
		    || (sp->tubes[i].load == 0))
			continue;
		if (sp->tubes[i].target == NULL)
			continue;
		tubes++;
		sp->tubes[i].status |= T_FIRING;
		if (tubes >= howmany)
			break;
	}
	return tubes;
}

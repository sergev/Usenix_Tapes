/*
 * TREK73: enemycom.c
 *
 * Enemy strategy sub-routines
 *
 * e_phasers, e_lockphasers, e_locktubes, e_checkprobe, e_evade,
 * e_pursue, e_launchprobe, e_destruct, e_torpedo, e_jettison,
 * e_checkarms, e_runaway, e_attack, e_loadtubes, e_closetorp
 *
 */

#include "defines.h"
#include "structs.h"

extern float fabs();
extern char science[];
extern char helmsman[];

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
	int bear;

	banks = 0;
	howmany = randm(2) + 2;
	sp->p_spread = 10 + randm(12);
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
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
		if (banks > howmany)
			break;
	}
	return banks;
}

/*
 * returns positive if we had to lock phasers
 */
int e_lockphasers(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register int banks;

	banks = 0;
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
			continue;
		if (sp->phasers[i].target != NULL)
			continue;
		sp->phasers[i].target = fed;
		banks++;
	}
	return banks;
}

/*
 * returns positive if we had to lock tubes
 */
int e_locktubes(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int i;
	register int tubes;

	tubes = 0;
	for (i=0; i<4; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			continue;
		if (sp->tubes[i].target != NULL)
			continue;
		sp->tubes[i].target = fed;
		tubes++;
	}
	return tubes;
}

/*
 * returns 1 if evasive action being taken (to avoid probe)
 */
e_checkprobe(sp)
struct ship *sp;
{
	extern	struct list head;
	extern	struct list *tail;
	register struct list *lp;
	register int range;
	register struct torpedo *tp;

	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_PROBE)
			continue;
		tp = lp->data.tp;
		range = rangefind(sp->x, tp->x, sp->y, tp->y);
		if (range < 1000) {
			e_evade(sp, tp->x, tp->y, I_PROBE);
			return 1;
		}
	}
	return 0;
}


/*
 * advance to the rear!
 */
e_evade(sp, x, y, type)
struct ship *sp;
int x;
int y;
int type;		/* Currently unused */
{
	register int i;
	register int newcourse;
	int	bear;

	bear = bearing(sp->x, x, sp->y, y);
	printf("%s taking evasive action!\n", sp->name);
	i = randm(3);
	switch (i) {
		case 1:
			newcourse = rectify(bear - 90);
			break;
		case 2:
			newcourse = rectify(bear + 90);
			break;
		case 3:
			newcourse = rectify(bear + 180);
			break;
		default:
			printf("error in evade()\n");
			break;
	}
	sp->target = NULL;
	sp->newcourse = newcourse;
	sp->newwarp = 2 + randm(8);
	if (sp->status & S_WARP)
		sp->newwarp = 1.0;
	type = type;				/* LINT */
	return 1;
}


e_pursue(sp, fed, speed)
struct ship *sp;
struct ship *fed;
int speed;
{
	int	bear;
	int	coursediff;

	bear = bearing(sp->x, fed->x, sp->y, fed->y);
	/*
	 * do a quick turn if our speed is > warp 10 and
	 * (thus) we are never going to bear on the fed ship
	 * speed = 7 is a magic cookie.  feel free to change.
	 */
	coursediff = abs(sp->course - bear);
	if (speed >= 9 && coursediff > 10)
		speed = 5;
	sp->target = fed;
	sp->newcourse = bear;
	sp->newwarp = speed;
	if (speed > 1 && (sp->status & S_WARP))
		sp->newwarp = 0.99;
	return 1;
}


int e_launchprobe(sp, fed)
struct ship *sp;
struct ship *fed;
{
	extern	struct list *newitem();
	register int i;
	register struct list *lp;
	register struct torpedo *tp;

	if ((sp->status & S_PROBE) || sp->energy <= 10)
		return 0;
	/*
	 * fed ship has to be going slow before we'll launch
	 * a probe at it.
	 */
	if (fabs(fed->warp) > 1.0)
		return 0;
	lp = newitem(I_PROBE);
	tp = lp->data.tp = MKNODE(struct torpedo, *, 1);
	printf("%s launching probe\n", sp->name);
	tp->speed = sp->warp;
	tp->newspeed = 3.0;
	tp->target = fed;
	tp->course = bearing(sp->x, fed->x, sp->y, fed->y);
	tp->x = sp->x;
	tp->y = sp->y;
	tp->prox = 200 + randm(200);
	tp->timedelay = 15 * 10;
	i = min(randm(15) + 10, sp->energy);
	tp->fuel = i;
	sp->energy -= i;
	sp->pods -= i;
	return 1;
}


/*
 * goodbye, cruel world
 */
int e_destruct(sp)
struct ship *sp;
{
	if (sp->delay < 5 * 9)
		return 0;
	sp->delay = 5 * 9;
	printf("%s: The %s is overloading what remains of it's\n",science, sp->name);
	printf("   antimatter pods -- obviously a suicidal gesture.\n");
	printf("   Detonation in five seconds\n");
	return 1;
}


int e_torpedo(sp)
struct ship *sp;
{
	extern	struct ship *shiplist[];
	extern	int shipnum;
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
		if ((sp1->status & S_DEAD) || sp1 == sp)
			continue;
		range = rangefind(sp->x, sp1->x, sp->y, sp1->y);
		if (range <= 400)
			return 0;
	}
	tubes = 0;
	howmany = randm(2) + 1;
	for (i=0; i<4; i++) {
		if ((sp->tubes[i].status & T_DAMAGED)
		    || (sp->tubes[i].load == 0))
			continue;
		if (sp->tubes[i].target == NULL)
			continue;
		tubes++;
		sp->tubes[i].status |= T_FIRING;
		if (tubes > howmany)
			break;
	}
	return tubes;
}


int e_jettison(sp)
struct ship *sp;
{
	extern	struct list *newitem();
	register struct list *lp;
	register struct torpedo *tp;

	if (sp->status & S_ENG)
		return 0;
	if (!(shiplist[0]->status & S_SENSOR)) {
		printf("%s: Sensors indicate debris being left by\n", science);
		printf("   the %s.  Insufficient mass?\n", sp->name);
	}
	lp = newitem(I_ENG);
	tp = lp->data.tp = MKNODE(struct torpedo, *, 1);
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
	tp->timedelay = 15 * 9;
	tp->fuel = sp->pods;
	sp->energy = sp->pods = 0;
	sp->regen = 0.0;
	tp->from = sp;
	if (sp->newwarp < -1.0)
		sp->newwarp = -0.99;
	if (sp->newwarp > 1.0)
		sp->newwarp = 0.99;
	sp->status |= S_ENG;
	sp->status |= S_WARP;
	return 1;
}

int e_checkarms(sp)
struct ship *sp;
{
	register int i;
	register int arms;

	arms = 0;
	for (i=0; i<4; i++)
		if (sp->phasers[i].load >= 0)
			arms++;
	for (i=0; i<6; i++)
		if (sp->tubes[i].load >= 0)
			arms++;
	return arms;
}

int e_runaway(sp, fed)
struct ship *sp;
struct ship *fed;
{
	register int speed;

	speed = randm(4) + 2;
	speed = -speed;
	e_pursue(sp, fed, speed);
	printf("%s: The %s is retreating.\n", helmsman, sp->name);
	return 1;
}

int e_attack(sp, fed)
struct ship *sp;
struct ship *fed;
{
	int	speed;
	float	tmpf;

	tmpf = fabs(fed->warp);
	if (sp->warp >= tmpf + 2.0 || (sp->status & S_WARP))
		return 0;
	speed = min(11, tmpf + randm(2) + 2.0);
	e_pursue(sp, fed, speed);
	printf("%s:  %s attacking.\n", helmsman, sp->name);
	return 1;
}

int e_loadtubes(sp)
struct ship *sp;
{
	register int i;
	register int j;
	register int loaded;
	register int below;

	below = 10;
	loaded = 0;
	for (i=0; i<6; i++) {
		if (sp->energy <= below)
			break;
		if (sp->tubes[i].status & T_DAMAGED)
			continue;
		j = min(sp->energy, 10-sp->tubes[i].load);
		if (j == 0)
			continue;
		sp->energy -= j;
		sp->pods -= j;
		sp->tubes[i].load += j;
		loaded++;
	}
	return loaded;
}

e_closetorps(sp, fed)
struct ship *sp;
struct ship *fed;
{
	extern	struct list head;
	extern	struct list *tail;
	register struct list *lp;
	register int range;
	register struct torpedo *tp;
	struct	torpedo *bad;

	bad = NULL;
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_TORPEDO)
			continue;
		tp = lp->data.tp;
		if (tp->from != fed)
			continue;
		range = rangefind(sp->x, tp->x, sp->y, tp->y);
		if (range < 1200) {
			bad = tp;
			/*
			 * fire phasers - hope they're pointing in
			 * the right direction!
			 */
			if (e_phasers(sp, (struct ship *) NULL))
				return 1;
			return 1;
		}
	}
	/*
	 * we can't get a phaser shot off.
	 * try and evade (although hopeless)
	 */
	if (bad != NULL) {
		e_evade(sp, tp->x, tp->y, I_TORPEDO);
		return 1;
	}
	return 0;
}

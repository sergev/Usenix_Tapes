/*
 * TREK73: damage.c
 *
 * Damage routines
 *
 * damage, check_locks
 *
 */

#include "externs.h"


damage(hit, ep, s, dam, flag)
int hit;
struct ship *ep;
int s;
struct damage *dam;
int flag;
{
	register int i;
	register int j;
	register int k;
	float	f1;		/* Damage factor except for shields */
	float	f2;		/* Shield damage factor */
	int percent;
	struct ship *fed;

	fed = shiplist[0];
	printf("hit %d on %s's shield %d\n", hit, ep->name, s);
	s--;
	/*
	 * Note that if the shield is at 100% efficiency, no
	 * damage at all will be taken (except to the shield itself)
	 */
	f1 = hit * (1.0 - ep->shields[s].eff * ep->shields[s].drain);
	if (f1 < 0)
		return 0;
	/* Calculate shield damage */
	if (flag == D_ANTIMATTER)
		f2 = ep->tu_damage * 100;
	else if (flag == D_PHASER)
		f2 = ep->ph_damage * 100;
	if (s == 0)
		f2 *= SHIELD1;
	ep->shields[s].eff -= max(hit/f2, 0);
	if (ep->shields[s].eff < 0.0)
		ep->shields[s].eff = 0.0;
	/* Calculate loss of fuel, regeneration, etc. */
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
	/* Kill some crew */
	if (ep->complement > 0) {
		j = f1 * dam->crew;
		if (j > 0)
			ep->complement -= randm(j);
		if (ep->complement < 0)
			ep->complement = 0;
	}
	/* Damage some weapons */
	j = f1/dam->weapon;
	for(i=0; i<j; i++) {
		k = randm(ep->num_phasers + ep->num_tubes) - 1;
		if (k < ep->num_phasers) {
			if (ep->phasers[k].status & P_DAMAGED)
				continue;
			ep->phasers[k].status |= P_DAMAGED;
			ep->phasers[k].target = NULL;
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
				printf("   phaser %d damaged\n", k);
		} else {
			k -= ep->num_phasers;
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
			ep->tubes[k].target = NULL;
			k++;
			if (ep == fed)
				printf("   tube %d damaged\n", k);
		}
	}
	/* Damage the different systems */
	for (i=0; i<S_NUMSYSTEMS; i++) {
		if (is_dead(ep, i))	/* Don't damage a dead system */
			continue;
		percent = 0;
		if (randm(dam->stats[i].roll) < f1) {
			/* A better method should be found */
			percent = (int) randm((int) f1);
			/* The expected value for the percent damage
			   to each system is roughly equal to:
			      f1 * f1 / (2 * dam->stats[i].roll)
			   Only these damages are proportional to hit
			   squared.  All others are linearly
			   proportional.  This includes shield damage,
			   ship's fuel supply, consumption and
			   regeneration rates, casualties, and weapons.
			   (When weapons are damaged, they are 100%
			   damaged - the number of weapons damaged is
			   proportional to hit.)
			   I think the old way decided whether or not to
			   completely damage a system based on the
			   comparison "randm(dam->stats[i].roll) < f1".
			   This is almost like the weapons are still
			   handled.  Another possibility is to always
			   damage each system by:
			      100 * randm((int)f1) / dam->stats[i].roll
			   percent.  This adds some randomness and makes
			   the approx. expected value of the damage to
			   each system:
			      100 * f1 / (2 * dam->stats[i].roll)
			   percent.  Perhaps this isn't such a good
			   idea after all; this is 100/f1 times the
			   current expected value, often > 2.  And it is
			   actually somewhat less random since each
			   system gets damaged each time.  I had thought
			   that the damage should be directly
			   proportional to f1, not to its square.
			   But perhaps it makes sense that a hit twice
			   as big has an expected value of damage four
			   times as big as that from a smaller hit.
			   The actual damage any given time is still
			   proportional to the hit, but the probability
			   that any damage will be done at all is also
			   directly proportional to the hit.  This is
			   a pretty good system after all.	[RJN]
			*/
			ep->status[i] += percent;
			if (ep->status[i] > 100)
				ep->status[i] = 100;
			if (ep == fed) {
				if (is_dead(ep, i))
					printf("   %s\n",
				    	    dam->stats[i].mesg);
				else
					printf("   %s damaged.\n",
					    sysname[i]);
			}
			/* Now check for the effects of the damage */
			/* Do the effects of a totally destroyed system */
			if (is_dead(ep, i)) {
				switch(i) {
				case S_SENSOR:
				case S_PROBE:
					/* No bookkeeping needed */
					break;
				case S_WARP:
					/* Reduce max speed */
					ep->max_speed = 1.0;
					break;
				case S_COMP:
					check_locks(ep, 100, fed);
					break;
				default:
					printf("How'd we get here?\n");
				}
			} else {
				/* Now check partially damaged systems */
				switch(i) {
				case S_SENSOR:
				case S_PROBE:
					/* No bookkeeping needed */
					break;
				case S_WARP:
					f2 = percent * ep->orig_max / 100;
					ep->max_speed -= f2;
					if (ep->max_speed < 1.0) {
						ep->max_speed = 1.0;
						ep->status[S_WARP] = 100;
					}
					break;
				case S_COMP:
					check_locks(ep, percent, fed);
					break;
				default:
					printf("Oh, oh....\n");
				}
			}
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
		ep->delay = 1.;
#endif
	return 0;
}

check_locks(ep, percent, fed)
struct ship *ep;
int percent;
struct ship *fed;
{
	register int i, j = 0;

	for (i=0; i<ep->num_phasers; i++) {
		if ((ep->phasers[i].target != NULL)
		    && (randm(100) <= percent)) {
			ep->phasers[i].target = NULL;
			if (ep != fed)
				continue;
			if (!j)
				printf("Computer: Phaser(s) %d", i+1);
			else
				printf(", %d", i+1);
			j++;
		}
	}
	if (j > 1)
		puts(" have lost their target locks.");
	else if (j == 1)
		puts(" has lost its target lock.");
	j = 0;
	for (i=0; i<ep->num_tubes; i++) {
		if ((ep->tubes[i].target != NULL)
		    && (randm(100) <= percent)) {
			ep->tubes[i].target = NULL;
			if (ep != fed)
				continue;
			if (!j)
				printf("Computer: Tube(s) %d", i+1);
			else
				printf(", %d", i+1);
			j++;
		}
	}
	if (j > 1)
		puts(" have lost their target locks.");
	else if (j == 1)
		puts(" has lost its target lock.");
	if ((ep->target != NULL) && (randm(100) <= percent)) {
		ep->target = NULL;
		ep->relbear = 0;
		if (ep == fed)
			printf("Computer: %s has lost helm lock\n",
			    shipname);
	}
}

/*
 * TREK73: dist.c
 *
 * Power distribution routines
 *
 * distribute
 *
 */

#include "defines.h"
#include "structs.h"
#include <math.h>

extern char engineer[];

distribute(sp)
struct ship *sp;
{
	extern char title[];
	register int i;
	register int fuel;
	register int load;
	register int effload;
	register int drain;
	float shield;
	extern char shutup[];
	extern struct ship *shiplist[];

	fuel = sp->energy + (int)(sp->regen * 2);
	/*
	 * Calculate negative phaser drains
	 */
	for (i=0; i<4; i++) {
		load = sp->phasers[i].load;
		drain = sp->phasers[i].drain;
		if ((sp->phasers[i].status & P_DAMAGED) || (drain >= 0)
		    || (sp->phasers[i].load <= 0))
			continue;
		/*
		 * Drain the lesser of either the current load if the
		 * load is less than the drain, or the drain value
		 */
		if (drain < 0) {
			effload = max(load + drain, 0);
			fuel += load - effload;
			sp->phasers[i].load = effload;
		}
	}
	/*
	 * Calculate shield drains
	 */
	shield = 0.0;
	for (i=0; i<4; i++)
		shield += sp->shields[i].attemp_drain;
	drain = ceil((double) shield);
	/*
	 * If all attempted drains are zero, or we have no
	 * fuel, our shields are down!
	 */
	if (((shield == 0) || (fuel == 0)) && !shutup[SHIELDSF] && sp == shiplist[0]) {
		printf("%s: %s, our shields are down!\n",engineer, title);
		shutup[SHIELDSF]++;
	}
	/*
	 * If there's not enough fuel to sustain the drains, then
	 * ration it out in proportion to the attempted drains and
	 * say that shields are fluctuating.
	 */
	if (drain <= fuel) {
		fuel -= drain;
		for (i=0; i<4; i++)
			sp->shields[i].drain = sp->shields[i].attemp_drain;
	} else {
		if (!shutup[SHIELDSF] && sp == shiplist[0]) {
			printf("%s: %s, our shields are fluctuating!\n", engineer, title);
			shutup[SHIELDSF]++;
		}
		for (i=0; i<4; i++)
			if (!sp->shields[i].attemp_drain)
				sp->shields[i].drain = sp->shields[i].attemp_drain * (float) fuel / drain;
			else
				sp->shields[i].drain = 0;
		fuel = 0;
	}
	/*
	 * Calculate positive phaser drains
	 */
	for (i=0; i<4 && fuel > 0; i++) {
		if (fuel <=0)
			break;
		load = sp->phasers[i].load;
		drain = sp->phasers[i].drain;
		if ((sp->phasers[i].status & P_DAMAGED) || load >= 10 || drain <= 0)
			continue;
		/*
		 * Load phasers either enough to top them off, or
		 * the full drain
		 */
		if (drain > 0) {
			effload = min(10, load + min(drain, fuel));
			fuel -= effload - load;
			sp->phasers[i].load = effload;
		}
	}
	/*
	 * Now balance the level of energy with the numer of pods
	 */
	if (fuel > sp->pods)
		sp->energy = sp->pods;
	else 
		sp->energy = fuel;
}

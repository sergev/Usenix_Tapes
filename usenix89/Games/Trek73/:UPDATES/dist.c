/*
 * TREK73: dist.c
 *
 * Power distribution routines
 *
 * distribute
 *
 */

#include "externs.h"
#include <math.h>


distribute(sp)
struct ship *sp;
{
	register int i;
	register float fuel;
	register int load;
	register int effload;
	register int drain;
	register int loop;
	float shield;
	struct ship *fed;

	fed = shiplist[0];
	/*
	 * Granularity of 1 second as far as this loop is concerned
	 */
	for (loop = 0; loop < (int)timeperturn; loop++) {

		fuel = sp->energy + sp->regen;	/* Slightly unrealistic */
		/*
		 * Calculate negative phaser drains
		 */
		for (i=0; i<sp->num_phasers; i++) {
			load = sp->phasers[i].load;
			drain = sp->phasers[i].drain;
			if ((sp->phasers[i].status & P_DAMAGED)
			    || (drain >= 0) || (load <= 0))
				continue;
			/*
			 * Drain the lesser of either the current load if the
			 * load is less than the drain, or the drain value
			 */
			effload = max(load + drain, 0);
			fuel += load - effload;
			sp->phasers[i].load = effload;
		}
		/*
		 * Calculate shield drains
		 */
		shield = 0.0;
		for (i=0; i<SHIELDS; i++)
			shield += sp->shields[i].attemp_drain;
		drain = ceil((double) shield);
		/*
		 * If all attempted drains are zero, or we have no
		 * fuel, our shields are down!
		 */
		if ((shield * fuel == 0) && !shutup[SHIELDSF]
		    && sp == shiplist[0]) {
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
			for (i=0; i<SHIELDS; i++)
				sp->shields[i].drain = sp->shields[i].attemp_drain;
		} else {
			if (!shutup[SHIELDSF] && sp == shiplist[0]) {
				printf("%s: %s, our shields are fluctuating!\n",
				    engineer, title);
				shutup[SHIELDSF]++;
			}
			for (i=0; i<SHIELDS; i++)
				sp->shields[i].drain =
				    sp->shields[i].attemp_drain *
				    fuel / drain;
			fuel = 0.;
		}
		/*
		 * Calculate cloaking device drains.  If there is
		 * in sufficient energy to run the device, then
		 * it is turned off completely
		 */
		if (cantsee(sp)) {
			if (fuel < sp->cloak_energy) {
				if (sp == shiplist[0]) {
					sp->cloaking = C_OFF;
					printf("%s:  %s, there's not enough energy to",
					    engineer, title);
					puts("    keep our cloaking device activated.");
				} else
					(void) e_cloak_off(sp, fed);
			} else
				fuel -= sp->cloak_energy;
		}
		/*
		 * Calculate positive phaser drains
		 */
		for (i=0; i<sp->num_phasers && fuel > 0; i++) {
			if (fuel <=0.)
				break;
			load = sp->phasers[i].load;
			drain = sp->phasers[i].drain;
			if ((sp->phasers[i].status & P_DAMAGED)
			    || load >= MAX_PHASER_CHARGE || drain <= 0)
				continue;
			/*
			 * Load phasers either enough to top them off, or
			 * the full drain
			 */
			effload = min(MAX_PHASER_CHARGE,
			    load + min(drain, fuel));
			fuel -= effload - load;
			sp->phasers[i].load = effload;
		}
		/*
		 * Now balance the level of energy with the numer of pods
		 */
		sp->energy = min(fuel, sp->pods);
	}
}

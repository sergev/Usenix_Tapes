/*
 * TREK73: strat1.c
 *
 * Standard Enemy Strategy
 *
 * standard_strategy
 *
 */

#include "defines.h"
#include "structs.h"


standard_strategy()
{
	extern	float fabs();
	extern	struct ship *shiplist[];
	extern	int shipnum;
	extern	int defenseless;
	extern  int corbomite;
	extern 	int surrender;
	extern  int surrenderp;
	extern  char captain[];
	extern	char science[];
	extern	char nav[];
	extern  char com[];
	extern	char helmsman[];
	extern 	char title[];
	extern	char foename[];
	extern	char foerace[];
	extern	int global;
	extern	char empire[];
	int	i;
	register struct ship *sp;
	register struct ship *fed;
	int	bear;
	int	range;
	float	tmpf;
	int	loop, loop2;
	int	probability;

	fed = shiplist[0];
	for (i=1; i <= shipnum; i++) {
		sp = shiplist[i];
		if (sp->status & S_DEAD)
			continue;
		range = rangefind(sp->x, fed->x, sp->y, fed->y);
		bear = bearing(sp->x, fed->x, sp->y, fed->y);
		bear = rectify(sp->course - bear);
		/*
		 * Take care of special commands like defenseless ruses,
		 * corbomite bluffs, surrenders (both sides)
		 */
		/*
		 * Play dead effects
		 */
		switch (defenseless) {
		case 1: {
			if (randm(10) > 2) {
				defenseless = 2;
			dstrat:
				if (randm(2) == 1) {
					sp->target = NULL;
					sp->newwarp = 0.0;
				} else {
					sp->newwarp = 1.0;
				}
				printf("%s:   The %s is ",helmsman, sp->name);
				if (sp->target != NULL) {
					printf("cautiously advancing.\n");
				} else {
					printf("turning away.\n");
				}
			astrat:
				if ((fabs(sp->target->warp) > 1.0)
				    || (range < 200)) {
					defenseless = 6;
				} else {
					for (loop = 0; loop < 4; loop++) {
						if (sp->target->shields[loop].drain) {
							defenseless = 6;
						}
					}
				}
			} else {
				printf("%s:   No apparent change in the enemy's actions.\n", helmsman);
				defenseless = 6;
			}
		}
		break;
		case 2: goto dstrat;
		case 4: goto astrat;
		case 5: goto astrat;
		}
		/*
		 * Corbomite bluff effects.
		 */
		switch (corbomite) {
		case 1: {
			probability = 30;
			if (!strcmp(foerace, "Romulan")) {
				probability = 50;
			}
			if (randm(100) < probability) {
				printf("%s:   %ss giving ground, Captain.  Obviously, they\n", science, foerace);
				printf("   tapped in as you expected them to.\n");
				printf("%s:  A logical assumption, Mr. %s.  Are they still\n", captain, science);
				printf("   retreating?\n");
				printf("%s:  Yes, %s\n",science, title);
				printf("%s:  Good.  All hands, stand by.\n",captain);
				corbomite = 2;
			cstrat:
				sp->target = NULL;
				sp->newwarp = 3.0 + randm(7);
			} else {
				printf("%s:  Message coming in from the %ss.\n",com, foerace);
				printf("%s:  Put it on audio.\n",captain);
				if (randm(2) == 1) {
					printf ("%s:  Ha, ha, ha, %s.  You lose.\n", foename, captain);
				} else {
					printf("%s:  I fell for that the last time we met, idiot!\n", foename);
				}
				corbomite = 6;
			}
		}
		break;
		case 2: goto cstrat;
		case 3: if (fabs(sp->target->warp) > 2.0) {
				corbomite = 6;
			}
			break;
		case 4: if (fabs(sp->target->warp) > 2.0) {
				corbomite = 6;
			}
			break;
		case 5: if (fabs(sp->target->warp) > 2.0) {
				corbomite = 6;
			}
			break;
		}
		/*
		 * Will the enemy accept your surrender?
		 */
		 
		if (surrender)
			switch (surrender) {
			case 1: {
				probability=49;
				if (!strcmp(foerace, "Romulan")) {
					probability = 4;
					printf("%s:  The %ss do not take prisoners.\n", nav, foerace);
				}
				if (randm(100) < probability) {
					if (randm(2) == 1) {
						printf("%s:  Prepare to die, Chicken %s!\n", foename, captain);
					} else {
						printf("%s:  No reply from the %ss.\n",com, foerace);
					}
					surrender = 6;
				} else {
					printf("%s:  Message coming in from the %s.\n", com, sp->name);
					printf("%s:  On behalf of the %s %s, I accept your surrender.\n", foename, foerace, empire);
					printf("   You have five seconds to drop your shields, cut\n");
					printf("   warp, and prepare to be boarded.\n");
					surrender = 2;
					global |= F_SURRENDER;
				sstrat:
					sp->target = fed;
					sp->newwarp = 2.0;
				}
				break;
			}
			case 2:
			case 3: {
				warn(3);
				goto sstrat;
			}
			case 4:
			case 5: {
				for (loop = 0; loop < 4; loop++) {
					if (sp->target->shields[loop].attemp_drain)
						goto breakout;
				}
				if (range <= 1400)
					sp->newwarp = 1.0;
				if ((range > 1000) || (fabs(sp->target->warp) > 1.0)) {
					if (surrender == 5) {
						printf("%s:  Captain %s, you have not fulfilled our terms.\n", foename, captain);
						printf("  We are resuming our attack.\n");
					}
					goto breakout;
				} else {
					fed->status |= S_SURRENDER;
					final(3);
				}
			}
			default: global &= ~F_SURRENDER;
				break;
			breakout:
				break;
			}
		/*
		 * Enemy surrenders?
		 */
		switch (surrenderp) {
			case 1:
			for (loop = 1; loop < shipnum; loop++)
				if (!(sp->status & S_ENG) && (sp->crew > 100)) {
					printf("%s:   You must be joking, Captain %s.  Why don't you\n", foename, captain);
					printf("   surrender?\n");
					surrenderp = 6;
					break;
				}
			if (loop = shipnum) {
				probability = 49;
				if (!strcmp (foerace, "Romulan"))
					probability = 4;
				if (randm(100) < probability) {
					printf("%s:  As much as I hate to, Captain %s, we will surrender.\n", foename, captain);
					printf("   We are dropping shields.  You may board us.\n");
					surrenderp = 2;
				dropshields:
					for (loop = 0; loop < 3; loop++)
						sp->shields[0].drain = 0;
					sp->newwarp = 0.0;
					for (loop = 1; loop < shipnum; loop++)
						shiplist[loop]->status |= S_SURRENDER;
					global |= E_SURRENDER;
				} else {
					printf("%s:   You must be joking, Captain %s.  Why don't you\n", foename, captain);
					printf("   surrender?\n");
					surrenderp = 6;
				}
			}
			break;
		case 2:
		case 3:
			warn(4);
			goto dropshields;
		}
		/*
		 * Unsportsmanlike firing
		 */
		if (betw(defenseless, 0, 6) || betw(corbomite, 0, 6)
		    || betw(surrender, 0, 6) || betw(surrenderp, 0, 6)) {
			for (loop = 0; loop < 4; loop++)
				if (fed->phasers[i].status & P_FIRING)
					break;
			for (loop2 = 0; loop2 < 6; loop2++)
				if (fed->tubes[i].status & T_FIRING)
					break;	
			/* Has he fired? */
			if ((loop != 4) || (loop2 != 6)) {
				/* Yes, be angry and disbelieve everything from now on */
				printf("%s: How dare you fire on us!  We are\n", foename);
				printf("  resuming our attack!\n");
				global = NORMAL;
				if (betw(defenseless,0,6))
					defenseless = 6;
				if (betw(corbomite,0,6))
					corbomite = 6;
				if (betw(surrender,0,6))
					surrender = 6;
				if (betw(surrenderp,0,6))
					surrenderp = 6;
				for (loop = 0; loop < shipnum; loop++)
					shiplist[loop]->status &= ~S_SURRENDER;
			}
		}
		if ((global & F_SURRENDER) || (global & E_SURRENDER))
			continue;
		/*
		 * short range? 
		 *   1). fire phasers
		 *   2). lock phasers
		 *   3). evade probes
		 *   4). enemy to the rear - turn around
		 *   5). launch a probe?
		 *   6). self destruct?
		 *   7). set course.
		 */
		if (range < 1050) {
			if (e_closetorps(sp, fed))
				continue;
			if (e_lockphasers(sp, fed))
				continue;
			if (e_phasers(sp, fed))
				continue;
			if (e_checkprobe(sp))
				continue;
			if (bear > 90 && bear < 270) {
				e_pursue(sp, fed, 1);
				continue;
			}
			if (bear > 90 && bear < 270 && e_launchprobe(sp, fed))
				continue;
			if (sp->pods<20 && sp->regen < 4.0 && e_destruct(sp))
				continue;
			/*
			 * set course?
			 */
			tmpf = fabs(fed->warp);
			if (sp->target != fed || sp->warp + tmpf > 2.0) {
				e_pursue(sp, fed, (int)tmpf);
				continue;
			}
		}
		if (range < 3800) {
			/*
			 * either medium range, or we can't figure out what
			 * to do at short range
			 */
			if (e_torpedo(sp))
				continue;
			if (e_locktubes(sp, fed))
				continue;
			/*
			 * should we run away; can we?
			 */
			if (e_checkarms(sp) < randm(3)) {
				e_runaway(sp, fed);
				continue;
			}
			/*
			 * pursued from behind, low power: jettison engineering!
			 */
			if (sp->energy<10 && sp->regen < 4.0 && e_jettison(sp))
				continue;
			/*
			 * put in other junk later
			 */
		}
		/*
		 * either distant range, or we can't figure out
		 * what to do at medium range
		 */
		if (fed->delay < 15*10 && (sp->status & S_WARP)) {
			e_runaway(sp, fed);
			continue;
		}
		/*
		 * enemy behind us?  make a quick turn.
		 */
		if (bear > 135 && bear < 225) {
			e_pursue(sp, fed, 1);
			continue;
		}
		/*
		 * attack?
		 */
		if (sp->pods > 40 && e_attack(sp, fed))
			continue;
		if (sp->energy > 30 && sp->pods > 40)
			if (e_loadtubes(sp))
				continue;
		if (e_locktubes(sp, fed))
			continue;
		if (e_lockphasers(sp, fed))
			continue;
		/*
		 * gee, there's nothing that we want to do!
		 */
		printf("%s:  We're being scanned by the %s\n", science, sp->name);
	}
}

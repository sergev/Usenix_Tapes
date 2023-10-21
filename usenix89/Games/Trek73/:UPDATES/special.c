/*
 * TREK73: special.c
 *
 * special: Take care of special commands like defenseless ruses,
 * corbomite bluffs, surrenders (both sides)
 *
 */

#include "externs.h"


special(sp, range, fed)
struct ship *sp;
int range;
struct ship *fed;
{
	int loop, loop2, loop3;

	/*
	 * Play dead effects
	 */
	switch (defenseless) {
	case 1:
		/* Monty Pythons? */
		if (aliens[enemynum].defenseless == -1)
			aliens[enemynum].defenseless = randm(100);
		if (randm(100) > aliens[enemynum].defenseless) {
			/* Didn't work.  Too bad. */
			if (cansee(sp)) {
				printf("%s:   No apparent change in the enemy's actions.\n",
				    helmsman);
			}
			defenseless = 6;
			break;
		}
		defenseless = 2;
	case 2:
	case 3:
		/* Okay, he's fallen for it.  Choose his action */
		if (randm(2) == 1) {
			sp->target = NULL;
			sp->newwarp = 0.0;
		} else {
			sp->newwarp = 1.0;
		}
		if (cansee(sp)) {
			printf("%s:   The %s is ", helmsman, sp->name);
			if (sp->target != NULL) {
				puts("cautiously advancing.");
			} else {
				puts("turning away.");
			}
		}
	case 4:
	case 5:
		/*
		 * Now he might get suspicious.  If he's moving too
		 * fast or if we're close enough, or if his shields
		 * are up, we'll spot him.
		 */
		if ((fabs(sp->target->warp) > 1.0) || (range < 200))
			defenseless = 6;
		else
			for (loop = 0; loop < SHIELDS; loop++)
				if (sp->target->shields[loop].drain)
					defenseless = 6;
	}

	/*
	 * Corbomite bluff effects.
	 */
	switch (corbomite) {
	case 1:
		/* Monty Pythons? */
		if (aliens[enemynum].corbomite == -1)
			aliens[enemynum].corbomite = randm(100);
		if (randm(100) > aliens[enemynum].corbomite) {
			/* He didn't fall for it */
			printf("%s:  Message coming in from the %ss.\n",
			    com, foerace);
			printf("%s:  Put it on audio.\n", captain);
			if (randm(2) == 1)
				printf ("%s:  Ha, ha, ha, %s.  You lose.\n",
				    foename, captain);
			else
				printf("%s:  I fell for that the last time we met, idiot!\n",
				    foename);
			corbomite = 6;
			break;
		}
		if (cansee(sp)) {
			printf("%s:   %ss giving ground, Captain.  Obviously they\n",
			    science, foerace);
			puts("   tapped in as you expected them to.");
			printf("%s:  A logical assumption, Mr. %s.  Are they still\n",
			    captain, science);
			puts("   retreating?");
			printf("%s:  Yes, %s\n", science, title);
			printf("%s:  Good.  All hands, stand by.\n", captain);
		}
		corbomite = 2;
	case 2:
		/* He fell for it, retrograde out of here! */
		sp->target = NULL;
		sp->newwarp = -(3.0 + randm(7));
		break;
	case 3:
	case 4:
	case 5:
		/* Begin to get suspicious */
		if (fabs(sp->target->warp) > 2.0)
			corbomite = 6;
		break;
	}

	/*
	 * Will the enemy accept your surrender?
	 */
	switch (surrender) {
	case 1:
		/* Monty Python */
		if (aliens[enemynum].surrender == -1)
			aliens[enemynum].surrender = randm(100);
		/* Just a little reminder */
		if (aliens[enemynum].surrender <= 10)
			printf("%s:  The %ss do not take prisoners.\n",
			    nav, foerace);
		if (randm(100) > aliens[enemynum].surrender) {
			/* Tough luck */
			if (randm(2) == 1) {
				printf("%s:  Message coming in from the %ss.\n",
				    com, foerace);
				printf("%s:  Put it on audio.\n", captain);
				printf("%s:  Prepare to die, Chicken %s!\n",
				    foename, captain);
			} else
				printf("%s:  No reply from the %ss",
				    com, foerace);
			surrender = 6;
			break;
		}
		/* He took it! */
		printf("%s:  Message coming in from the %ss.\n",
		    com, foerace);
		printf("%s:  Put it on audio.\n", captain);
		printf("%s:  On behalf of the %s %s, I accept your surrender.\n",
		    foename, foerace, empire);
		puts("   You have five seconds to drop your shields, cut");
		puts("   warp, and prepare to be boarded.");
		global |= F_SURRENDER;
	case 2:
	case 3:
		if (surrender == 1)
			surrender = 2;
		else
			(void) warn(FIN_F_SURRENDER);
		sp->target = fed;
		sp->newwarp = sp->max_speed;
		(void) e_cloak_off(sp, fed);
		break;
	case 4:
	case 5:
		/* Begin checking surrender conditions */
		for (loop = 0; loop < SHIELDS; loop++)
			if (sp->target->shields[loop].drain)
				break;
		if (loop < SHIELDS)
			break;
		if (range <= 1400)
			sp->newwarp = 1.0;
		if ((range <= 1000) && (fabs(sp->target->warp) <= 1.0)) {
			fed->status[S_SURRENDER] = 100;
			final(FIN_F_SURRENDER);
		}
		if (surrender == 4)
			break;
		if (!shutup[SURRENDER])
			printf("%s:  Captain %s, you have not fulfilled our terms.\n",
			    foename, captain);
			printf("  We are resuming our attack.\n");
			surrender = 6;
		shutup[SURRENDER]++;
	default:
		global &= ~F_SURRENDER;
		break;
	}

	/*
	 * Enemy surrenders?
	 */
	switch (surrenderp) {
	case 1:
		for (loop = 1; loop <= shipnum; loop++)
			if (!is_dead(shiplist[loop], S_ENG)
			    && (sp->complement > 100)) {
				printf("%s:  Message coming in from the %ss.\n",
				    com, foerace);
				printf("%s:  Put it on audio.\n", captain);
				printf("%s:  You must be joking, Captain %s.\n",
				    foename, captain);
				puts("  Why don't you surrender?");
				surrenderp = 6;
				break;
			}
		if (loop <= shipnum)
			break;
		/* Monty Python */
		if (aliens[enemynum].surrenderp == -1)
			aliens[enemynum].surrenderp = randm(100);
		if (randm(100) > aliens[enemynum].surrenderp) {
			printf("%s:  I'll never surrender to you, %s\n",
			    foename, captain);
			surrenderp = 6;
			break;
		}
		printf("%s:  As much as I hate to, Captain %s, we will surrender.\n",
		    foename, captain);
		puts("   We are dropping shields.  You may board us.");
	case 2:
	case 3:
		if (surrenderp == 1)
			surrenderp = 2;
		else
			(void) warn(FIN_E_SURRENDER);
		for (loop = 0; loop < SHIELDS; loop++)
			sp->shields[loop].attemp_drain = 0.0;
		sp->newwarp = 0.0;
		for (loop = 1; loop <= shipnum; loop++)
			shiplist[loop]->status[S_SURRENDER] = 100;
		global |= E_SURRENDER;
		break;
	}

	/*
	 * Unsportsmanlike firing
	 */
	if (betw(defenseless, 0, 6) || betw(corbomite, 0, 6)
	    || betw(surrender, 0, 6) || betw(surrenderp, 0, 6)) {
		for (loop = 0; loop < fed->num_phasers; loop++)
			if (fed->phasers[loop].status & P_FIRING)
				break;
		for (loop2 = 0; loop2 < fed->num_tubes; loop2++)
			if (fed->tubes[loop].status & T_FIRING)
				break;	
		loop3 = (fed->probe_status != PR_NORMAL);
		/* Has he fired? */
		if ((loop != fed->num_phasers) ||
		    (loop2 != fed->num_tubes) ||
		    (loop3 == 1)) {
			/* Yes, be angry and disbelieve everything from now on */
			printf("%s: How dare you fire on us!  We are resuming our attack!\n",
			    foename);
			global = NORMAL;
			if (betw(defenseless,0,6))
				defenseless = 6;
			if (betw(corbomite,0,6))
				corbomite = 6;
			if (betw(surrender,0,6))
				surrender = 6;
			if (betw(surrenderp,0,6))
				surrenderp = 6;
			for (loop = 0; loop <= shipnum; loop++)
				shiplist[loop]->status[S_SURRENDER] = 0;
		}
	}
}

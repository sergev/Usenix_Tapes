/*
 * TREK73: cmds4.c
 *
 * User Commands
 *
 * alterpntparams, play_dead, corbomite_bluff, surrender_ship,
 * request_surrender, self_destruct, abort_self_destruct
 *
 */

#include "externs.h"


alterpntparams(sp)
struct ship *sp;
{
	int temp;
	char buf1[30];

	printf("\n%s:  Reset tubes, %s?\n",nav,title);
	printf("%s:  [yes or no] ",captain);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	if ((buf1 != NULL) && (buf1[0] != 'n')) {
		printf("   Set launch speed to [0-%d] ", MAX_TUBE_SPEED);
		(void) Gets(buf1, sizeof(buf1));
		if (buf1[0] == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < MAX_TUBE_SPEED + 1))
				sp->t_lspeed = temp;
		}
		printf("   Set time delay to [0-%d] ", (int) MAX_TUBE_TIME);
		(void) Gets(buf1, sizeof(buf1));
		if (buf1[0] == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < (int)(MAX_TUBE_TIME + 1)))
				sp->t_delay = temp;
		}
		printf("   Set proximity delay to [0-%d] ", MAX_TUBE_PROX);
		(void) Gets(buf1, sizeof(buf1));
		if (buf1[0] == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < MAX_TUBE_PROX + 1))
				sp->t_prox = temp;
		}
	}
	printf("%s:  Reset phasers, %s?\n",nav ,title);
	printf("%s:  [yes or no] ",captain);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	if ((buf1 != NULL) && (buf1[0] != 'n')) {
		printf("   Reset firing percentage to [0-100] ");
		if (Gets(buf1, sizeof(buf1)) == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < 101))
				sp->p_percent = temp;
		}
	}
	return 0;
}

play_dead(sp)
struct ship *sp;
{
	
	char buf1[30];
	int  phaser_charge;
	register int i;

	printf("%s:   %s, drop shields ...\n", captain, nav);
	if (defenseless) {
		printf("%s:   %s, the %ss are not that stupid.\n",
		    science, title, foerace);
		return 0;
	}
	printf("   Transfer power to [engines or phasers]: ");
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL) {
		printf("%s:   I cannot transfer power there, %s.\n",
		    nav, title);
		return 0;
	}
	phaser_charge = -MAX_PHASER_CHARGE;
	if (buf1[0] != 'e' && buf1[0] != 'E') {
		phaser_charge = MAX_PHASER_CHARGE;
		if (buf1[0] != 'p' && buf1[0] != 'P') {
			printf("%s:   I cannot transfer power there, %s.\n",
			    nav, title);
			return 0;
		}
	}
	for (i=0;i<SHIELDS;i++) 
		sp->shields[i].attemp_drain = 0.0;
	for (i=0;i<sp->num_phasers;i++)
		sp->phasers[i].drain = phaser_charge;
	defenseless = 1;
	return 0;
}

corbomite_bluff(sp)
struct ship *sp;
{
	
	if (randm(2) == 1) {
		printf("%s:   Open a hailing frequency, ship-to-ship.\n",
		    captain);
		printf("%s:  Hailing frequency open, %s.\n", com, title);
		printf("%s:  This is the Captain of the %s.  Our respect for\n",
		    captain, sp->name);
		puts("   other life forms requires that we give you this warning--");
		puts("   one critical item of information which has never been");
		puts("   incorporated into the memory banks of any Earth ship.");
		puts("   Since the early years of space exploration, Earth vessels");
		puts("   have had incorporated into them a substance know as corbomite.");
		if (!corbomite) {
			puts("      It is a material and a device which prevents attack on");
			puts("   us.  If any destructive energy touchs our vessel, a re-");
			puts("   verse reaction of equal strength is created, destroying");
			puts("   the attacker.  It may interest you to know that, since");
			puts("   the initial use of corbomite for more than two of our");
			puts("   centuries ago, no attacking vessel has survived the attempt.");
			puts("   Death has little meaning to us.  If it has none to you,");
			puts("   then attack us now.  We grow annoyed with your foolishness.");
		}
	} else {
		printf("%s:   Open a special channel to Starfleet Command.\n",
		    captain);
		printf("%s:   Aye, %s.\n", com, title);
		printf("%s:   Use Code 2.\n", captain);
		printf("%s:   but, %s, according to our last Starfleet\n",
		    com, title);
		printf("   Bulletin, the %ss have broken code 2.\n", foerace);
		printf("%s:   That's an order, Lieutenant.  Code 2!\n",
		    captain);
		printf("%s:   Yes, Captain.  Code 2.\n", com);
		printf("%s:   Message from %s to Starfleet Command, this sector.\n",
		    captain, sp->name);
		printf("   have inadvertantly encroached upon %s neutral zone,\n",
		    foerace);
		printf("   surrounded and under heavy %s attack.  Escape\n",
		    foerace);
		puts("   impossible.  Shields failing.  Will implement destruct");
		puts("   order using corbomite device recently installed.");
		if (!corbomite) {
			printf("   This will result in the destruction of the %s and\n",
			    sp->name);
			puts("   all matter within a 200 megameter diameter and");
			puts("   establish corresponding dead zone, all Federation");
			puts("   vessels will aviod this area for the next four solar");
			printf("   years.  Explosion will take place in one minute.  %s,\n",
			    captain);
			printf("   commanding %s, out.\n",sp->name);
		}
	}
	if (!corbomite) {
		printf("      Mr. %s.  Stand by.\n", helmsman);
		printf("%s:  Standing by.\n", helmsman);
		corbomite = 1;
	} else {
		printf("\n%s:  I don't believe that they will fall for that maneuver\n",
		    science);
		printf("   again, %s.\n", title);
	}
	return 0;
}

surrender_ship(sp)
struct ship *sp;
{
	printf("%s:   %s, open a channel to the %ss.\n", captain, com, foerace);
	printf("%s:   Aye, %s.\n", com, title);
	printf("%s:   This is Captain %s of the U.S.S. %s.  Will\n",
	    captain, captain, sp->name);
	puts("   you accept my unconditional surrender?");
	if (global & F_SURRENDER) {
		printf("%s:  %s, we have already surrendered.\n",
		    science, title);
		return 0;
	}
	if (surrender) {
		printf("%s:  The %ss have already refused.\n",science, foerace);
	} else {
		if (foerace == "Romulan") {
			printf("%s:  The %ss have not been know to have taken\n",
			    science, foerace);
			puts("   prisoners.");
		}
		surrender = 1;
	}
	return 0;
}

request_surrender(sp)
struct ship *sp;
{
	printf("%s:   %s, open a hailing frequency to the %ss.\n",
	    com, captain,foerace);
	printf("%s:  Aye, %s.\n", com, title);
	printf("%s:  This is Captain %s of the U. S. S. %s.  I give you\n",
	    captain, captain, sp->name);
	puts("   one last chance to surrender before we resume our attack.");
	if (global & E_SURRENDER) {
		printf("%s:  %s, we are already complying with your previous request!\n",
		    foename, captain);
		return 0;
	}
	if (surrenderp) {
		printf("%s:   %s, our offer has already been refused.\n",
		    science, title);
	} else {
		surrenderp = 1;
	}
	return 0;
}

self_destruct(sp)
struct ship *sp;
{
	printf("%s:   Lieutenant %s, tie in the bridge to the master\n",captain, com);
	printf("   computer.\n");
	if (is_dead(sp, S_COMP)) {
		printf("%s:  Our computer is down.\n", science);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s:  That program has been lost.  Restoring from backup.",
		    science);
		return 0;
	}
	printf("%s:  Aye, %s.\n",com, title);
	printf("%s:  Computer.  Destruct sequence.  Are you ready to copy?\n",captain);
	puts("Computer:  Working.");
	printf("%s:  Computer, this is Captain %s of the U.S.S. %s.\n",
	    captain, captain, sp->name);
	puts("   Destruct sequence one, code 1-1A.");
	puts("Computer:  Voice and code verified and correct.");
	puts("   Sequence one complete.");
	printf("%s:  This is Commander %s, Science Officer.  Destruct\n",
	    science, science);
	puts("   sequence two, code 1-1A-2B.");
	puts("Computer:  Voice and code verified and correct.  Sequence");
	puts("   two complete.");
	printf("%s:  This is Lieutenant Commander %s, Chief Engineering\n",
	    engineer, engineer);
	printf("   Officer of the U. S. S. %s.  Destruct sequence\n",sp->name);
	puts("   number three, code 1B-2B-3.");
	puts("Computer:  Voice and code verified and correct.");
	puts("   Destruct sequence complete and engaged.  Awaiting final");
	puts("   code for twenty second countdown.");
	printf("%s:  Computer, this is Captain %s of the U. S. S. %s.\n",
	    captain, captain, sp->name);
	puts("   begin countdown, code 0-0-0, destruct 0.");
	puts("Computer:  20 seconds to self-detruct.");
	sp->delay = 20.;
	return 0;
}

abort_self_destruct(sp)
struct ship *sp;
{
	printf("%s:   Computer, this is Captain %s of the U.S.S. %s.\n",
	    captain, captain, sp->name);
	puts("   Code 1-2-3 continuity abort destruct order, repeat:");
	puts("   Code 1-2-3 continuity abort destruct order!");
	if (is_dead(sp, S_COMP)) {
		printf("%s:  Our computer is down.\n", science);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s:  Temporary memory loss.  Unable to find program.\n",
		    science);
		return 0;
	}
	if (sp->delay > 1000.) {
		puts("Computer:  Self-destruct sequence has not been");
		puts("   initiated.");
		return 0;
	}
	printf("Computer:   Self-destruct order ... ");
	(void) fflush(stdout);
	sleep(4);
	if (sp->delay > 4.) {
		puts("aborted.  Destruct order aborted.");
		sp->delay = 10000.;
		return 0;
	} else {
		puts("cannot be aborted.");
		return 0;
	}
}

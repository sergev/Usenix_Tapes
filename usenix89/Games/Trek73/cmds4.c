/*
 * TREK73: cmds4.c
 *
 * User Commands
 *
 * alterpntparams, play_dead, corbomite_bluff, surrender_ship,
 * request_surrender, self_destruct, abort_self_destruct
 *
 */

#include "defines.h"
#include "structs.h"
#include <stdio.h>

extern	 int  defenseless;
extern   char captain[];
extern   char title[];
extern	 char science[];
extern	 char com[];
extern	 char nav[];
extern	 char helmsman[];
extern	 char engineer[];
extern   char foerace[];
extern   int  corbomite;
extern   int  surrender;
extern   int  surrenderp;

alterpntparams(sp)
struct ship *sp;
{
	int temp;
	char buf1[30];

	printf("\n%s:  Reset tubes, %s?\n",nav,title);
	printf("%s:  [yes or no] ",captain);
	if (Gets(buf1) == NULL)
		return 0;
	if ((buf1 != NULL) && (buf1[0] != 'n')) {
		printf("   Set launch speed to [0-12] ");
		if (Gets(buf1) == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < 13))
				sp->t_lspeed = temp;
		}
		printf("   ...time delay to [0-10] ");
		if (Gets(buf1) == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < 11))
				sp->t_delay = temp;
		}
		printf("   ...proximity delay to [0-500] ");
		if (Gets(buf1) == NULL)
			return 0;
		if (buf1 != NULL) {
			temp = atoi(buf1);
			if ((temp > -1) && (temp < 501))
				sp->t_prox = temp;
		}
	}
	printf("%s:  Reset phasers, %s?\n",nav ,title);
	printf("%s:  [yes or no] ",captain);
	if (Gets(buf1) == NULL)
		return 0;
	if ((buf1 != NULL) && (buf1[0] != 'n')) {
		printf("   Reset firing percentage to [0-100] ");
		if (Gets(buf1) == NULL)
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

	printf("%s:   Weapons officer, drop shields ...\n",captain);
	if (defenseless) {
		printf("%s:   %s, the %ss are not that stupid.",science, title,foerace);
		return 0;
	}
	printf("   Transfer power to [engines or phasers]: ");
	if (Gets(buf1) == NULL || *buf1 == NULL) {
		printf("%s:   What?\n", nav);
		return 0;
	}
	phaser_charge = -10.;
	if (buf1[0] != 'e') {
		phaser_charge = 10.;
		if (buf1[0] != 'p') {
			printf("%s:   What?\n", nav);
			return 0;
		}
	}
	for (i=0;i<4;i++) 
		sp->shields[i].attemp_drain = 0.;
	for (i=0;i<4;i++)
		sp->phasers[i].drain = phaser_charge;
	defenseless = 1;
	return 0;
}

corbomite_bluff(sp)
struct ship *sp;
{
	
	if (randm(2) == 1) {
		printf("%s:   Open a hailing frequency, ship-to-ship.\n",captain);
		printf("%s:  Hailing frequency open, %s.\n", com, title);
		printf("%s:   This is the Captain of the %s.  Our respect for\n",captain, sp->name);
		printf("   other life forms requires that we give you this warning--\n");
		printf("   one critical item of information which has never been\n");
		printf("   incorporated into the memory banks of any Earth ship.\n");
		printf("   Since the early years of space exploration, Earth vessels\n");
		printf("   have had incorporated into them a substance know as corbomite.\n");
		if (!corbomite) {
			printf("      It is a material and a device which prevents attack on\n");
			printf("   us.  If any destructive energy touchs our vessel, a re-\n");
			printf("   verse reaction of equal strength is created, destroying\n");
			printf("   the attacker.  It may interest you to know that, since\n");
			printf("   the initial use of corbomite for more than two of our\n");
			printf("   centuries ago, no attacking vessel has survived the attempt.\n");
			printf("   Death has little meaning to us.  If it has none to you,\n");
			printf("   then attack us now.  We grow annoyed with your foolishness.\n");
		}
	} else {
		printf("%s:   Open a special channel to Starfleet Command.\n",captain);
		printf("%s:   Aye, %s.\n",com, title);
		printf("%s:   Use Code 2.\n",captain);
		printf("%s:   but, Captain, according to our last Starfleet\n", com);
		printf("   Bulletin, the %ss have broken code 2.\n",foerace);
		printf("%s:   That's an order, Lieutenant.  Code 2!\n",captain);
		printf("%s:   Yes, Captain.  Code 2.\n", com);
		printf("%s:   Message from %s to Starfleet Command, this sector.\n",captain,sp->name);
		printf("   have inadvertantly encroached upon %s neutral zone,\n",foerace);
		printf("   surrounded and under heavy %s attack.  Escape\n",foerace);
		printf("   impossible.  Shields failing.  Will implement destruct\n");
		printf("   order using corbomite device recently installed.\n");
		if (!corbomite) {
			printf("   This will result in the destruction of the %s and\n",sp->name);
			printf("   all matter within a 200 megameter diameter and\n");
			printf("   establish corresponding dead zone, all federation\n");
			printf("   vessels will aviod this area for the next four solar\n");
			printf("   years.  Explosion will take place in one minute.  %s,\n",captain);
			printf("   commanding %s, out.\n",sp->name);
		}
	}
	if (!corbomite) {
		printf("      Mr. %s.  Stand by.\n", helmsman);
		printf("%s:  Standing by.\n", helmsman);
		corbomite = 1;
	} else {
		printf("\n%s:  I don't believe that they will fall for tha maneuver\n", science);
		printf("   again, %s.\n",title);
	}
	return 0;
}

surrender_ship(sp)
struct ship *sp;
{
	printf("%s:   Lieutenant, open a channel to the %ss.\n", captain, foerace);
	printf("%s:   Aye, %s.\n", com, title);
	printf("%s:   This is Captain %s of the U. S. S. %s.  Will\n",captain,captain,sp->name);
	printf("   you accept my unconditional surrender?\n");
	if (surrender) {
		printf("%s:  The %ss have already refused.\n",science, foerace);
	} else {
		if (foerace == "Romulan") {
			printf("%s:  The %ss have not been know to have taken\n",science, foerace);
			printf("   prisoners.\n");
		}
		surrender = 1;
	}
	return 0;
}

request_surrender(sp)
struct ship *sp;
{
	printf("%s:   Lieutenant, open a hailing frequency to the %ss.\n",captain,foerace);
	printf("%s:  Aye, %s.\n", com, title);
	printf("%s:  This is Captain %s of the U. S. S. %s.  I give you\n",captain, captain, sp->name);
	printf("   one last chance to surrender before we resume our attack.\n");
	if (surrenderp) {
		printf("%s:   %s, our offer has already been refused.\n",science, title);
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
	if (sp->status & S_COMP) {
		printf("%s:  Our computer is down.\n", science);
		return 0;
	}
	printf("%s:   Aye, %s.\n",com, title);
	printf("%s:  Computer.  Destruct sequence.  Are you ready to copy?\n",captain);
	printf("Computer:  Working.\n");
	printf("%s:  Computer, this is Captain %s of the U. S. S. %s.\n",captain,captain,sp->name);
	printf("   destruct sequence one, code 1-1a.\n");
	printf("Computer:  Voice and code verified and correct.\n");
	printf("   Sequence one complete.\n");
	printf("%s:  This is Commander %s, Science Officer.  Destruct\n", science, science);
	printf("   sequence two, code 1-1a-2b.\n");
	printf("Computer:  Voice and code verified and correct.  Sequence\n");
	printf("   two complete.\n");
	printf("%s:  This is Lieutenant Commander %s, Chief Engineering\n", engineer, engineer);
	printf("   Officer of the U. S. S. %s.  Destruct sequence\n",sp->name);
	printf("   number three, code 1-b2-b3.\n");
	printf("Computer:  Voice and code verified and correct.\n");
	printf("   Destruct sequence complete and engaged.  Awaiting final\n");
	printf("   code for twenty second countdown.\n");
	printf("%s:  Computer, this is Captain %s of the U. S. S. %s.\n",captain, captain, sp->name);
	printf("   begin countdown, code 0-0-0, destruct 0.\n");
	printf("Computer:  20 seconds to self-detruct.\n");
	sp->delay = 9 * 20;
return 0;
}

abort_self_destruct(sp)
struct ship *sp;
{
	printf("%s:   Computer, this is Captain %s of the U. S. S. %s.\n",captain,captain,sp->name);
	printf("   Code 1-2-3 continuity abort destruct order, repeat:\n");
	printf("   Code 1-2-3 continuity abort destruct order!\n");
	if (sp->status & S_COMP) {
		printf("%s:  Our computer is down.\n", science);
		return 0;
	}
	if (sp->delay > 1000) {
		printf("Computer:  Self-destruct sequence has not been\n");
		printf("   initiated.\n");
		return 0;
	}
	printf("Computer:   Self-destruct order ...");
	fflush(stdout);
	sleep(4);
	if (sp->delay > 4 * 9) {
		printf("aborted.  Destruct order aborted.\n");
		sp->delay = 10000;
		return 0;
	} else {
		printf("\n%s:  Too late, captain ...\n", science);
		return 0;
	}
}

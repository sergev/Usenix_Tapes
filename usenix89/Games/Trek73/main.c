/*
 * TREK73: main.c
 *
 * Originally written (in HP-2000 BASIC) by
 *	William K. Char, Perry Lee, and Dan Gee
 *
 * Rewritten in C by
 *	Dave Pare (sdcsvax!sdamos!mr-frog)
 *		and
 *	Christopher Williams (ucbvax!ucbmerlin!williams)
 *
 * Corrected, Completed, and Enhanced by
 *	Jeff Okamoto	(ucbvax!okamoto)
 *	Peter Yee	(ucbvax!yee)
 *	Matt Dillon	(ucbvax!dillon)
 *	Dave Sharnoff	(ucbvax!ucbcory!muir)
 *		and
 *	Joel Duisman	(ucbvax!duisman)
 *
 * Main Loop
 *
 * main, alarmtrap
 *
 */

#include "defines.h"
#include "structs.h"
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

int	timeout;
jmp_buf	jumpbuf;
extern	char shutup[];


main()
{
	extern	char **environ;
	extern	struct list *newitem();
	extern	char captain[];
	extern	char title[];
	extern	char foename[];
	extern	char *foeraces[];
	extern	char *foecaps[];
	extern	char *foeshiptype[];
	extern	char foerace[];
	extern	char foestype[];
	extern	int terse;
	extern	int silly;
	extern	char *feds[];
	extern	char *baddies[MAXFOERACES][MAXBADS];
	extern	struct cmd cmds[];
	extern	struct ship *shiplist[];
	extern	int shipnum;
	extern 	char shipname[];
	extern  char slots[];
	struct	cmd *scancmd();
	int	alarmtrap();
	register int i;
	register int j;
	register struct ship *sp;
	register struct list *lp;
	char	buf1[30];
	struct	cmd *cp;
	int	range;
	float	bearing;
	int	loop;
	int	len;
	int	enemynum;
	extern	char slots[];
	char	*tmp;
	int	swap1;
	int	swap2;
	int	offset;
	extern	int (*strategies[])();
	extern	char *options;
	extern	char *getenv();
	extern	char sex[];
	extern	char shipbuf[];
	extern	char science[];
	extern 	char engineer[];
	extern	char com[];
	extern	char nav[];
	extern	char helmsman[];
	extern	char racename[];
	extern	int init_p_turn[];
	extern	int init_t_turn[];
	extern	char *foeempire[];
	extern	char empire[];

	signal(SIGALRM, alarmtrap);
	signal(SIGINT, SIG_IGN);
	srandom(time(0));
	options = getenv("TREK73OPTS");
	if (options != NULL) {
		parse_opts(options);
	}
	if (strlen(science) == 0)
		strcpy(science, "Spock");
	if (strlen(engineer) == 0)
		strcpy(engineer, "Scott");
	if (strlen(com) == 0)
		strcpy(com, "Uhura");
	if (strlen(nav) == 0)
		strcpy(nav, "Chekov");
	if (strlen(helmsman) == 0)
		strcpy(helmsman, "Sulu");
	if (strlen(captain) == 0) {
		printf("\n\nCaptain: my last name is ");
		if (gets(buf1) == NULL || *buf1 == NULL)
			exit(1);
		strcpy (captain, buf1);
	}
	if (*captain == '*') {
		terse = 1;
		len = strlen(captain) + 1;
		for (loop = 1; loop < len; loop++)
			captain[loop-1] = captain[loop];
	}
	if (strlen(sex) != 0)
		strcpy(buf1,sex);
	else {
		printf("%s: My sex is: ",captain);
		if (gets(buf1) == NULL || *buf1 == NULL)
			exit(1);
	}
	if ((*buf1 <= 'z') && (*buf1 >= 'a'))
		*buf1 = *buf1 - 'a' + 'A';
	switch(*buf1) {
	case 'M':
		strcpy(title, "Sir");
		break;
	case 'F':
		strcpy(title, "Ma'am");
		break;
	default :
		switch (random() % 6) {
		case 0:
			strcpy(title, "Fag");
			break;
		case 1:
			strcpy(title, "Fairy");
			break;
		case 2:
			strcpy(title, "Fruit");
			break;
		case 3:
			strcpy(title, "Weirdo");
			break;
		case 4:
			strcpy(title, "Gumby");
			break;
		case 5:
			strcpy(title, "Freak");
			break;
		}
	}
	if (strlen(shipbuf) != 0) {
		strcpy(buf1,shipbuf);
	} else {
	    getships:
		printf("   I'm expecting [1-9] enemy vessels ");
		if (gets(buf1) == NULL || *buf1 == NULL)
			exit(1);
	}
	i = atoi(buf1);
	if (i < 1 || i > 9) {
		printf("%s:   %s, Starfleet Command reports that it can only\n", com, title);
		printf("   be from 1 to 9.  Try again.\n");
		printf("%s:  Correct, Lieutenant -- just testing your attention..\n", captain);
		goto getships;
	}
	shipnum = i;
	for (loop = 0; loop < shipnum; loop++);
		slots[loop] = 'X';
	if (strlen(racename) == 0) {
		if (silly == 0)
			offset = 1;
		else
			offset = 0;
		enemynum = randm(MAXFOERACES - offset) - 1;
	} else {
		for (loop=0; loop<MAXFOERACES; loop++)
			if (strncmp(racename, foeraces[loop], strlen(racename)) == 0) {
				enemynum = loop;
				break;
			}
		if (loop == MAXFOERACES) {
			printf("Cannot find race %s.\n", racename);
			enemynum = randm(MAXFOERACES) - 1;
		}
	}
	strcpy(foerace, foeraces[enemynum]);
	strcpy(foestype, foeshiptype[enemynum]);
	strcpy(empire, foeempire[enemynum]);
	if (strlen(foename) == 0)
		strcpy(foename, foecaps[randm(MAXENCOMM) - 1]);
	/*
	 * Randomize the enemy ships 
	 */
	for (loop = 0; loop < 20; loop++) {
		swap1 = randm(MAXBADS) - 1;
		swap2 = randm(MAXBADS) - 1;
		tmp = baddies[enemynum][swap1];
		baddies[enemynum][swap1] = baddies[enemynum][swap2];
		baddies[enemynum][swap2] = tmp;
	}
	/*
	 * everybody is centered on the federation ship
	 * (for now, anyways)
	 */
	for (i=0; i<=shipnum; i++) {
		lp = newitem(I_SHIP);
		lp->data.sp = MKNODE(struct ship, *, 1);
		sp = shiplist[i] = lp->data.sp;
		if (i)
			strcpy(sp->name, baddies[enemynum][i-1]);
		sp->warp = sp->newwarp = 1.0;
		sp->course = sp->newcourse = randm(360);
		sp->eff = .75;
		sp->regen = 10.0;
		sp->energy = 150;
		sp->pods = 200;
		sp->id = i;
		for (j=0; j<4; j++) {
			sp->phasers[j].target = NULL;
			sp->phasers[j].bearing = init_p_turn[j];
			sp->phasers[j].load = 10;
			sp->phasers[j].drain = 10;
			sp->phasers[j].status = P_NORMAL;
		}
		for (j=0; j<4; j++) {
			sp->shields[j].eff = 1.0;
			sp->shields[j].drain = 0.0;
			sp->shields[j].attemp_drain = 1.0;
		}
		sp->p_spread = 10;
		for (j=0; j<6; j++) {
			sp->tubes[j].target = NULL;
			sp->tubes[j].bearing = init_t_turn[j];
			sp->tubes[j].load = 0;
			sp->tubes[j].status = T_NORMAL;
		}
		sp->t_lspeed = 12;
		sp->t_prox = 200;
		sp->t_delay = 10;
		sp->p_percent = 100;
		sp->status = S_NORMAL;		/* all is well */
		sp->target = NULL;
		sp->eluding = 0;
		sp->delay = 10000;
		range = 4100 + randm(300) - i * 200;
		bearing = toradians(randm(360));
		sp->x = range * cos(bearing);
		sp->y = range * sin(bearing);
		sp->crew = 350;
		sp->strategy = strategies[0];
	}
	/*
	 * federation exceptions
	 */
	sp = shiplist[0];
	sp->course = sp->newcourse = 0;
	sp->eff = 1.0;
	sp->x = sp->y = 0;
	sp->crew = 450;
	if (strlen(shipname) == 0) {
		i = randm(MAXFEDS) - 1;
		strcpy(sp->name, feds[i]);
	} else {
		strcpy(sp->name, shipname);
	}
	for(loop=shipnum+1; loop<300; loop++)
		slots[loop] = ' ';
	mission();
	warning();
	setjmp(jumpbuf);
	timeout = 0;
	signal(SIGALRM, alarmtrap);
	alarm(0);
	for (;;) {
		sp = shiplist[0];
		if (!(sp->status & S_DEAD)) {
			alarm(20);
			for (loop = 0; loop < HIGHSHUTUP; loop++)
				shutup[loop] = 0;
			printf("\n%s: Code [1-30] ", captain);
			if (gets(buf1) != NULL) {
				cp = scancmd(buf1);
				if (cp != NULL) {
					(*cp->routine)(sp);
					if (!cp->turns)
						continue;
				} else
					printf("\n%s: What??\n", science);
			}
		}
		alarm(0);
		alarmtrap(0);
	}
}

alarmtrap(sig)
int sig;
{
	extern	int timeout;
	extern	jmp_buf jumpbuf;

	if (sig) {
		printf("\n** TIME **\n");
		stdin->_cnt = 0;
	}
	if (!(shiplist[0]->status & S_DEAD))
		printf("\n");
	shiplist[1]->strategy();
	move_ships();
	if (sig) {
		timeout = 1;
		longjmp(jumpbuf, 1);
	}
}

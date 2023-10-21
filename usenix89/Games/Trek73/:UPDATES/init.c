/*
 * TREK73: init.c
 *
 * Game initialization routines
 *
 * name_crew, init_ships
 *
 */

#include "externs.h"
#include <fcntl.h>

name_crew()
{
	char buf1[30];
	int loop;
	int len;

	if (com_delay[0] != 0)
		time_delay = atoi(com_delay);
	if (science[0] == '\0')
		(void) strcpy(science, "Spock");
	if (engineer[0] == '\0')
		(void) strcpy(engineer, "Scott");
	if (com[0] == '\0')
		(void) strcpy(com, "Uhura");
	if (nav[0] == '\0')
		(void) strcpy(nav, "Chekov");
	if (helmsman[0] == '\0')
		(void) strcpy(helmsman, "Sulu");
	if (captain[0] == '\0') {
		printf("\n\nCaptain: my last name is ");
		if (gets(buf1) == NULL || *buf1 == '\0')
			exit(1);
		(void) strncpy (captain, buf1, sizeof captain);
		captain[sizeof captain - 1] = '\0';
	}
	if (captain[0] == '*') {
		terse = 1;
		len = strlen(captain) + 1;
		for (loop = 1; loop <= len; loop++)
			captain[loop-1] = captain[loop];
	}
	if (sex[0] != '\0') {
		(void) strncpy(buf1, sex, sizeof sex);
		sex[sizeof sex - 1] = '\0';
	} else {
		printf("%s: My sex is: ",captain);
		if (gets(buf1) == NULL || *buf1 == '\0')
			exit(1);
	}
	switch(*buf1) {
	case 'M':
	case 'm':
		(void) strcpy(title, "Sir");
		break;
	case 'F':
	case 'f':
		(void) strcpy(title, "Ma'am");
		break;
	default :
		switch ((int)(random() % 6)) {
		case 0:
			(void) strcpy(title, "Fag");
			break;
		case 1:
			(void) strcpy(title, "Fairy");
			break;
		case 2:
			(void) strcpy(title, "Fruit");
			break;
		case 3:
			(void) strcpy(title, "Weirdo");
			break;
		case 4:
			(void) strcpy(title, "Gumby");
			break;
		case 5:
			(void) strcpy(title, "Freak");
			break;
		}
	}
}


init_ships()
{
	struct ship_stat *my_class, *his_class, *ship_class(), *read_class();
	int myread, hisread;	/* Did we read from file? */
	register int i;
	register int j;
	register struct ship *sp;
	register struct list *lp;
	int range;
	float bear;
	char *tmp;
	int swap1, swap2;
	int offset;
	int loop;
	char buf1[30];

getships:
	printf("   I'm expecting [1-9] enemy vessels ");
	if (gets(buf1) == NULL || *buf1 == NULL) {
		printf("%s:  %s, Starfleet Command reports that you have been\n",
		    com, title);
		puts("   relieved of command for dereliction of duty.");
		exit(1);
	}
	if (buf1[0] != '\0') {
		i = atoi(buf1);
		if (i < 1 || i > MAXESHIPS) {
			printf("%s:   %s, Starfleet Command reports that it can only\n",
			    com, title);
			printf("   be from 1 to 9.  Try again.\n");
			printf("%s:  Correct, Lieutenant -- just testing your attention..\n",
			    captain);
			goto getships;
		}
		shipnum = i;
	} else
		shipnum = randm(MAXESHIPS);
	for (loop = 0; loop < shipnum; loop++);
		slots[loop] = 'X';
	offset = !silly;
	if (racename[0] == '\0') {
		enemynum = randm(MAXFOERACES - offset) - 1;
	} else {
		for (loop=0; loop<MAXFOERACES; loop++)
			if (!strncmp(racename, aliens[loop].race_name, strlen(racename))) {
				enemynum = loop;
				break;
			}
		if (loop == MAXFOERACES) {
			printf("Cannot find race %s.\n", racename);
			enemynum = randm(MAXFOERACES - offset) - 1;
		}
	}
	if (class[0] == '\0')
		(void) strcpy(class, "CA");
	if (foeclass[0] == '\0')
		(void) strcpy(foeclass, "CA");
	myread = 0;
	hisread = 0;
	if ((my_class = ship_class(class)) == NULL)
		if ((my_class = read_class(class, 0)) == NULL)
			my_class = ship_class("CA");
		else
			myread = 1;
	if ((his_class = ship_class(foeclass)) == NULL)
		if ((his_class = read_class(foeclass, 1)) == NULL)
			his_class = ship_class("CA");
		else {
			hisread = 1;
			enemynum = randm(MAXFOERACES-1);
		}
	if (!hisread) {
		(void) strncpy(foerace, aliens[enemynum].race_name,
		    sizeof foerace);
		(void) strncpy(foestype,
		    aliens[enemynum].ship_types[his_class->class_num],
		    sizeof foestype);
		(void) strncpy(empire, aliens[enemynum].empire_name,
		    sizeof empire);
	}
	while (foename[0] == '\0')
		(void) strncpy(foename,
		    aliens[enemynum].captains[randm(MAXECAPS) - 1],
		    sizeof foename);
	foerace[sizeof foerace - 1] = '\0';
	foestype[sizeof foestype - 1] = '\0';
	empire[sizeof empire - 1] = '\0';
	foename[sizeof foename - 1] = '\0';
	/*
	 * Randomize the enemy ships 
	 */
	for (loop = 0; loop < 20; loop++) {
		swap1 = randm(MAXESHIPS) - 1;
		swap2 = randm(MAXESHIPS) - 1;
		tmp = aliens[enemynum].ship_names[swap1];
		aliens[enemynum].ship_names[swap1] = aliens[enemynum].ship_names[swap2];
		aliens[enemynum].ship_names[swap2] = tmp;
	}
	/*
	 * Everybody is centered on the Federation ship
	 * (for now, anyways)
	 */
	for (i=0; i<=shipnum; i++) {
		lp = newitem(I_SHIP);
		lp->data.sp = MKNODE(struct ship, *, 1);
		sp = shiplist[i] = lp->data.sp;
		if (sp == (struct ship *)NULL) {
			fprintf(stderr, "init_ships: malloc failed\n");
			exit(2);
		}
		if (i)
			(void) strncpy(sp->name,
			    aliens[enemynum].ship_names[i-1],
			    sizeof sp->name);
		(void) strncpy(sp->class, his_class->abbr,
		    sizeof sp->class);
		sp->warp = sp->newwarp = 1.0;
		sp->course = sp->newcourse = (float)randm(360);
		sp->eff = his_class->e_eff;
		sp->regen = his_class->regen;
		sp->energy = his_class->energy;
		sp->pods = his_class->pods;
		sp->id = i;
		sp->p_spread = INIT_P_SPREAD;
		sp->num_phasers = his_class->num_phaser;
		sp->num_tubes = his_class->num_torp;
		sp->max_speed = his_class->e_warpmax;
		sp->orig_max = his_class->e_warpmax;
		sp->deg_turn = his_class->turn_rate;
		sp->ph_damage = his_class->ph_shield;
		sp->tu_damage = his_class->tp_shield;
		sp->p_blind_left = his_class->p_blind_left;
		sp->p_blind_right = his_class->p_blind_right;
		sp->t_blind_left = his_class->t_blind_left;
		sp->t_blind_right = his_class->t_blind_right;
		for (j=0; j<SHIELDS; j++) {
			sp->shields[j].eff = 1.0;
			sp->shields[j].drain = 1.0;
			sp->shields[j].attemp_drain = 1.0;
		}
		for (j=0; j<sp->num_phasers; j++) {
			sp->phasers[j].target = NULL;
			sp->phasers[j].bearing = init_p_turn[sp->num_phasers][j];
			sp->phasers[j].load = INIT_P_LOAD;
			sp->phasers[j].drain = INIT_P_DRAIN;
			sp->phasers[j].status = P_NORMAL;
		}
		for (j=0; j<sp->num_tubes; j++) {
			sp->tubes[j].target = NULL;
			sp->tubes[j].bearing = init_t_turn[sp->num_tubes][j];
			sp->tubes[j].load = INIT_T_LOAD;
			sp->tubes[j].status = T_NORMAL;
		}
		sp->t_lspeed = INIT_T_SPEED;
		sp->t_prox = INIT_T_PROX;
		sp->t_delay = INIT_T_TIME;
		sp->p_percent = INIT_P_PERCENT;
		for (j=0; j<MAXSYSTEMS; j++)	/* Everything is OK */
			sp->status[j] = 0;
		sp->target = NULL;
		sp->relbear = 0.0;
		sp->delay = 10000.;
		range = 4100 + randm(300) - i * 200;
		bear = toradians(randm(360));
		sp->x = range * cos(bear);
		sp->y = range * sin(bear);
		sp->complement = his_class->e_crew;
		sp->strategy = strategies[0];
		if (!strcmp(foerace, "Romulan") || can_cloak)
			sp->cloaking = C_OFF;
		else
			sp->cloaking = C_NONE;
		sp->cloak_energy = his_class->cloaking_energy;
		sp->cloak_delay = CLOAK_DELAY;
		sp->position.x = 0;
		sp->position.y = 0;
		sp->position.warp = 0.0;
		sp->position.range = 0;
		sp->position.bearing = 0.0;
		sp->position.course = 0.0;
		sp->p_firing_delay = his_class->p_firing_delay;
		sp->t_firing_delay = his_class->t_firing_delay;
	}
	/*
	 * federation exceptions
	 */
	sp = shiplist[0];
	(void) strcpy(sp->class, my_class->abbr);
	sp->course = sp->newcourse = 0.0;
	sp->x = sp->y = 0;
	sp->eff = my_class->o_eff;
	sp->regen = my_class->regen;
	sp->energy = my_class->energy;
	sp->pods = my_class->pods;
	sp->complement = my_class->o_crew;
	sp->num_phasers = my_class->num_phaser;
	sp->num_tubes = my_class->num_torp;
	sp->max_speed = my_class->o_warpmax;
	sp->orig_max = my_class->o_warpmax;
	sp->deg_turn = my_class->turn_rate;
	sp->ph_damage = my_class->ph_shield;
	sp->tu_damage = my_class->tp_shield;
	sp->cloaking = C_NONE;
	for (j=0; j<sp->num_phasers; j++) {
		sp->phasers[j].target = NULL;
		sp->phasers[j].bearing = init_p_turn[sp->num_phasers][j];
		sp->phasers[j].load = INIT_P_LOAD;
		sp->phasers[j].drain = INIT_P_DRAIN;
		sp->phasers[j].status = P_NORMAL;
	}
	for (j=0; j<sp->num_tubes; j++) {
		sp->tubes[j].target = NULL;
		sp->tubes[j].bearing = init_t_turn[sp->num_tubes][j];
		sp->tubes[j].load = INIT_T_LOAD;
		sp->tubes[j].status = T_NORMAL;
	}
	sp->p_firing_delay = my_class->p_firing_delay;
	sp->t_firing_delay = my_class->t_firing_delay;
	if (strlen(shipname) == 0) {
		i = randm(MAXFEDS) - 1;
		(void) strcpy(sp->name, feds[i]);
		(void) strcpy(shipname, sp->name);
	} else {
		(void) strcpy(sp->name, shipname);
	}
	for(loop=shipnum + 1; loop < HIGHSLOT; loop++)
		slots[loop] = ' ';
}

struct ship_stat *read_class(str, flag)
char *str;
int flag;
{
	int fd, bytes;
	char path[BUFSIZ];

	sprintf(path, "%s.trek%s", home, str);
	if ((fd = open(path, O_RDONLY, 0)) < 0) {
		printf("Could not find file %s\n", path);
		return(NULL);
	}
	switch (flag) {
	case 0:
		if ((bytes = read(fd, &us, sizeof(us))) != sizeof(us)) {
			puts("Bad format in file");
			return(NULL);
		}
		return(&us);
		break;
	case 1:
		if ((bytes = read(fd, &them, sizeof(them)))
		    != sizeof(them)) {
			puts("Bad format in file");
			return(NULL);
		}
		if ((bytes = read(fd, foestype, 30)) < 0)
			return(NULL);
		if ((bytes = read(fd, foerace, 30)) < 0)
			return(NULL);
		if ((bytes = read(fd, empire, 30)) < 0)
			return(NULL);
		if ((bytes = read(fd, &can_cloak, 1)) < 0)
			return(NULL);
		if ((bytes = read(fd, &e_bpv, sizeof(int))) < 0)
			return(NULL);

		return(&them);
		break;
		
	}
}

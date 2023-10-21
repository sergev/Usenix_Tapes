/*
 * TREK73: cmds1.c
 *
 * User Commands
 *
 * fire_phasers, fire_tubes, lock_phasers, lock_tubes
 * turn_phasers, turn_tubes, load_tubes, launch_probe,
 * probe_control
 *
 */

#include "defines.h"
#include "structs.h"

extern	char title[];
extern	char science[];
extern	char engineer[];
extern  char nav[];
extern	char helmsman[];

fire_phasers(sp) 
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	char	c;
	int	typed[4];
	register int i;
	register int k;

	for (i=0; i<4; i++)
		typed[i] = 0;
	printf("   fire phasers [1-4] ");
	if (Gets(buf1) == NULL)
		return 0;
	printf("   spread [10-45] ");
	if (Gets(buf2) == NULL)
		return 0;
	i = atoi(buf2);
	if (i < 10 || i > 45)
		return 0;
	sp->p_spread = i;
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 3)
			continue;
		typed[k]++;
		if ((sp->phasers[k].status & P_DAMAGED) ||
			(sp->phasers[k].status) & P_FIRING)
			continue;
		sp->phasers[k].status |= P_FIRING;
	}
	check_p_damage(typed, sp, "fire");	/* Type out if damaged */
	check_p_turn(typed, sp, 1);
	return 1;
}


fire_tubes(sp)
struct ship *sp;
{
	char	buf1[20];
	char	c;
	int	typed[6];
	register int i;
	register int j;
	register int k;

	for (i=0; i<6; i++)
		typed[i] = 0;
	printf("   fire tubes [1-6] ");
	if (Gets(buf1) == NULL)
		return 0;
	j = strlen(buf1);
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 5)
			continue;
		typed[k]++;
		if ((sp->tubes[k].status & T_DAMAGED) ||
			(sp->tubes[k].status & T_FIRING))
			continue;
		sp->tubes[k].status |= T_FIRING;
	}
	check_t_damage(typed, sp, "fire");	/* Type if damaged */
	check_t_turn(typed, sp, 1);
	j = 0;
	for (i=0; i<6; i++) {
		if ((typed[i] == 0) || (!(sp->tubes[i].status & T_FIRING)))
			continue;
		if (sp->tubes[i].load == 0) {
			if (!j)
				printf("Computer: Tube(s) %d", i + 1);
			else
				printf(", %d", i + 1);
			j++;
		}
	}
	if (j)
		printf(" have no charge in them.\n");
	return 0;
}


lock_phasers(sp)
struct ship *sp;
{
	extern	struct ship *ship_name();
	char	buf1[20];
	char	buf2[20];
	int	typed[4];
	char	c;
	struct	ship *ep;
	register int i;
	register int k;

	for (i=0; i<4; i++)
		typed[i] = 0;
	if (sp->status & S_COMP) {
		printf("%s:  Impossible %s, our computer is dead.\n", science, title);
		return 0;
	}
	printf("   lock phasers [1-4] ");
	if (Gets(buf1) == NULL)
		return 0;
	printf("   onto whom ");
	if (Gets(buf2) == NULL)
		return 0;
	ep = ship_name(buf2,ENEMYONLY);
	if (ep == NULL)
		return 0;
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 3)
			continue;
		typed[k]++;
		if (sp->phasers[k].status & P_DAMAGED)
			continue;
		sp->phasers[k].target = ep;
	}
	check_p_damage(typed, sp, "lock");
	check_p_turn(typed, sp, 0);
	return 1;
}


lock_tubes(sp)
struct ship *sp;
{
	extern	struct ship *ship_name();
	char	buf1[20];
	char	buf2[20];
	int	typed[6];
	char	c;
	struct	ship *ep;
	register int i;
	register int k;

	for (i=0; i<6; i++)
		typed[i] = 0;
	if (sp->status & S_COMP) {
		printf("%s:  Impossible %s, our computer is dead.\n", science, title);
		return 0;
	}
	printf("   lock tubes [1-6] ");
	if (Gets(buf1) == NULL)
		return 0;
	printf("   onto whom ");
	if (Gets(buf2) == NULL)
		return 0;
	ep = ship_name(buf2,ENEMYONLY);
	if (ep == NULL)
		return 0;
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 5)
			continue;
		typed[k]++;
		if (sp->tubes[k].status & T_DAMAGED)
			continue;
		sp->tubes[k].target = ep;
	}
	check_t_damage(typed, sp, "lock");
	check_t_turn(typed, sp, 0);
	return 1;
}


turn_phasers(sp)
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	char	c;
	int	typed[4];
	register int i;
	register int j;
	register int k;

	for (i=0; i<4; i++)
		typed[i] = 0;
	printf("   turn phasers [1-4] ");
	if (Gets(buf1) == NULL)
		return 0;
	printf("   to [0-360] ");
	if (Gets(buf2) == NULL)
		return 0;
	j = atoi(buf2);
	if (j < 0 || j > 360)
		return 0;
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 3)
			continue;
		typed[k]++;
		if (sp->phasers[k].status & P_DAMAGED)
			continue;
		sp->phasers[k].target = NULL;
		sp->phasers[k].bearing = j;
	}
	check_p_damage(typed, sp, "turn");
	check_p_turn(typed, sp, 0);
	return 1;
}


turn_tubes(sp)
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	char	c;
	int	typed[6];
	register int i;
	register int j;
	register int k;

	for (i=0; i<6; i++)
		typed[i] = 0;
	printf("   turn tubes [1-6] ");
	if (Gets(buf1) == NULL)
		return 0;
	printf("   to [0-360] ");
	if (Gets(buf2) == NULL)
		return 0;
	j = atoi(buf2);
	if (j < 0 || j > 360)
		return 0;
	for (i=0; c = buf1[i]; i++) {
		k = c - '1';
		if (k < 0 || k > 5)
			continue;
		typed[k]++;
		if (sp->tubes[k].status & T_DAMAGED)
			continue;
		sp->tubes[k].target = NULL;
		sp->tubes[k].bearing = j;
	}
	check_t_damage(typed, sp, "turn");
	check_t_turn(typed, sp, 0);
	return 1;
}


load_tubes(sp)
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	char	c;
	int	load;
	struct	tube *tp;
	register int i;
	register int j;
	register int k;

	load = 0;
	printf("   [load or unload] ");
	if (Gets(buf1) == NULL)
		return 0;
	if (*buf1 == 'l' || *buf1 == 'L')
		load++;
	else if (*buf1 != 'u' && *buf1 != 'U')
		return 0;
	printf("   tubes [1-6] ");
	if (Gets(buf2) == NULL)
		return 0;
	for (i=0; c = buf2[i]; i++) {
		if (sp->energy <= 0)
			break;
		k = c - '1';
		if (k < 0 || k > 5)
			continue;
		tp = &sp->tubes[k];
		if (tp->status & T_DAMAGED)
			continue;
		if (load) {
			j = min(sp->energy, 10-tp->load);
			if (j == 0)
				continue;
			sp->energy -= j;
			sp->pods -= j;
			tp->load += j;
		} else {
			j = tp->load;
			if (j == 0)
				continue;
			sp->energy += j;
			sp->pods += j;
			tp->load = 0;
		}
	}
	printf("%s: tubes now ", engineer);
	for (i=0; i<6; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			printf(" -- ");
		else
			printf(" %-2d ", sp->tubes[i].load);
	}
	printf(" pods at %d\n", sp->pods);
	return 1;
}


launch_probe(sp)
struct ship *sp;
{
	extern	char captain[];
	extern	struct list *newitem();
	char	buf1[20];
	int	pods, delay, prox, course;
	struct	ship *target;
	struct	list *lp;
	struct	torpedo *pp;

	pods = delay = prox = course = 0;
	target = NULL;
	if (sp->status & S_PROBE) {
		printf("%s: probe launcher destroyed!\n", engineer);
		return 0;
	}
	if (sp->energy < 10) {
		printf("%s: we've not enough power, Captain.\n", engineer);
		return 0;
	}
	printf("%s: %d pods available.\n", engineer, sp->energy);
	printf("%s: number to launch [10+] is ", captain);
	if (Gets(buf1) == NULL)
		return 0;
	pods = atoi(buf1);
	if (pods < 10 || pods > sp->energy)
		return 0;
	printf("   set time delay to [0-15] ");
	if (Gets(buf1) == NULL)
		return 0;
	delay = atoi(buf1);
	if (delay < 0 || delay > 15)
		return 0;
	printf("   set proximity delay to [50+] ");
	if (Gets(buf1) == NULL)
		return 0;
	prox = atoi(buf1);
	if (prox < 50)
		return 0;
	printf("   launch towards [whom, if anyone] ");
	Gets(buf1);
	/*
	 * This must be fixed in the near future...
	 */
	if (*buf1) {
		target = ship_name(buf1,ENEMYONLY);
		if (target == NULL)
			return 0;
	} else {
		printf("   course [0-360] ");
		if (gets(buf1) == NULL)
			return 0;
		course = atoi(buf1);
		if (course < 0 || course > 360)
			return 0;
		target = NULL;
	}
	/*
	 * add a new item to the list of items in space
	 */
	lp = newitem(I_PROBE);
	lp->data.tp = MKNODE(struct torpedo, *, 1);
	pp = lp->data.tp;
	pp->from = sp;
	pp->fuel = pods;
	pp->timedelay = delay * 10;
	pp->speed = sp->warp;
	pp->newspeed = 2.0;
	pp->prox = prox;
	pp->target = target;
	pp->course = course;
	pp->x = sp->x;
	pp->y = sp->y;
	pp->id = new_slot();
	/*
	 * subtract the pods used
	 */
	sp->pods -= pods;
	sp->energy -= pods;
	printf("%s: probe %d away\n",engineer, pp->id);
	return 1;
}


probe_control(sp)
struct ship *sp;
{
	extern	int rangefind();
	extern	int bearing();
	extern	struct list head;
	extern	struct list *tail;
	register struct list *lp;
	register int i;
	register int j;
	register struct torpedo *pp;
	struct	torpedo *probes[10];
	int	probenum;
	struct	ship *ep;
	int	pnum;
	int	bear;
	int	range;
	char	buf1[20];
	char	*bp;

	pnum = 0;
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_PROBE)
			continue;
		pp = lp->data.tp;
		if (pp->from != sp)
			continue;
		if (!pnum)
			printf("\nprobe bearng range course time  prox units target\n");
		probes[pnum] = pp;
		pnum++;
		range = rangefind(sp->x, pp->x, sp->y, pp->y);
		bear = bearing(sp->x, pp->x, sp->y, pp->y);
		if (pp->target == NULL)
			bp = "NONE";
		else
			bp = pp->target->name;
		printf(" %2d    %3d   %5d  %3d    %2d  %5d  %3d  %s\n",
			pp->id, bear, range, pp->course, pp->timedelay,
			pp->prox, pp->fuel, bp);
	}
	if (!pnum) {
		printf("%s: what probes?\n", nav);
		return 0;
	}
	printf("%s:  detonate all probes?\n", nav);
	printf("%s:  [yes or no] ", captain);
	if (Gets(buf1) != NULL && (*buf1 == 'Y' || *buf1 == 'y')) {
		printf("%s:  aye, %s\n",nav, title);
		for (i=0; i<pnum; i++)
			probes[i]->timedelay = 0;
		return 1;
	}
	printf("   control probe [#] ");
	if (Gets(buf1) == NULL)
		return 0;
	probenum = atoi(buf1);
	for (i=0; i<pnum; i++)
		if (probes[i]->id == probenum)
			break;
	if (i == pnum)
		return 0;
	probenum = i;
	printf("%s: detonate it?\n", nav);
	printf("%s:  [yes or no] ", captain);
	if (Gets(buf1) != NULL && (*buf1 == 'y' || *buf1 == 'Y')) {
		probes[probenum]->timedelay = 0;
		return 0;
	}
	printf("   lock it onto [whom] ");
	if (Gets(buf1) != NULL && *buf1 != NULL) {
		ep = ship_name(buf1,ENEMYONLY);
		if (ep != NULL) {
			probes[probenum]->target = ep;
			printf("%s: locking.\n", nav);
			return 1;
		}
	}
	printf("   set it to course [0-360] ");
	if (Gets(buf1) == NULL || *buf1 == NULL)
		return 0;
	j = atoi(buf1);
	if (j < 0 || j > 360)
		return 0;
	probes[probenum]->course = j;
	probes[probenum]->target = NULL;
	printf("%s: setting in new course.\n", nav);
	return 1;
}

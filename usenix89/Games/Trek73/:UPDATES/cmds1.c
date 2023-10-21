/*
 * TREK73: cmds1.c
 *
 * User Commands
 *
 * fire_phasers, fire_tubes, lock_phasers, lock_tubes,
 * turn_phasers, turn_tubes, load_tubes, launch_probe,
 * probe_control
 *
 */

#include "externs.h"

fire_phasers(sp) 
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	char	c;
	int	typed[MAXPHASERS];
	register int i;
	register int k;

	for (i=0; i<MAXPHASERS; i++)
		typed[i] = 0;
	printf("   fire phasers [1-%d] ", sp->num_phasers);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL) {
		printf("%s:  Belay that order!\n", captain);
		return 0;
	}
	printf("   spread [10-45] ");
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL) {
		printf("%s:  Belay that order!\n", captain);
		return 0;
	}
	i = atoi(buf2);
	if (i < 10 || i > 45)
		return 0;
	sp->p_spread = i;
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_phasers - 1)
				continue;
			typed[k]++;
			if (sp->phasers[k].status & (P_DAMAGED | P_FIRING))
				continue;
			sp->phasers[k].status |= P_FIRING;
		}
	else
		for (i=0; i<sp->num_phasers; i++) {
			typed[i]++;
			if (sp->phasers[i].status & (P_DAMAGED | P_FIRING))
				continue;
			sp->phasers[i].status |= P_FIRING;
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
	int	typed[MAXTUBES];
	register int i;
	register int j;
	register int k;

	for (i=0; i<MAXTUBES; i++)
		typed[i] = 0;
	printf("   fire tubes [1-%d] ", sp->num_tubes);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL) {
		printf("%s:  Belay that order!\n", captain);
		return 0;
	}
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_tubes - 1)
				continue;
			typed[k]++;
			if (sp->tubes[k].status & (T_DAMAGED | T_FIRING))
				continue;
			sp->tubes[k].status |= T_FIRING;
		}
	else
		for (i=0; i<sp->num_tubes; i++) {
			typed[i]++;
			if (sp->tubes[i].status & (T_DAMAGED | T_FIRING))
				continue;
			sp->tubes[i].status |= T_FIRING;
		}
	check_t_damage(typed, sp, "fire");	/* Type if damaged */
	check_t_turn(typed, sp, 1);
	j = 0;
	for (i=0; i < sp->num_tubes; i++) {
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
	if (j > 1)
		puts(" are not loaded.");
	else if (j == 1)
		puts(" is not loaded.");
	return 0;
}


lock_phasers(sp)
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	int	typed[MAXPHASERS];
	char	c;
	struct	ship *ep;
	register int i;
	register int k;

	for (i=0; i<MAXPHASERS; i++)
		typed[i] = 0;
	if (is_dead(sp, S_COMP)) {
		printf("%s:  Impossible %s, our computer is dead.\n", science, title);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s:  Our computer is temporarily buggy", science);
		return 0;
	}
	printf("   lock phasers [1-%d] ", sp->num_phasers);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	printf("   onto [whom] ");
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL)
		return 0;
	ep = ship_name(buf2);
	if (ep == NULL)
		return 0;
	if (cantsee(ep)) {
		printf("%s:  %s, unable to lock phasers onto %s.\n",
		    nav, title, ep->name);
		return 0;
	}
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_phasers - 1)
				continue;
			typed[k]++;
			if (sp->phasers[k].status & P_DAMAGED)
				continue;
			sp->phasers[k].target = ep;
		}
	else
		for (i=0; i<sp->num_phasers; i++) {
			typed[i]++;
			if (sp->phasers[i].status & P_DAMAGED)
				continue;
			sp->phasers[i].target = ep;
		}
	check_p_damage(typed, sp, "lock");
	check_p_turn(typed, sp, 0);
	return 1;
}


lock_tubes(sp)
struct ship *sp;
{
	char	buf1[20];
	char	buf2[20];
	int	typed[MAXTUBES];
	char	c;
	struct	ship *ep;
	register int i;
	register int k;

	for (i=0; i<sp->num_tubes; i++)
		typed[i] = 0;
	if (is_dead(sp, S_COMP)) {
		printf("%s:  Impossible %s, our computer is dead.\n", science, title);
		return 0;
	}
	if (!syswork(sp, S_COMP)) {
		printf("%s:  Our computer is temporarily buggy", science);
		return 0;
	}
	printf("   lock tubes [1-%d] ", sp->num_tubes);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	printf("   onto whom ");
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL)
		return 0;
	ep = ship_name(buf2);
	if (ep == NULL)
		return 0;
	if (cantsee(ep)) {
		printf ("%s:  %s, unable to lock tubes onto %s.",
		    nav, title, ep->name);
		return 0;
	}
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_tubes - 1)
				continue;
			typed[k]++;
			if (sp->tubes[k].status & T_DAMAGED)
				continue;
			sp->tubes[k].target = ep;
		}
	else
		for (i=0; i<sp->num_tubes; i++) {
			typed[i]++;
			if (sp->tubes[i].status & T_DAMAGED)
				continue;
			sp->tubes[i].target = ep;
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
	int	typed[MAXPHASERS];
	register int i;
	register float j;
	register int k;

	for (i=0; i<MAXPHASERS; i++)
		typed[i] = 0;
	printf("   turn phasers [1-%d] ", sp->num_phasers);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	printf("   to [0-360] ");
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL)
		return 0;
	j = atof(buf2);
	if (j < 0.0 || j > 360.0)
		return 0;
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_phasers - 1)
				continue;
			typed[k]++;
			if (sp->phasers[k].status & P_DAMAGED)
				continue;
			sp->phasers[k].target = NULL;
			sp->phasers[k].bearing = j;
		}
	else
		for (i=0; i<sp->num_phasers; i++) {
			typed[i]++;
			if (sp->phasers[i].status & P_DAMAGED)
				continue;
			sp->phasers[i].target = NULL;
			sp->phasers[i].bearing = j;
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
	int	typed[MAXTUBES];
	register int i;
	register float j;
	register int k;

	for (i=0; i<MAXTUBES; i++)
		typed[i] = 0;
	printf("   turn tubes [1-%d] ", sp->num_tubes);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	printf("   to [0-360] ");
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL)
		return 0;
	j = atof(buf2);
	if (j < 0.0 || j > 360.0)
		return 0;
	if (strcmp(buf1, "all") && strcmp(buf1, "ALL"))
		for (i=0; c = buf1[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_tubes - 1)
				continue;
			typed[k]++;
			if (sp->tubes[k].status & T_DAMAGED)
				continue;
			sp->tubes[k].target = NULL;
			sp->tubes[k].bearing = j;
		}
	else
		for (i=0; i<sp->num_tubes; i++) {
			typed[i]++;
			if (sp->tubes[i].status & T_DAMAGED)
				continue;
			sp->tubes[i].target = NULL;
			sp->tubes[i].bearing = j;
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
	int	typed[MAXTUBES];
	register int i;
	register float j;
	register int k;

	for (i=0; i<MAXTUBES; i++)
		typed[i] = 0;
	load = 0;
	printf("   [load or unload] ");
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	if (*buf1 == 'l' || *buf1 == 'L')
		load++;
	else if (*buf1 != 'u' && *buf1 != 'U')
		return 0;
	printf("   tubes [1-%d] ", sp->num_tubes);
	(void) Gets(buf2, sizeof(buf2));
	if (buf2[0] == NULL)
		return 0;
	if (strcmp(buf2, "all") && strcmp(buf2, "ALL"))
		for (i=0; c = buf2[i]; i++) {
			k = c - '1';
			if (k < 0 || k > sp->num_tubes - 1)
				continue;
			typed[k]++;
		}
	else
		for (i=0; i<sp->num_tubes; i++)
			typed[i]++;
	for (i = 0; i < sp->num_tubes; i++) {
		tp = &sp->tubes[i];
		if (!typed[i] || tp->status & T_DAMAGED)
			continue;
		if (load) {
			j = min(sp->energy, MAX_TUBE_CHARGE - tp->load);
			if (j <= 0)
				continue;
			sp->energy -= j;
			sp->pods -= j;
			tp->load += j;
		} else {
			j = tp->load;
			if (j <= 0)
				continue;
			sp->energy += j;
			sp->pods += j;
			tp->load = 0;
		}
	}
	printf("%s: Tubes now ", engineer);
	for (i=0; i<sp->num_tubes; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			printf(" -- ");
		else
			printf(" %-2d ", sp->tubes[i].load);
	}
	printf(" energy at %d/%d\n", (int)sp->energy, (int)sp->pods);
	return 1;
}


launch_probe(sp)
struct ship *sp;
{
	char	buf1[20];
	int	pods, delay, prox;
	float	course;
	struct	ship *target;
	struct	list *lp;
	struct	torpedo *pp;

	pods = delay = prox = 0;
	course = 0.0;
	target = NULL;
	if (is_dead(sp, S_PROBE)) {
		printf("%s:  Probe launcher destroyed!\n", engineer);
		return 0;
	}
	if (!syswork(sp, S_PROBE)) {
		printf("%s:  Probe launcher temporarily disabled, %s\n",
		    engineer, title);
		return 0;
	}
	if (sp->energy < MIN_PROBE_CHARGE) {
		printf("%s: We've not enough power, Captain.\n", engineer);
		return 0;
	}
	printf("%s: %d pods available.\n", engineer, (int)sp->energy);
	printf("%s: Number to launch [%d+] is ", captain, MIN_PROBE_CHARGE);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		goto bad_param;
	pods = atoi(buf1);
	if (pods < MIN_PROBE_CHARGE || pods > sp->energy)
		goto bad_param;
	printf("   set time delay to [0-%d] ", MAX_PROBE_DELAY);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		goto bad_param;
	delay = atoi(buf1);
	if (delay < 0 || delay > MAX_PROBE_DELAY)
		goto bad_param;
	printf("   set proximity delay to [%d+] ", MIN_PROBE_PROX);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		goto bad_param;
	prox = atoi(buf1);
	if (prox < MIN_PROBE_PROX)
		goto bad_param;
	printf("   launch towards [whom, if anyone] ");
	Gets(buf1, sizeof(buf1));
	if (*buf1) {
		target = ship_name(buf1);
		if (target == NULL)
			goto bad_param;
		if (cantsee(target) || !syswork(sp, S_SENSOR)) {
			printf("%s:  %s, unable to lock probe onto the %s.\n",
			    helmsman, title, target->name);
			return 0;
		}
	} else {
		printf("   course [0-360] ");
		if (gets(buf1) == NULL)
			goto bad_param;
		course = atof(buf1);
		if (course < 0.0 || course > 360.0)
			goto bad_param;
		target = NULL;
	}
	/*
	 * add a new item to the list of items in space
	 */
	lp = newitem(I_PROBE);
	lp->data.tp = MKNODE(struct torpedo, *, 1);
	pp = lp->data.tp;
	if (pp == (struct torpedo *)NULL) {
		fprintf(stderr, "launch_probe: malloc failed\n");
		exit(2);
	}
	pp->from = sp;
	pp->fuel = pods;
	pp->timedelay = delay;
	pp->speed = sp->warp;
	pp->newspeed = 3.0;
	pp->prox = prox;
	pp->target = target;
	pp->course = course;
	pp->x = sp->x;
	pp->y = sp->y;
	pp->id = new_slot();
	pp->type = TP_PROBE;
	/*
	 * subtract the pods used
	 */
	sp->pods -= pods;
	sp->energy -= pods;
	sp->probe_status = PR_LAUNCHING;
	printf("%s: Probe %d away\n",engineer, pp->id);
	return 1;
bad_param:
	printf("%s: Bad parameters, %s.\n", science, title);
	return 0;
}


probe_control(sp)
struct ship *sp;
{
	register struct list *lp;
	register int i;
	register float j;
	register struct torpedo *pp;
	struct	torpedo *probes[20];
	int	probenum;
	struct	ship *ep;
	int	pnum;
	float	bear;
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
		if (pnum >= sizeof probes / sizeof probes[0]) {
			printf("\n%s:  There are other probes out but\n",
			    nav);
			puts("   these are all we can control at one time.");
		}
		probes[pnum] = pp;
		pnum++;
		range = rangefind(sp->x, pp->x, sp->y, pp->y);
		bear = bearing(sp->x, pp->x, sp->y, pp->y);
		if (pp->target == NULL)
			bp = "NONE";
		else
			bp = pp->target->name;
		printf(" %2d    %4.1f %5d  %4.0f  %4.1f %5d  %3d  %s\n",
			pp->id, bear, range, pp->course, pp->timedelay,
			pp->prox, pp->fuel, bp);
	}
	if (!pnum) {
		printf("%s: What probes?\n", nav);
		return 0;
	}
	printf("%s:  Detonate all probes?\n", nav);
	printf("%s:  [yes or no] ", captain);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] != NULL && (*buf1 == 'Y' || *buf1 == 'y')) {
		printf("%s:  Aye, %s\n", nav, title);
		for (i=0; i<pnum; i++)
			probes[i]->timedelay = 0.0;
		sp->probe_status = PR_DETONATE;
		return 1;
	}
	printf("   control probe [#] ");
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	probenum = atoi(buf1);
	for (i=0; i<pnum; i++)
		if (probes[i]->id == probenum)
			break;
	if (i == pnum)
		return 0;
	probenum = i;
	printf("%s:  Detonate it?\n", nav);
	printf("%s:  [yes or no] ", captain);
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] != NULL && (*buf1 == 'y' || *buf1 == 'Y')) {
		probes[probenum]->timedelay = 0.;
		sp->probe_status = PR_DETONATE;
		return 1;
	}
	printf("   lock it onto [whom, if anyone] ");
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] != NULL) {
		ep = ship_name(buf1);
		if (ep != NULL) {
			sp->probe_status = PR_LOCK;
			if (cansee(ep) && syswork(sp, S_SENSOR)) {
				probes[probenum]->target = ep;
				printf("%s: locking.\n", nav);
				return 1;
			} else {
				printf("%s:  %s, unable to lock probe on the %s.\n",
				    helmsman, title, ep->name);
				return 0;
			}
		}
	}
	printf("   set it to course [0-360] ");
	(void) Gets(buf1, sizeof(buf1));
	if (buf1[0] == NULL)
		return 0;
	sp->probe_status = PR_LOCK;
	j = atof(buf1);
	if (j < 0.0 || j > 360.0)
		return 0;
	probes[probenum]->course = j;
	probes[probenum]->target = NULL;
	printf("%s: setting in new course.\n", nav);
	return 1;
}

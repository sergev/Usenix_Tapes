/*
 * TREK73: cmds3.c
 *
 * User Commands
 *
 * jettison_engineering, detonate_engineering, phaser_status, tube_status,
 * survivors, alter_power
 *
 * (do_jettison)
 *
 */

#include "defines.h"
#include "structs.h"
#include <ctype.h>
#include <stdio.h>

extern char title[];
extern char engineer[];

jettison_engineering(sp)
struct ship *sp;
{

	printf("   Mr. %s, jettison our engineering section!\n", engineer);
	if (sp->status & S_ENG) {
		printf("%s:  But Captain, it's already jettisonned.\n", engineer);
		return 0;
	}
	do_jettison(sp);
	printf("%s:  Jettisoning engineering.\n", engineer);
	return 1;
}

do_jettison(sp)
struct ship *sp;
{
	extern	struct list *newitem();
	register struct list *lp;
	register struct torpedo *tp;

	lp = newitem(I_ENG);
	tp = lp->data.tp = MKNODE(struct torpedo, *, 1);
	tp->target = NULL;
	tp->speed = sp->warp;
	tp->newspeed = 0.0;
	tp->x = sp->x;
	tp->y = sp->y;
	tp->course = sp->course;
	tp->fuel = sp->pods;
	tp->timedelay = 10 * 10;
	tp->prox = 0;
	tp->from = sp;
	tp->id = new_slot();
	sp->energy = sp->pods = 0;
	sp->newwarp = .99 * (sp->warp < 0.0 ? -1.0 : 1.0);
	sp->regen = 0.0;
	sp->status |= S_ENG;
	sp->status |= S_WARP;
}

detonate_engineering(sp)
struct ship *sp;
{
	extern	struct list head;
	extern	struct list *tail;
	register struct list *lp;
	register struct torpedo *tp;
	register int found;
	char buf[10];

	found = 0;
	printf("   %s, detonate engineering!\n", engineer);
	if (!(sp->status & S_ENG)) {
		printf("%s: But %s, it's still attached.\n",engineer,title);
		printf("   Detonate anyway? ");
		if ((Gets(buf) == NULL) || (buf == NULL))
			return 0;
		if (buf[0] != 'y')
			return 0;
		else 
			do_jettison(sp);
	}
	for (lp = &head; lp != tail; lp = lp->fwd) {
		if (lp->type != I_ENG)
			continue;
		tp = lp->data.tp;
		if (tp->from != sp)
			continue;
		found++;
		tp->timedelay = 1;
		break;
	}
	if (found)
		printf("%s:  Aye, %s.\n",engineer, title);
	else
		printf("%s:  Ours has already detonated.\n", engineer);
	return 1;
}


phaser_status(sp)
struct ship *sp;
{
	register int i;

	printf("Phasers\n\n");
	printf("Control: ");
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
			printf("\tdamaged");
		else if (sp->phasers[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", sp->phasers[i].target->name);
	}
	printf("\n Turned: ");
	for (i=0; i<4; i++)
		if (sp->phasers[i].status & P_DAMAGED)
			printf("\t");
		else if (sp->phasers[i].target == NULL)
			printf("\t%d", sp->phasers[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n  Level: ");
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
			printf("\t");
		else
			printf("\t%d", sp->phasers[i].load);
	}
	printf("\n  Drain: ");
	for (i=0; i<4; i++) {
		if (sp->phasers[i].status & P_DAMAGED)
			printf("\t");
		else
			printf("\t%d", sp->phasers[i].drain);
	}
	printf("\n\nFiring percentage: %d\n",sp->p_percent);
	return 1;
}

tube_status(sp)
struct ship *sp;
{
	register int i;

	printf("Torpedos\n\n");
	printf("Control: ");
	for (i=0; i<6; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			printf("\tdamaged");
		else if (sp->tubes[i].target == NULL)
			printf("\tmanual");
		else
			printf("\t%.7s", sp->tubes[i].target->name);
	}
	printf("\n Turned: ");
	for (i=0; i<6; i++)
		if (sp->tubes[i].status & T_DAMAGED)
			printf("\t");
		else if (sp->tubes[i].target == NULL)
			printf("\t%d", sp->tubes[i].bearing);
		else
			printf("\tLOCKED");
	printf("\n  Level: ");
	for (i=0; i<6; i++) {
		if (sp->tubes[i].status & T_DAMAGED)
			printf("\t");
		else
			printf("\t%d", sp->tubes[i].load);
	}
	printf("\n\nLaunch speed: %d\n", sp->t_lspeed);
	printf("  time delay: %d\n", sp->t_delay);
	printf("  prox delay: %d\n", sp->t_prox);
	return 1;
}

survivors(sp)
struct ship *sp;
{
	extern struct ship *shiplist[];
	extern int shipnum;
	struct ship *ep;
	register int i;

	printf("\nSurvivors reported:\n");
	for (i=0; i<=shipnum; i++) {
		ep = shiplist[i];
		printf("   %s -- %d\n", ep->name, ep->crew);
	}
	sp = sp;				/* LINT */
}

alter_power()
{
	extern char **argp;
	extern struct ship *shiplist[];
	extern char captain[];
	extern double atof();
	register int i;
	float j;
	char buf1[20];

	printf("\n%s:  Regeneration rate is %5.2f.\n",engineer, shiplist[0]->regen);
	for (i=0; i<4; i++) {
		printf("%s:  Shield %d drain is ", captain, i + 1);
		Gets(buf1);
		j = (float) atof(buf1);
		if (buf1[strlen(buf1) - 1] == '*') {
			for (; i<4; i++)
				shiplist[0]->shields[i].attemp_drain =j;
			break;
		} else if ((j < 0.0) || (j > 1.0))
			goto badparam;
		else
			shiplist[0]->shields[i].attemp_drain = j;
	}				
	printf("\n");
	for (i=0; i<4; i++) {
		printf("%s:  Phaser %d drain is ", captain, i + 1);
		Gets(buf1);
		j = (float) atof(buf1);
		if (buf1[strlen(buf1) - 1] == '*') {
			for (; i<4; i++)
				shiplist[0]->phasers[i].drain = (int) j;
			break;
		} else if ((j < -10.0) || (j > 10.0))
			goto badparam;
		else
			shiplist[0]->phasers[i].drain = (int) j;
	}
	return 1;
badparam:
	printf("%s:  Bad parameters, %s.\n", engineer, title);
	return 0;
}

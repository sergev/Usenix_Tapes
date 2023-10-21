/*
 * TREK73: misc.c
 *
 * Miscellaneous Routines
 *
 * help, scancmd, new_slot, return_slot, betw, vowelstr, plural,
 * check_p_damage, check_t_damage, check_p_turn, check_t_turn
 *
 */

#include "defines.h"
#include "structs.h"
#include <stdio.h>

int help(dummy)
struct ship *dummy;
{
	extern	struct cmd cmds[];
	struct	cmd *cp;
	int column = 0;

	printf("\nTrek84 Commands: \n");
	printf("Code		Command\n\n");
	for (cp = &cmds[0]; cp->routine != NULL; cp++) {
		printf("%3s: ", cp->word1);
		if (cp->turns == 0)
			printf (" *");
		else
			printf ("  ");
		printf(" %-31s", cp->word2);
		if (column++ & 1)
			puts("");
	}
	printf("\n\n\n * does not use a turn\n");
	dummy = dummy;				/* LINT */
}

struct cmd *scancmd(buf)
char *buf;
{
	extern	struct cmd cmds[];
	struct	cmd *cp;
	extern	char **argp;
	int	argnum;
	int	first;

	argnum = parsit(buf, &argp);
	first = strlen(argp[0]);
	if (argnum && first) {
		for (cp = &cmds[0]; cp->routine != NULL; cp++) {
			if (strncmp(argp[0], cp->word1, first) == 0)
				return (cp);
		}
	}
	return (NULL);
}

/*
 * This routine handles getting unique identifier numbers for
 * all objects.
 */
new_slot()
{
	extern char slots[];
	extern int shipnum;
	/*
	 * This is to make it appear that in a 2-ship duel, for
	 * instance, the first object to appear will be numbered
	 * as 3.
	 */
	int i = shipnum + 2;

	while (slots[i] == 'X')
		i++;
	slots[i] = 'X';
	return i;
}

/* 
 * This routine handles returning identifiers
 */
return_slot(i)
int i;
{
	extern char slots[];
	
	if (slots[i] != 'X')
		printf("FATAL ERROR - Slot already empty!");
	slots[i] = ' ';
}

betw(i, j, k)
int i, j, k;
{
	if ((i > j) && (i < k))
		return(1);
	else
		return(0);
}

char *vowelstr(str)
char *str;
{
	switch(*str) {
		case 'a': case 'A':
		case 'e': case 'E':
		case 'i': case 'I':
		case 'o': case 'O':
		case 'u': case 'U':
			return "n";
		default:
			return "";
	}
}

char *plural(i)
int i;
{
	if (i != 1)
		return("s");
	else
		return("");
}

/*
 * This routine takes an array generated from commands 1, 3, and 5
 * to print out a list of those phasers damaged and unable to
 * either fire, lock, or turn.
 */
check_p_damage(array, sp, string)
int array[];
struct ship *sp;
char *string;
{
	int i, j = 0;

	for (i=0; i<4; i++) {
		if ((array[i] != 0) && (sp->phasers[i].status & P_DAMAGED)) {
			if (!j)
				printf("Computer: Phaser(s) %d", i+1);
			else
				printf(", %d", i+1);
			j++;
		}
	}
	if (j)
		printf(" damaged and unable to %s.\n", string);
}

/*
 * This routine takes an array generated from commands 2, 4, and 6
 * to print out a list of those tubes damaged and unable to either
 * fire, lock, or turn.
 */
check_t_damage(array, sp, string)
int array[];
struct ship *sp;
char *string;
{
	int i, j = 0;

	for (i=0; i<6; i++) {
		if ((array[i] != 0) && (sp->tubes[i].status & P_DAMAGED)) {
			if (!j)
				printf("Computer: Tube(s) %d", i+1);
			else
				printf(", %d", i+1);
			j++;
		}
	}
	if (j)
		printf(" damaged and unable to %s.\n", string);
}

/*
 * This routine checks to see if a phaser is pointing into our
 * blind side
 */
check_p_turn(array, sp, flag)
int array[];
struct ship *sp;
int flag;			/* If 1, came from fire_phasers */
{
	register int i;
	register int j = 0;
	register int k;
	register int bear;
	struct ship *target;

	for (i=0; i<4; i++) {
		if (array[i] == 0)
			continue;
		if ((flag) && (!(sp->phasers[i].status & P_FIRING)))
			continue;
		target = sp->phasers[i].target;
		/*
		 * This hack is here since when the phaser is locked,
		 * the bearing points at the target, whereas when
		 * not locked, the bearing is relative to the ship.
		 */
		if (target == NULL) {
			bear = sp->phasers[i].bearing + sp->course;
			k = sp->phasers[i].bearing;
		} else {
			bear = bearing(sp->x, target->x, sp->y, target->y);
			k = bear - sp->course;
		}
		k = rectify(k);
		if ((k > 125) && (k < 235) && (!(sp->status & S_ENG))) {
			if (!j)
				printf("Computer: Phaser(s) %d", i + 1);
			else
				printf(", %d", i + 1);
			j++;
		}
	}
	if (j)
		printf(" are pointing into our blind side.\n");
}

/*
 * This routine checks to see if a tube is turned into
 * our blind side.
 */
check_t_turn(array, sp, flag)
int array[];
struct ship *sp;
int flag;			/* If 1, came from fire_tubes */
{
	register int i;
	register int j = 0;
	register int k;
	register int bear;
	struct ship *target;

	for (i=0; i<6; i++) {
		if (array[i] == 0)
			continue;
		if (flag && (!(sp->tubes[i].status & T_FIRING)))
			continue;
		target = sp->tubes[i].target;
		/*
		 * This hack is here since when the tube is locked,
		 * the bearing points at the target, whereas when
		 * not locked, the bearing is relative to the ship.
		 */
		if (target == NULL) {
			bear = sp->tubes[i].bearing + sp->course;
			k = sp->tubes[i].bearing;
		} else {
			bear = bearing(sp->x, target->x, sp->y, target->y);
			k = bear - sp->course;
		}
		k = rectify(k);
		if ((k > 135) && (k < 225) && (!(sp->status & S_ENG))) {
			if (!j)
				printf("Computer: Tubes(s) %d", i + 1);
			else
				printf(", %d", i + 1);
			j++;
		}
	}
	if (j)
		printf(" are pointing into our blind side.\n");
}

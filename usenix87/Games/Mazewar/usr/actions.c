#ifndef lint
static char rcsid[] = "$Header: acts.c,v 1.1 84/08/25 17:11:08 jcoker Exp $";
#endif

#include <stdio.h>
#include "../h/defs.h"

extern int	act_beep(),
		act_redraw(),
		act_forward(),
		act_shoot(),
		act_quit(),
		act_180turn(),
		act_left(),
		act_right(),
		pact_left(),
		pact_stop(),
		pact_right(),
		act_back_up();

#undef CTRL
#define CTRL(c)		('c' & 037)

#define ASCII_MASK	0177
#ifdef sun
#define NUM_ACTS	(NUM_ASCII + 3)
#else
#define NUM_ACTS	NUM_ASCII
#endif

static int	(*player_acts[NUM_ACTS])();

/*
 *  Set up the default key mappings.
 *  We are hended the flag dobeep to tell us what the
 *  default acts are for the unused keys.
 *  We also set the undef type to the proper thing.
 */

static int	dobeep = 0;

set_default_acts(beep)
{
	int	i;

	dobeep = beep;

	if (dobeep) {
		for (i = 0; i < NUM_ASCII; i++)
			player_acts[i] = act_beep;
	} else {
		for (i = 0; i < NUM_ASCII; i++)
			player_acts[i] = NULL;
	}

	player_acts[CTRL(l)]	= act_redraw;
	player_acts[CTRL(r)]	= act_redraw;
	player_acts['w']	= act_forward;
	player_acts['s']	= act_shoot;
	player_acts['Q']	= act_quit;
	player_acts['x']	= act_180turn;
	player_acts['a']	= act_left;
	player_acts['d']	= act_right;
	player_acts['j']	= pact_left;
	player_acts['l']	= pact_right;
	player_acts['k']	= pact_stop;
	player_acts[' ']	= act_back_up;

	return(0);
}


/*
 *  Set the act for the given character to the
 *  function represented by the given string.
 */

struct act_list {
	char	*act_name;
	int	(*act_funct)();
};
static struct act_list	act_list[] = {
	{ "redraw",	act_redraw },
	{ "forward",	act_forward },
	{ "shoot",	act_shoot },
	{ "quit",	act_quit },
	{ "turn-back",	act_180turn },
	{ "turn-left",	act_left },
	{ "turn-right",	act_right },
	{ "peek-left",	pact_left },
	{ "stop-peek",	pact_stop },
	{ "peek-right",	pact_right },
	{ "back-up",	act_back_up },
	{ NULL,		NULL }
};

set_act(key, act)
	char	*act;
{
	struct act_list	*act;

	if (key >= NUM_ACTS || key < 0) {
		fprintf(stderr, "%d (0%o): Illegal character!\n", key, key);
		return(-1);
	}

	if (!strcmp(act, "undef")) {
		if (dobeep)
			player_acts[key] = act_beep;
		else
			player_acts[key] = NULL;

		return(0);
	}

	for (act = act_list; act->act_name; act++)
		if (!strcmp(act->act_name, act)) {
			player_acts[key] = act->act_funct;
			return(0);
		}

	fprintf(stderr, "act \"%s\" not found.\n", act);
	return(-1);
}


/*
 *  Set the acts based on the input string.
 */

parse_act_string(string)
	char	*string;
{
	char	act[BUFSIZ], *ap, *sp;
	char	key[10], *kp;

	if (string == NULL || *string == '\0') {
		fprintf(stderr, "Null act string!\n");
		return(-1);
	}

	for (sp = string; *sp; sp++) {
		kp = key;
		do
			*kp++ = *sp++;
		while (*sp && *sp != '=');
		*kp = '\0';
		if (strlen(key) != 1) {
#ifdef sun
			if (*key == 'M') {
				switch (key[1]) {
				case '1':	/* left key */
					*key = MOUSE_LEFT;
					break;

				case '2':	/* middle key */
					*key = MOUSE_MIDDLE;
					break;

				case '3':	/* right key */
					*key = MOUSE_RIGHT;
					break;

				default:	/* what?!? */
					fputs(
				"Bad mouse key specification; use M1 - M3.\n",
					    stderr);
					return(-1);
				}
			}
#else
			fputs("Bad format in act, use <char>=act.\n",
			    stderr);
			return(-1);
#endif
		}
		if (*sp != '=') {
			fputs("Bad format in act, use <char>=act.\n",
			    stderr);
			return(-1);
		} else
			sp++;
		ap = act;
		do
			*ap++ = *sp++;
		while (*sp && *sp != ',');
		*ap = '\0';
		if (set_act(*key, act) < 0) {
			/* set_act will bitch on error */
			return(-1);
		}
	}
}


/*
 *  Return an act from the given character.
 */

int (*input_act(ch))()
{
	if (ch > NUM_ACTS || ch < 0) {
		fprintf(stderr, 
		    "input_act: %d (0%o): Illegal act character!\n", 
		    ch, ch);
		return(NULL);
	}

	return player_acts[ch];
}

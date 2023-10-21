/**********************************  scores.c  **********************/
#include <pwd.h>
#include <stdio.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * This code is responsible 
 * reading, updating, and displaying the score file.
 */

#ifndef SCOREFILE
#define SCOREFILE "/usr/games/lib/sdi_scores"
#endif

struct scores *get_scores();

/* Already updated is used to prevent rewriting the score file more than
 * once per game.
 */
static int already_updated = 0;

/*
 * This routine is called whenever there is absolutely positively a new
 * game beginning, and so if it ever gets anywhere it is ok to update
 * the score file.
 */
new_score()
{
	already_updated = 0;
}

/*
 * Read in the old scores and insert the current game score into the file
 * if it is within the top ten.  No scores are recorded in gamemaster mode.
 */
update_scores()
{
	extern int gamemaster;
	struct scores *scp, *new_pos = NULL;
	int i;
	int score = atol((char *)panel_get_value(score_item));
	int level = atol((char *)panel_get_value(level_item));
	int skill = (int)panel_get_value(skill_item);
	if (already_updated || gamemaster)
		return;
	if ((scp = get_scores()) == NULL)
		return;
	while (scp->score > score) {
		scp += 1;
	}
	if (scp->score >= 0) {
		struct scores *new_sc = sc_end;
		while (--new_sc > scp) {
			strcpy(new_sc->name, (new_sc-1)->name);
			new_sc->score = (new_sc-1)->score;
			new_sc->level = (new_sc-1)->level;
			new_sc->skill = (new_sc-1)->skill;
		}
		strcpy(scp->name, USER_NAME);
		scp->score = score;
		scp->level = level;
		scp->skill = skill;
	}
	if (! put_scores(sc)) {
		printf("Could not write to score file '%s'.\n",scorefile);
	}
	already_updated = 1;
}

/*
 * Get the name of the current user.
 */
char *
get_name()
{
	char *s;
	struct passwd *p;
	if (s = (char *)getlogin())
		return s;
	p = (struct passwd *)getpwuid(getuid());
	return p->pw_name;
}

/*
 * Read the score file.  The format is obvious.  Exactly 11 scores are always
 * returned.  The 11th is a sentinal with score '-1'.  If the score file did
 * not have 10, then the extras are for user '(nobody)', score 0.
 * A user name may not contain a "'".  The result of get_scores is an array
 * of score structures, statically allocated.
 */
struct scores *
get_scores()
{
	struct scores *scp;
	char *tmpfile, *getenv();
	FILE *f;
	if (scorefile) {
		tmpfile = scorefile;
	} else if ((tmpfile = getenv("SDI_SCORES")) == NULL) {
			tmpfile = scorefile;
	} else {
		tmpfile = "/usr/games/lib/sdi_scores";
	}
	/* Try hard to open some kind of score file */
	if (((f = fopen(tmpfile, "r+")) == NULL)) {
		tmpfile = SCOREFILE;
		if (((f = fopen(tmpfile, "r+")) == NULL)) {
			tmpfile = "/usr/games/lib/sdi_scores";
			if ((f = fopen("/usr/games/lib/sdi_scores", "r+")) == NULL)
				return NULL;
		}
	}
	scorefile = tmpfile;

	/* read entries */
	scp = sc;
	while (scp < sc_end &&
		(fscanf(f, " '%[^']' %d %d %d", scp->name, &scp->score, &scp->level, &scp->skill) == 4) &&
		scp->score > 0) {
		scp += 1;
	}
	while (scp < sc_end) {
		strcpy(scp->name, "(nobody)");
		scp->score = 0;
		scp->level = 0;
		scp->skill = 0;
		scp += 1;
	}
	scp->score = -1;
	fclose(f);
	return sc;
}

/*
 * Write a list of scores to the score file.  Never called directly, except
 * by update_scores.
 */
put_scores(scp)
struct scores *scp;
{
	FILE *f;
	if ((f = fopen(scorefile, "w")) == NULL)
		return 0;
	/* write entries */
	while (scp->score > 0) {
		fprintf(f, "'%s' %d %d %d\n", scp->name, scp->score, scp->level, scp->skill);
		scp += 1;
	}
	fclose(f);
	return 1;
}

/*
 * This routine formats the score file for display when the 'scores'
 * button is pressed.
 */
static char string_for_build_scores[1024];

char *
build_scores()
{
	char *sp = string_for_build_scores;
	struct scores *scp;
	char *type_of_skill;
	scp = (struct scores *)get_scores();
	if (scp == NULL) {
		strcpy(sp, "Could not find a score file.");
	} else {
		strcpy(sp, "No scores yet: play some games.");
		while (scp->score > 0) {
			switch(scp->skill) {
				case 0:
					type_of_skill = " novice";
					break;
				case 1:
					type_of_skill = "n occasional user";
					break;
				case 2:
					type_of_skill = "n expert";
					break;
			}
			sprintf(sp,
				"%s, playing as a%s, melted with score %d at level %d.\n",
				scp->name, type_of_skill, scp->score, scp->level);
			scp += 1;
			sp += strlen(sp);
		}
	}
	return string_for_build_scores;
}

/*
 * Free the result of a call to build_scores.
 */
free_scores(a)
char **a;
{
/* Oh, how I long for garbage collection. */
	while (*++a) 
		free(*a);
}


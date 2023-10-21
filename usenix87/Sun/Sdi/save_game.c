/*****************************  save_game.c  ******************************/
#include <sunwindow/notify.h>
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
 * Code to save and restore games.
 */

/* If the save format is changed, change the version number. */
#define SAVE_VERSION 6

extern int time_to_play, gamemaster, cursor_type;
extern char save_file_name[];
extern Panel_item cursor_item, cycle_time_item;
extern char *strcpy();

void
save_game() {
	char tmpbuf[128];
	int control_x, control_y;
	FILE *Savefile;

	if ((Savefile = fopen(SAVE_FILE_NAME, "r")) != NULL) {
		sprintf(tmpbuf, "Save file '%s' already exists.  Overwrite?", SAVE_FILE_NAME);
		if (! easy_warn(tmpbuf))
			return;
		fclose(Savefile);
	}

	if ((Savefile = fopen(SAVE_FILE_NAME, "w")) == NULL) {
		sprintf(tmpbuf, "Cannot open save file '%s'.", SAVE_FILE_NAME);
		easy_pop(tmpbuf);
	}


	fprintf(Savefile, "version = %d\n", SAVE_VERSION);

	fprintf(Savefile, "level = %s\n",(char *)panel_get_value(level_item));

	fprintf(Savefile, "skill = %d\n", panel_get_value(skill_item));

	fprintf(Savefile, "interceptor_val = %d\n", panel_get_value(interceptor_item));

	fprintf(Savefile, "laser_val = %d\n", panel_get_value(laser_item));

	fprintf(Savefile, "rock_val = %d\n", panel_get_value(rock_item));

	fprintf(Savefile, "game_master = %d\n", gamemaster);

	fprintf(Savefile, "score = %s\n", panel_get_value(score_item));

	fprintf(Savefile, "name = '%s'\n", USER_NAME);

	fprintf(Savefile, "playing field size = %d, %d\n", 
		window_get(cityframe, WIN_WIDTH), 
		window_get(cityframe, WIN_HEIGHT));

	fprintf(Savefile, "control panel position = %d, %d\n",
		(control_x = (int)window_get(controlframe, WIN_X)), 
		(control_y = (int)window_get(controlframe, WIN_Y)));

	/*
	 * Kludge around a "feature" of Sunview in which negative
	 * positions are ignored.
	 */
	window_set(controlframe, WIN_X, 0, WIN_Y, 0, 0);

	fprintf(Savefile, "city field position = %d, %d\n",
		window_get(cityframe, WIN_X), 
		window_get(cityframe, WIN_Y));

	fprintf(Savefile, "launch field position = %d, %d\n",
		window_get(launchframe, WIN_X), 
		window_get(launchframe, WIN_Y));

	window_set(controlframe, WIN_X, control_x, WIN_Y, control_y, 0);

	fprintf(Savefile, "blast_delay = %d\n", blast_delay);

	fprintf(Savefile, "cursor_type = %d\n", cursor_type);

	fclose(Savefile);
}

void
restore_game() {
	int version;
	int tmpint, w, h, x, y;
	int control_x, control_y;
	char tmpbuf[256];
	FILE *Savefile;
	if ((Savefile = fopen(SAVE_FILE_NAME, "r")) == NULL) {
		sprintf(tmpbuf, "Can't open restore file '%s'.", SAVE_FILE_NAME);
		easy_pop(tmpbuf);
		return;
	}

	fscanf(Savefile, "version = %d\n", &version);
	if (version != SAVE_VERSION) {
		easy_pop("The save file has the wrong version.");
		return;
	}

	fscanf(Savefile, "level = %s\n", tmpbuf);
	panel_set_value(level_item, tmpbuf);

	fscanf(Savefile, "skill = %d\n", &tmpint);
	panel_set_value(skill_item, tmpint);

	fscanf(Savefile, "interceptor_val = %d\n", &tmpint);
	panel_set(interceptor_item, PANEL_VALUE, tmpint,
		PANEL_MAX_VALUE, tmpint,
		0);

	fscanf(Savefile, "laser_val = %d\n", &tmpint);
	panel_set(laser_item, PANEL_VALUE, tmpint,
		PANEL_MAX_VALUE, tmpint,
		0);

	fscanf(Savefile, "rock_val = %d\n", &tmpint);
	panel_set(rock_item, PANEL_VALUE, tmpint,
		PANEL_MAX_VALUE, tmpint,
		0);

	fscanf(Savefile, "game_master = %d\n", &gamemaster);

	fscanf(Savefile, "score = %s\n", tmpbuf);
	panel_set_value(score_item, tmpbuf);

	fscanf(Savefile, "name = '%[^']'\n", tmpbuf);
	if (user_name[0] == '\0')
		panel_set_value(user_name_item, tmpbuf);
	strcpy(user_name, tmpbuf);

	fscanf(Savefile, "playing field size = %d, %d\n", &w, &h);
	/* Only set one, they track each other. */
	window_set(cityframe, WIN_WIDTH, w, WIN_HEIGHT, h, 0);

	fscanf(Savefile, "control panel position = %d, %d\n",&control_x,&control_y);
	/*
	 * Kludge around a "feature" of Sunview in which negative
	 * positions are ignored.
	 */
	window_set(controlframe, WIN_X, 0, WIN_Y, 0, 0);

	fscanf(Savefile, "city field position = %d, %d\n",&x,&y);
	window_set(cityframe, WIN_X, x, WIN_Y, y, 0);

	fscanf(Savefile, "launch field position = %d, %d\n",&x,&y);
	window_set(launchframe, WIN_X, x, WIN_Y, y, 0);

	window_set(controlframe, WIN_X, control_x, WIN_Y, control_y, 0);

	fscanf(Savefile, "blast_delay = %d\n", &blast_delay);
	panel_set_value(cycle_time_item, blast_delay);

	fscanf(Savefile, "cursor_type = %d\n", &tmpint);
	cursor_type = tmpint;

	fclose(Savefile);
}

FILE *
getsavefile(s)
char *s;
{
	char *filename;
	FILE *stream;
	Event event;
	filename = SAVE_FILE_NAME;
	if (((stream = fopen(filename, s)) == NULL)) {
		easy_pop("Can't open the save file.");
	}
	return stream;
}

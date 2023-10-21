/*****************************  text.c  ******************************/
#include "sdi.h"
#define PIES
#ifdef PIES
#include "piemenu.h"
#endif

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Find the size of the longest line in a string of lines separated by newlines
 */
max_line(s)
char *s;
{
	int max = 0, count = 0;
	while (*s) {
		if (*s++ == '\n') {
			if (count > max)
				max = count;
			count = 0;
			continue;
		}
		count += 1;
	}
	if (count > max)
		max = count;
	return max;
}

/*
 * Count the number of lines in a string of lines separated by newlines.
 */
count_lines(s)
char *s;
{
	int count = 0;
	while (*s) {
		if (*s++ == '\n')
			count += 1;
	}
	return count+1;
}

static char *help_msg[] = {
#include "novice_advice.h"
	,
#include "occasional_advice.h"
	,
#include "expert_advice.h"
	};

/*
 * Display a brief message of advice appropriate to the current skill level.
 */
void
help_proc(item, event)
Panel_item item;
Event *event;
{
	int skill = (int)panel_get_value(skill_item);
	popup_msg(controlframe, event, help_msg[skill]);
}

static char *about_msg = 
#include "about_msg.h"
	;
/*
 * Display a brief informative message about the game.
 */
void
about_proc(item, event)
Panel_item item;
Event *event;
{
	popup_msg(controlframe, event, about_msg);
}

static char *art_msg = "This space reserved for Marcel Duchamp";
/*
 * Display a brief informative message about art.
 */
void
art_proc(item, event)
Panel_item item;
Event *event;
{
	popup_msg(controlframe, event, art_msg);
}

/*
 * Display the complete source code of the game in a popup window.
 * The external variable 'source code' must be properly filled elsewhere.
 * (See the sdi makefile for one way.)
 */
void
source_proc(item, event)
Panel_item item;
Event *event;
{
	extern char *source_code;
	popup_msg(controlframe, event, source_code);
}

/*
 * Display the history of the game's development in a popup window.
 * The external variable 'history_text' must be properly filled elsewhere.
 * (See the sdi makefile for one way.)
 */
void
history_proc(item, event)
Panel_item item;
Event *event;
{
	extern char *history_text;
	popup_msg(controlframe, event, history_text);
}

/*
 * Display the man entry in a popup (sort of) window.
 * The external variable 'man_text' must be properly filled elsewhere.
 * (See the sdi makefile for one way.)
 */
void
man_proc(item, event)
Panel_item item;
Event *event;
{
	extern char *man_text;
	popup_msg(controlframe, event, man_text);
}

void
instructions_proc()
{
	easy_pop(
#include "instructions.h"
		);
}

void
version_proc()
{
	extern char *version;
	easy_pop(version);

}

text_options_proc(item, event)
Panel_item item;
Event *event;
{
	extern scores_proc();
	extern struct pixfont *buttonfont; /* use 'struct pixfont' for 3.0 compatiblity */
	Menu_item mi;
	Menu menu, menu_create();
	int (*selection)();
	suspend_proc();
	menu = menu_create(MENU_NOTIFY_PROC, menu_return_item, 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Source", MENU_CLIENT_DATA, source_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "History", MENU_CLIENT_DATA, history_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Man", MENU_CLIENT_DATA, man_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Scores", MENU_CLIENT_DATA, scores_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "About", MENU_CLIENT_DATA, about_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Art", MENU_CLIENT_DATA, art_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Advice", MENU_CLIENT_DATA, help_proc, 0), 0);
	menu_set(menu, MENU_APPEND_ITEM, menu_create_item(MENU_STRING, "Version", MENU_CLIENT_DATA, version_proc, 0), 0);
	special_menu_show(menu, event);
}

special_menu_show(menu, event)
{
	int notify_me();
	pie_menu_show(menu, controlframe, event, notify_me, "Things To Read ('pie' form)");
}

notify_me(mi, event)
Menu_item mi;
{
	int (*selection)();
	if (mi != NULL){
		(caddr_t)selection = (caddr_t)menu_item_get(mi, MENU_CLIENT_DATA);
		(*selection)(0, event);
	}
	resume_proc();
}

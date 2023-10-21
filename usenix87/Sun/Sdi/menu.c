#include <suntool/sunview.h>
#include <stdio.h>
#include "walkmenu_impl.h"
#include "image_impl.h"

#define MAX_ITEMS 32

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Pie menu code -- patent applied for.
 */

/*
 * The front-end routines to make pie-menus look like the normal
 * Sun window library (at least a little bit).
 */


void
pie_menu_set(m, arg)
struct menu *m;
int arg;
{
	char **argptr = &(char *)arg;
	pie_menu_set_internal(m, argptr);
}

pie_menu_item_set(mi, arg)
struct menu_item *mi;
int arg;
{
	char **argptr = &(char *)arg;
	pie_menu_item_set_internal(mi, argptr);
}

struct menu *
pie_menu_create(arg)
int arg;
{
	char **argptr = &(char *)arg;
	struct menu *m;
	m = (struct menu *)calloc(sizeof(struct menu),1);
	m->item_list = (struct menu_item **)calloc(sizeof(struct menu_item *), MAX_ITEMS);
	m->max_nitems = MAX_ITEMS;
	pie_menu_set_internal(m, argptr);
	return m;
}

struct menu_item *
pie_menu_create_item(arg)
int arg;
{
	char **argptr = &(char *)arg;
	struct menu_item *mi;
	mi = (struct menu_item *)calloc(sizeof(struct menu_item),1);
	mi->image = (struct image *)calloc(sizeof(struct image),1);
	pie_menu_item_set_internal(mi, argptr);
	return mi;
}

pie_menu_set_internal(m, argptr)
struct menu *m;
char **argptr;
{
	while (*argptr != 0) {
		switch(*argptr) {
			case MENU_APPEND_ITEM: {
				if (m->nitems >= (m->max_nitems-1))
					{
					fprintf("Too many items in a pie menu.\n");
					abort();
				}
				m->item_list[m->nitems] = (struct menu_item *)(*(argptr+1));
				((struct menu_item *)m->item_list[m->nitems])->parent = m;
				m->nitems += 1;
				argptr += 2; break;
			}
			case MENU_NOTIFY_PROC: {
				(caddr_t)m->notify_proc = (caddr_t)(*(argptr+1));
				argptr += 2; break;
			}
			default: {
				fprintf(stderr, "Unexpected item in pie_menu_set_internal.\n");
				abort();
				break;
			}
		}
	}
}

pie_menu_item_set_internal(mi, argptr)
struct menu_item *mi;
char **argptr;
{
	while (*argptr != 0) {
		switch(*argptr) {
			case MENU_IMAGE:  {
				mi->image->pr = (struct pixrect *)*(argptr+1);
				argptr += 2; break;
			}
			case MENU_STRING: {
				mi->image->string = (char *)*(argptr+1);
				argptr += 2; break;
			}
			case MENU_PULLRIGHT: {
				mi->pullright = TRUE;
				mi->value = (caddr_t)*(argptr+1);
				((struct menu *)mi->value)->parent = mi;
				argptr += 2; break;
			}
			case MENU_RELEASE: {
				mi->free_item = TRUE;
				argptr += 1; break;
			}
			case MENU_RELEASE_IMAGE: {
				mi->image->free_image = TRUE;
				argptr += 1; break;
			}
			case MENU_CLIENT_DATA: {
				mi->client_data = (caddr_t)*(argptr+1);
				argptr += 2; break;
			}
			default: {
				fprintf(stderr, "Unexpected item in pie_menu_item_set_internal.\n");
				abort();
				break;
			}
		}
	}
}

pie_menu_destroy(m)
struct menu *m;
{
	int i;
	if (m == NULL) return;
	for (i=0; i<m->nitems; i++) {
		if (m->item_list[i]->free_item) {
			free(m->item_list[i]->image);
			free(m->item_list[i]);
		}
	}
	free(m->item_list);
	free(m);
}

caddr_t
pie_menu_get(m, arg)
struct menu *m;
Menu_attribute arg;
{
	switch(arg) {
		case MENU_CLIENT_DATA: {
			return (caddr_t) m->client_data;
			break;
		}
		default: {
			fprintf(stderr, "Unexpected item in pie_menu_get.\n");
			abort();
			break;
		}
	}
}

caddr_t
pie_menu_item_get(mi, arg)
struct menu_item *mi;
Menu_attribute arg;
{
	switch(arg) {
		case MENU_CLIENT_DATA: {
			return (caddr_t) mi->client_data;
			break;
		}
		default: {
			fprintf(stderr, "Unexpected item in pie_menu_item_get.\n");
			abort();
			break;
		}
	}
}

caddr_t
pie_menu_show(m, w, e, f, s)
struct menu *m;
Window w;
Event *e;
int (*f)();
char *s;
{
	struct menu_item *rval;
	rval = (struct menu_item *)menu_track(m, w, e, f, s);
}

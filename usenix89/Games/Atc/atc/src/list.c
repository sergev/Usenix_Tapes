/*
 * Copyright (c) 1987 by Ed James, UC Berkeley.  All rights reserved.
 *
 * Copy permission is hereby granted provided that this notice is
 * retained on all partial or complete copies.
 *
 * For more info on this and all of my stuff, mail edjames@berkeley.edu.
 */

#include "include.h"

PLANE	*
newplane()
{
	return ((PLANE *) calloc(1, sizeof (PLANE)));
}

append(l, p)
	LIST	*l;
	PLANE	*p;
{
	if (l->head == NULL) {
		p->next = p->prev = NULL;
		l->head = l->tail = p;
	} else {
		p->prev = l->tail;
		p->next = NULL;
		l->tail->next = p;
		l->tail = p;
	}
}

delete(l, p)
	LIST	*l;
	PLANE	*p;
{
	if (l->head == NULL)
		loser(p, "deleted a non-existant plane! Get help!");
	
	if (l->head == p && l->tail == p)
		l->head = l->tail = NULL;
	else if (l->head == p) {
		l->head = p->next;
		l->head->prev = NULL;
	} else if (l->tail == p) {
		l->tail = p->prev;
		l->tail->next = NULL;
	} else {
		p->prev->next = p->next;
		p->next->prev = p->prev;
	}
}

/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Log:	list.c,v $
 * Revision 2.0  85/12/07  18:21:44  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: list.c,v 2.0 85/12/07 18:21:44 toddb Rel $";
#include	"server.h"
#include	<stdio.h>

/*
 * Stick a new item of any type on top of the list.
 */
l_list *addlist(list, item)
	register l_list	**list;
	register l_list	*item;
{
	item->l_next = *list;
	if (*list)
	{
		item->l_prev = (*list)->l_prev;
		(*list)->l_prev = item;
	}
	else
		item->l_prev = item;
	*list = item;
	return(item);
}

/*
 * delete an item from a list.  The item itself is left intact.  It is
 * the responsibility of the caller to deal with the deleted item.
 */
l_list *deletelist(list, item)
	register l_list	**list;
	register l_list	*item;
{
	if (item == *list)
	{
		if (item->l_next == NULL)
			*list = NULL;
		else
		{
			*list = item->l_next;
			item->l_next->l_prev = item->l_next;
		}
	}
	else
	{
		item->l_prev->l_next = item->l_next;
		if (item->l_next != NULL)
			item->l_next->l_prev = item->l_prev;
	}
	return (*list);
}

/*
 * stick 'item' at the top of the 'list' ( if it isn't there
 * already.
 */
l_list *toplist(list, item)
	register l_list	**list;
	register l_list	*item;
{
	if (item == *list)
		return;
	item->l_prev->l_next = item->l_next;
	if (item->l_next)
		item->l_next->l_prev = item->l_prev;
	item->l_next = (*list);
	/*
	 * if our target is the last on the list, then
	 * be careful that we don't make a cycle.  Since
	 * we want the head of the list's l_prev pointer
	 * to point to the last in the list, we don't
	 * have to do anything.
	 */
	if (item != (*list)->l_prev) /* NOT last on list */
		item->l_prev = (*list)->l_prev;
	(*list)->l_prev = item;
	*list = item;
}
